#include "tokens.h"

#include <stdexcept>

Slice::Slice(char *start, int len) : start(start), len(len) {
}

Slice::Slice(char *start, char *end) : start(start), len(end - start + 1) {
}

void Slice::show() {
    char buf[len + 1];
    strncpy(buf, start, len);
    buf[len] = '\0';
    printf("Slice: %s\n", buf);
}

bool Slice::operator==(const std::string &rhs) {
    for (size_t i = 0; i < this->len; i++) {
        if (rhs[i] != this->start[i]) {
            return false;
        }
    }
    return rhs[this->len] == '\0';
}

Slice::operator std::string() {
    return std::string(start, len);
}
