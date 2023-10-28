#include "parse.h"

#include "tokens.h"

#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <stddef.h>
#include <stdlib.h>
#include <typeinfo>
#include <vector>

static char *start;
static char *current;

/**
 * @brief Determines whether a character is a token separator.
 * A character is a token separator if it is contained within {' ', '\t', '\n', '(', ')',
 * ';', '{', '}'}
 *
 * @param c the character to test
 * @returns if `c` is a token separator
 */
static inline bool isSep(char c);

void tokenize(char *program, off_t size) {
    std::vector<Slice> slices;
    start = current = program;

    // Tokenize the input source file into Slices, skipping over whitespace
    while (current - start < size) {
        while (*current == ' ' || *current == '\t' || *current == '\n') {
            current++;
        }
        if (current - start >= size) {
            break;
        }
        char *tokenStart = current;
        do {
            current++;
        } while (!isSep(*current) && current - start < size);
        slices.emplace_back(tokenStart, current - 1);
    }

    if (slices.size() == 0) {
        puts("File does not contain any source code");
        exit(1);
    }
}

static inline bool isSep(char c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '(' || c == ')' || c == ';'
           || c == '{' || c == '}';
}
