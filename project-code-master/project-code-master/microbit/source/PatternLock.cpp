#include "PatternLock.h"

extern MicroBit uBit;
extern Keyboard keyboard;

#define IMAGE_SIZE 15

inline void move(int& x, int& y);

PatternLock::PatternLock(InteractiveComponent* next, bool force_relog)
    : InteractiveComponent(next), // superclass constructor
      im(IMAGE_SIZE, IMAGE_SIZE) {

    if (force_relog) {
        uBit.storage.remove("user");
    }
}

void PatternLock::controlGiven() {
    // attempts to get stored lock combination
    KeyValuePair* username_stored = uBit.storage.get("user");
    keyboard.parent = this;
    compare_to_local = username_stored != NULL;
    if (username_stored == NULL) {
        // if it isn't stored, prompt the user to enter it
        uBit.display.scroll("Username:");

        switchToComponent(&keyboard);
        return;
    } else {
        char username_buf[32];
        memcpy(&username_buf, username_stored->value, 32);
        delete username_stored;
        username = username_buf;
    }

    pattern();
}

void PatternLock::pattern() {
    // draw grid points
    for (size_t x = 0; x < 3; x++) {
        for (size_t y = 0; y < 3; y++) {
            draw_point(x, y, 20);
        }
    }
    show();

    while (true) {
        // use accelerometer to move
        move(x, y);
        // logic
        bool exit = update_pattern();
        if (exit) {
            return;
        }
        // update display
        show();
    }
    uBit.sleep(500);
    // switch to the next component
    toParent();
}

void PatternLock::controlReturned() {
    // control returned from keyboard after the user has entered their username
    username = keyboard.getMessage();

    keyboard.freeBuffer();

    pattern();
}

void PatternLock::show() {
    // draw center dot
    int val = im.getPixelValue(x + 2, y + 2);
    im.setPixelValue(x + 2, y + 2, 255);
    uBit.display.print(im, -x, -y);

    uBit.sleep(200);
    // un-draw centre dot
    im.setPixelValue(x + 2, y + 2, val);
}

bool PatternLock::update_pattern() {
    bool a_pressed = uBit.buttonA.isPressed();
    bool b_pressed = uBit.buttonB.isPressed();

    if (a_pressed && b_pressed) {
        // if A and B buttons pressed, attempt to log in
        bool correct;

        if (compare_to_local) {
            // if only comparing to the stored value, compare to the stored
            // value
            KeyValuePair* pattern_stored = uBit.storage.get("lock");
            int pattern;
            memcpy((char*)&pattern, (char*)pattern_stored->value,
                   sizeof(pattern));
            delete pattern_stored;
            correct = pattern == curr_combination;
        } else {
            // else send a login request
            if (serial::server_request(username, curr_combination, SERIAL_LOGIN,
                                       " ")) {
                // if the server responded
                int type;
                // get server response
                serial::receive(&type);
                correct = type;
                if (!correct) {
                    uBit.display.scroll("Wrong. Username:");
                }
            } else {
                correct = false;
                uBit.display.scroll("no connection");
            }
        }
        if (correct) {
            // if login succeeded
            if (!compare_to_local) {
                // if logged in by contacting the server, store username and
                // pattern
                uint8_t buf[32];
                memcpy((char*)buf, username.toCharArray(), username.length());
                buf[username.length()] = '\0';

                uBit.storage.put("user", buf, 32);
                uBit.storage.put("lock", (uint8_t*)&curr_combination,
                                 sizeof(curr_combination));
            }
            // if login succeeded, go to menu
            toParent();
        } else {
            // if login failed, try again

            // reset to blank grid before filling in
            prev = 0;
            visited = 0;
            curr_combination = 0;
            x = 0;
            y = 0;
            im.clear();
            for (size_t x = 0; x < 3; x++) {
                for (size_t y = 0; y < 3; y++) {
                    draw_point(x, y, 20);
                }
            }
            if (!compare_to_local) {
                switchToComponent(&keyboard);
            } else {
                return false;
            }
        }
        return true;

    } else if (a_pressed || b_pressed) {
        // if a button is pressed, record it
        button_pressed();
    }

    return false;
}

// move the cursor on the screen
void move(int& x, int& y) {
    int pitch = uBit.accelerometer.getPitch();
    int roll = uBit.accelerometer.getRoll();

    if (pitch > 20 && y < IMAGE_SIZE - 5) {
        y++;
    } else if (pitch < -20 && y > 0) {
        y--;
    }
    if (roll > 20 && x < IMAGE_SIZE - 5) {
        x++;
    } else if (roll < -20 && x > 0) {
        x--;
    }
}

void PatternLock::button_pressed() {
    // check if on a point
    int x = this->x + 2;
    int y = this->y + 2;

    if (x % 6 >= 3 || y % 6 >= 3) {
        // if not on a point, exit
        return;
    }
    // convert to point number
    int cx = x / 6;
    int cy = y / 6;
    int curr = cx + 3 * cy + 1;

    // check visited
    if ((visited >> curr) & 1) {
        return;
    }
    // mark visited
    visited |= 1 << curr;

    // draw line
    if (prev != 0) {
        draw_line(prev, curr);
    }
    prev = curr;

    draw_point(cx, cy, 200);

    // add to curr_combination
    curr_combination *= 10;
    curr_combination += curr;
}

void PatternLock::draw_line(int from, int to) {
    // covert from point number (a number from 1 to 9) to screen coord
    from--;
    to--;
    int x1 = (from % 3) * 6 + 1;
    int y1 = (from / 3) * 6 + 1;

    int x2 = (to % 3) * 6 + 1;
    int y2 = (to / 3) * 6 + 1;

    // get the vector between the numbers
    float dirx = (float)(x2 - x1);
    float diry = (float)(y2 - y1);
    {
        // normalise the vector

        // find distance^2 = x^2 + y^2
        float magnitude_squared = dirx * dirx + diry * diry;

        // find an approximiation of the square root
        float magnitude1 = magnitude_squared / 2;
        float magnitude2 = 0;
        while (magnitude1 - magnitude2 > 0.001 ||
               magnitude1 - magnitude2 < -0.001) {
            magnitude2 = magnitude_squared / magnitude1;
            magnitude1 = (magnitude1 + magnitude2) / 2;
        }
        // divide by the magnitude to normalise the vector
        dirx /= magnitude1;
        diry /= magnitude1;
    }
    // find the vector divided by 15 in order to take 15 steps over each pixel:
    // this is simple antialiasing
    dirx /= 15;
    diry /= 15;

    // start the line at x1
    float x = (float)x1;
    float y = (float)y1;
    while (true) {
        // get the current point on which
        int xi = (int)(x + 0.5);
        int yi = (int)(y + 0.5);

        if (xi == x2 && yi == y2) {
            // if the second point has been reached
            break;
        } else if (xi % 6 >= 3 || yi % 6 >= 3) {
            // if not between points
            im.setPixelValue(xi, yi, im.getPixelValue(xi, yi) + 1);
        }

        x += dirx;
        y += diry;
    }
}

void PatternLock::draw_point(int cx, int cy, int brightness) {
    for (int16_t x = 0; x < 3; x++) {
        for (int16_t y = 0; y < 3; y++) {
            im.setPixelValue(cx * 6 + x, cy * 6 + y, brightness);
        }
    }
}
