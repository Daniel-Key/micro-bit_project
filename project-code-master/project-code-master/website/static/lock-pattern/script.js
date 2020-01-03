let canvas = $('#canvas');
let ctx = canvas[0].getContext("2d");

const CANVAS_SIZE = 500;
const CANVAS_BORDER = 50;
const POINT_RADIUS = 40;
const LINE_WIDTH = 20;

// calculating spacing between grid points
let spacing = (CANVAS_SIZE - CANVAS_BORDER * 2) / 2;

let points = [];

// a point in the grid
class Point {
    // constructor takes one args: the grid number
    constructor(num) {
        // convert number to position
        let x = (num - 1) % 3;
        let y = Math.floor((num - 1) / 3);
        this.num = num;

        this.x = x * spacing + CANVAS_BORDER;
        this.y = y * spacing + CANVAS_BORDER;

        // add this to the list
        points.push(this);
    }

    // check if a point is within the radius of the circle of this point
    includesPoint(x, y) {
        return Math.hypot(this.x - x, this.y - y) < POINT_RADIUS;
    }

    // find the grid point that the (x,y) position is within
    static find(x, y) {
        for (let point of points) {
            if (point.includesPoint(x, y)) {
                return point;
            }
        }
        return undefined;
    }

    // draw this point
    draw() {
        ctx.fillStyle = '#bbba';
        ctx.beginPath();
        ctx.arc(this.x, this.y, POINT_RADIUS, 0, 2 * Math.PI);
        ctx.fill();
        ctx.fillStyle = '#ccc';
        ctx.beginPath();
        ctx.arc(this.x, this.y, LINE_WIDTH, 0, 2 * Math.PI);
        ctx.fill();
    }
}
let lines = new Set();
// a line between two points
class Line {
    constructor(x1, y1, x2, y2) {
        this.x1 = x1;
        this.y1 = y1;
        this.x2 = x2;
        this.y2 = y2;
    }

    // draw the line
    draw() {
        ctx.lineWidth = LINE_WIDTH * 2;
        ctx.strokeStyle = "rgba(200,0,0,1)";
        ctx.lineCap = "round";
        ctx.beginPath();
        ctx.moveTo(this.x1, this.y1);
        ctx.lineTo(this.x2, this.y2);
        ctx.stroke();
    }
}

// make points
for (let i = 1; i <= 9; i++) {
    new Point(i);
}

// redraw the canvas
function redraw() {
    ctx.clearRect(0, 0, canvas[0].width, canvas[0].height);

    if (tempLine) {
        tempLine.draw();
    }
    for (let line of lines) {
        line.draw();
    }
    for (let point of points) {
        point.draw();
    }
}

// click handlers and logic
let dragging = false;

let prev = undefined;
let tempLine = undefined;
let visited = new Set();
let code = 0;

// when mouse clicked or dragged...
function mousedown(e) {
    dragging = true;
    // get coords within canvas
    let x = e.pageX - canvas.offset().left;
    let y = e.pageY - canvas.offset().top;

    // get the point being click on
    let point = Point.find(x, y);
    // if clicked or dragged over a point that hasn't yet been visited
    if (point && !visited.has(point)) {
        // if there is a previous point (i.e. if this isn't the first point)
        if (prev) {
            // then add a line from the previous point to this point
            lines.add(new Line(prev.x, prev.y, point.x, point.y));
        }
        // add the current point to the pattern code
        code *= 10;
        code += point.num;

        // add this point to the visited set
        visited.add(point);
        // set previous
        prev = point;

        if (visited.size === 9) {
            // if all 9 points have been visited, don't show a line from the
            // previous point to the current mouse position
            tempLine = undefined;
        } else if (visited.size === 3) {
            // if visited 3 or more points, then allow pattern lock submission
            $('#submit').prop('disabled', false);
        }

    } else if (prev) {
        // if not on a point, and if there has already been a point selected
        if (visited.size < 9) {
            // then if there could be another point selected, show a line
            // from the previous point to the current mouse position
            tempLine = new Line(prev.x, prev.y, x, y);
        }
    }
}

canvas.mousedown(mousedown);
canvas.mousemove(e => {
    if (dragging) {
        mousedown(e);
    }
});
// when the mouse is released, remove the line from
// the previous point to the current mouse position
$(window).mouseup(e => {
    dragging = false;
    tempLine = undefined;
});
setInterval(redraw, 10);

// when the submit button is clicked
$('#submit').click(() => {
    // send a request to set the lock pattern
    $.ajax({
        url: '/JH-project/api/pattern',
        type: "POST",
        dataType: "json",
        contentType: "application/json; charset=utf-8",
        data: JSON.stringify({
            pattern: code
        })
    }).done(
        // on success, report "Pattern Set"
        () => {
            $('#message').empty();
            $('#message').append($('<span>Pattern Set</span>'));
        }
    ).fail(
        // on failure, report "Error setting the pattern"
        () => {
            $('#message').empty();
            $('#message').append($('<span>Error setting the pattern</span>'));
        });
});

// when the clear button is clicked, reset the pattern
$('#clear').click(() => {
    tempLine = undefined;
    prev = undefined;
    dragging = false;
    visited = new Set();
    lines = new Set();
    code = 0;
    $('#submit').prop('disabled', true);
});
