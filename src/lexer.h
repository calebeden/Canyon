#ifndef LEXER_H
#define LEXER_H

#include "tokens.h"

#include <cstdint>
#include <vector>

#define TAB_WIDTH 4

class Lexer {
    const char *program;
    off_t size;
    const char *source;
    const char *current;
    std::vector<Slice> slices;
public:
    /**
     * @brief Construct a new Lexer object to tokenize Canyon source code
     *
     * @param program the source code to tokenize
     * @param size the length of the source code
     * @param source the name of the source code file
     */
    Lexer(const char *const program, off_t size, const char *const source);
    /**
     * @brief Tokenizes the Canyon source code
     *
     * @return the source code as a series of Tokens, stored in a vector
     */
    std::vector<Token *> *tokenize();
private:
    /**
     * @brief Splits the input source file into Slices based on whitespace and other token
     * separator rules
     */
    void slice();

    /**
     * @brief Determines whether the character pointed to by c represents the start of a
     * new token
     *
     * @param c the character to test at
     * @returns whether `c` is at the start of a new token
     */
    static inline bool isSep(const char *const c);
};

#endif
