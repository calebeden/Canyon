#ifndef TOKENS_H
#define TOKENS_H

#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <stddef.h>
#include <stdexcept>
#include <stdlib.h>
#include <string>
#include <typeinfo>
#include <vector>

class Slice {
    char *start;
    size_t len;
public:
    /**
     * @brief Construct a new Slice object based on a pointer to its start and its length
     *
     * @param start a pointer to the first character in the Slice
     * @param len the number of characters in the Slice
     */
    Slice(char *start, int len);
    /**
     * @brief Construct a new Slice object based on pointers to its start and end
     *
     * @param start a pointer to the first character in the Slice
     * @param end a pointer to the final character of the Slice (inclusive)
     */
    Slice(char *start, char *end);
    void show();
    /**
     * @brief Compares a Slice to a string
     *
     * @param rhs the std::string to compare to
     * @return whether the Slice and string contain the same characters and are the same
     * length
     */
    bool operator==(const std::string &rhs);

    operator std::string();
};

#endif
