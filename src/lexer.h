#ifndef LEXER_H
#define LEXER_H

#include "tokens.h"
#include <string_view>

#include <cstdint>
#include <queue>
#include <vector>

#ifdef DEBUG_TEST_MODE
#	define mockable virtual
#else
#	define mockable
#endif

class Lexer {
#ifdef DEBUG_TEST_MODE
public:
#endif
	const char *program;
	const char *current;
	off_t size;
	const char *source;
	std::queue<Slice> slices;
	uint32_t tabSize;
public:
	/**
	 * @brief Construct a new Lexer object to tokenize Canyon source code
	 *
	 * @param program the source code to tokenize
	 * @param size the length of the source code
	 * @param source the name of the source code file
	 * @param tabSize the width of a tab stop (default = 4)
	 */
	Lexer(const char *program, off_t size, const char *source, uint32_t tabSize = 4);
	/**
	 * @brief Tokenizes the Canyon source code
	 *
	 * @return the source code as a series of Tokens, stored in a vector
	 */
	mockable std::vector<Token *> tokenize();
#ifndef DEBUG_TEST_MODE
private:
#endif
	/**
	 * @brief Splits the input source file into Slices based on whitespace and other token
	 * separator rules
	 */
	mockable void slice();

	/**
	 * @brief Determines whether the character pointed to by c represents the start of a
	 * new token. If c points to the first character of a string and if isalnum(*c)
	 * returns true, this is considered UB
	 *
	 * @param c the character to test at
	 * @returns whether `c` is at the start of a new token
	 */
	mockable bool isSep(const char *c);

	mockable Keyword *createKeyword(const Slice &s);
	mockable Primitive *createPrimitive(const Slice &s);
	mockable Punctuation *createPunctuation(const Slice &s);
	mockable Identifier *createIdentifier(const Slice &s);
};

#endif
