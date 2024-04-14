#ifndef LEXER_H
#define LEXER_H

#include "tokens.h"

#include <cstdint>
#include <filesystem>
#include <queue>
#include <string>
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
	std::string_view program;
	size_t current;
	std::filesystem::path source;
	std::queue<Slice> slices;
	uint32_t tabSize;
public:
	/**
	 * @brief Construct a new Lexer object to tokenize Canyon source code
	 *
	 * @param program the source code to tokenize
	 * @param source the name of the source code file
	 * @param tabSize the width of a tab stop (default = 4)
	 */
	Lexer(std::string_view program, std::filesystem::path source, uint32_t tabSize = 4);
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
	 * @brief Determines whether a character in a string represents the start of a new
	 * token. If offset == 0 && isalnum(s[offset]), this is considered UB
	 *
	 * @param s the string to test with
	 * @param offset where in the string to test at
	 * @returns whether s[offset] is at the start of a new token
	 */
	mockable bool isSep(std::string_view s, size_t offset);

	mockable Keyword *createKeyword(const Slice &s);
	mockable Primitive *createPrimitive(const Slice &s);
	mockable Punctuation *createPunctuation(const Slice &s);
	mockable Identifier *createIdentifier(const Slice &s);
public:
	~Lexer() = default;
};

#endif
