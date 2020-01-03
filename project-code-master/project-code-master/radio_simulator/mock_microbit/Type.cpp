#include "Type.h"

#include <mutex>

std::mutex log_mutex;

void log(Type type, int len, const char* msg) {
    // make sure it it thread-safe
    std::lock_guard<std::mutex> guard(log_mutex);
    // message format is
    // "[type]\n[length]\n[byte1decimal],[byte2decimal],[...]\n"
    printf("%d\n%d\n", type, len);
    for (size_t i = 0; i < len; i++) {
        printf("%d", msg[i]);
        putchar(i + 1 < len ? ',' : '\n');
    }
    fflush(stdout);
}
