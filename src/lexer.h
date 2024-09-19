#ifndef LEXER_H
#define LEXER_H

#include "errorhandler.h"
#include "tokens.h"

#include <cstdint>
#include <filesystem>
#include <memory>
#include <queue>
#include <string_view>
#include <vector>

/**
 * @brief Converts Canyon source code into a series of Tokens
 * 
 */
class Lexer {
	std::string_view program;
	size_t current = 0;
	std::filesystem::path source;
	std::queue<Slice> slices;
	uint32_t tabSize;
	ErrorHandler *errorHandler;
	size_t line = 1;
	size_t col = 1;
public:
	/**
	 * @brief Construct a new Lexer object to tokenize Canyon source code
	 *
	 * @param program the source code to tokenize
	 * @param source the name of the source code file
	 * @param errorHandler the error handler to use
	 * @param tabSize the width of a tab stop (default = 4)
	 */
	Lexer(std::string_view program, std::filesystem::path source,
	      ErrorHandler *errorHandler, uint32_t tabSize = 4);
	Lexer &operator=(const Lexer &l);
	~Lexer() = default;
	std::vector<std::unique_ptr<Token>> lex();
private:
	/**
	 * @brief Scans the Canyon source code
	 *
	 * @return the source code as a series of Tokens, stored in a vector
	 */
	std::vector<std::unique_ptr<Token>> scan();
	std::vector<std::unique_ptr<Token>> evaluate(
	      std::vector<std::unique_ptr<Token>> tokens);
	static bool isDigitInBase(char c, int base);
	static std::unique_ptr<IntegerLiteral> evaluateLiteral(SymbolOrLiteral *literal);
	/**
	 * @brief Splits the input source file into Slices based on whitespace and other token
	 * separator rules
	 */
	void slice();

	/**
	 * @brief Determines whether a character in the program represents the start of a new
	 * token. If offset == 0 && isalnum(program[offset]), this is considered UB
	 *
	 * @param offset where in the string to test at
	 * @returns whether program[offset] is at the start of a new token
	 */
	bool isSep(size_t offset);

	static std::unique_ptr<Whitespace> createWhitespace(const Slice &s);
	static std::unique_ptr<Keyword> createKeyword(const Slice &s);
	static std::unique_ptr<Punctuation> createPunctuation(const Slice &s);
	static std::unique_ptr<SymbolOrLiteral> createSymbolOrLiteral(const Slice &s);
};

#endif
