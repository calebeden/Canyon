#ifndef DEBUG_TEST_MODE
#	error "DEBUG_TEST_MODE not defined"
#endif

#include "errorhandler.h"
#include "lexer.h"
#include "test_utilities.h"
#include "tokens.h"

#include "gtest/gtest.h"

#include <algorithm>
#include <array>
#include <filesystem>
#include <memory>
#include <optional>
#include <queue>
#include <stdexcept>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

using namespace ::testing;

static constexpr std::array whitespaces = {' ', '\n', '\t', '\v', '\f', '\r'};
static constexpr std::array punctuations = {
      std::pair('(', Punctuation::Type::OpenParen),
      std::pair(')', Punctuation::Type::CloseParen),
      std::pair(';', Punctuation::Type::Semicolon),
      std::pair('{', Punctuation::Type::OpenBrace),
      std::pair('}', Punctuation::Type::CloseBrace),
      std::pair('.', Punctuation::Type::Period),
      std::pair(',', Punctuation::Type::Comma),
      std::pair('=', Punctuation::Type::Equals),
      std::pair(':', Punctuation::Type::Colon),
      std::pair('+', Punctuation::Type::Plus),
      std::pair('-', Punctuation::Type::Hyphen),
      std::pair('*', Punctuation::Type::Asterisk),
      std::pair('/', Punctuation::Type::ForwardSlash),
      std::pair('%', Punctuation::Type::Percent),
      std::pair('!', Punctuation::Type::Exclamation),
      std::pair('<', Punctuation::Type::LessThan),
      std::pair('>', Punctuation::Type::GreaterThan),
      std::pair('&', Punctuation::Type::Ampersand),
      std::pair('|', Punctuation::Type::VerticalBar),
      std::pair('~', Punctuation::Type::Tilde),
      std::pair('^', Punctuation::Type::Caret),
};
static constexpr std::array operators = {
      std::pair("=", Operator::Type::Assignment),
      std::pair("==", Operator::Type::Equality),
      std::pair("!=", Operator::Type::Inequality),
      std::pair("<", Operator::Type::LessThan),
      std::pair(">", Operator::Type::GreaterThan),
      std::pair("<=", Operator::Type::LessThanOrEqual),
      std::pair(">=", Operator::Type::GreaterThanOrEqual),
      std::pair("+", Operator::Type::Addition),
      std::pair("-", Operator::Type::Subtraction),
      std::pair("*", Operator::Type::Multiplication),
      std::pair("/", Operator::Type::Division),
      std::pair("%", Operator::Type::Modulus),
      std::pair("::", Operator::Type::Scope),
      std::pair("!", Operator::Type::LogicalNot),
      std::pair("&&", Operator::Type::LogicalAnd),
      std::pair("||", Operator::Type::LogicalOr),
      std::pair("~", Operator::Type::BitwiseNot),
      std::pair("&", Operator::Type::BitwiseAnd),
      std::pair("|", Operator::Type::BitwiseOr),
      std::pair("^", Operator::Type::BitwiseXor),
      std::pair("<<", Operator::Type::BitwiseShiftLeft),
      std::pair(">>", Operator::Type::BitwiseShiftRight),
};

static constexpr std::array escapeSequences = {std::pair("\\a", '\a'),
      std::pair("\\b", '\b'), std::pair("\\f", '\f'), std::pair("\\n", '\n'),
      std::pair("\\r", '\r'), std::pair("\\t", '\t'), std::pair("\\v", '\v'),
      std::pair("\\\\", '\\'), std::pair("\\'", '\''), std::pair("\\\"", '\"')};

class TestLexer : public testing::Test {
public:
	TestLexer() : l("", "", &e) {
	}
protected:
	NoErrorHandler e;
	Lexer l;
	std::vector<std::unique_ptr<Token>> tokens;
};

class TestLexerError : public testing::Test {
public:
	TestLexerError() : l("", "", &e) {
	}
protected:
	HasErrorHandler e;
	Lexer l;
	std::vector<std::unique_ptr<Token>> tokens;
};

class TestLexerPrimitives
    : public TestLexer,
      public testing::WithParamInterface<std::pair<std::string, IntegerLiteral::Type>> {
};

class TestLexerKeywords
    : public TestLexer,
      public testing::WithParamInterface<std::pair<std::string, Keyword::Type>> { };

class TestLexerSymbols : public TestLexer,
                         public testing::WithParamInterface<std::string> { };

class TestLexerLiterals
    : public TestLexer,
      public testing::WithParamInterface<std::pair<std::string, IntegerLiteral>> { };

class TestLexerInvalidLiterals
    : public TestLexerError,
      public testing::WithParamInterface<std::pair<std::string,
            std::tuple<std::filesystem::path, size_t, size_t, std::string>>> { };

/**
 * @brief Ensure that the Lexer constructor throws an exception when tabSize is 0
 */
TEST(testLexer, testConstructor) {
	NoErrorHandler e;
	EXPECT_THROW(Lexer("", "", &e, 0), std::invalid_argument);
}

/**
 * @brief Ensure that the lexer returns only an EOF token when the input is empty
 *
 */
TEST_F(TestLexer, testEmpty) {
	l = Lexer("", "", &e);
	tokens = l.lex();
	EXPECT_EQ(tokens.size(), 1);
	EXPECT_TRUE(dynamic_cast<EndOfFile *>(tokens[0].get()));
}

/**
 * @brief Ensure that whitespace is never included in the output tokens
 */
TEST_F(TestLexer, testWhitespace) {
	// Test 1: Single whitespace characters
	for (const auto c : whitespaces) {
		const std::string program = std::string(1, c);
		l = Lexer(program, "", &e);
		tokens = l.lex();
		EXPECT_EQ(tokens.size(), 1);
		EXPECT_TRUE(dynamic_cast<EndOfFile *>(tokens[0].get()));
	}

	// Test 2: Double whitespace characters
	for (const auto c : whitespaces) {
		const std::string program = std::string(1, c) + std::string(1, c);
		l = Lexer(program, "", &e);
		tokens = l.lex();
		EXPECT_EQ(tokens.size(), 1);
		EXPECT_TRUE(dynamic_cast<EndOfFile *>(tokens[0].get()));
	}

	// Test 3: Mixed whitespace characters
	for (const auto c1 : whitespaces) {
		for (const auto c2 : whitespaces) {
			const std::string program = std::string(1, c1) + std::string(1, c2);
			l = Lexer(program, "", &e);
			tokens = l.lex();
			EXPECT_EQ(tokens.size(), 1);
			EXPECT_TRUE(dynamic_cast<EndOfFile *>(tokens[0].get()));
		}
	}

	// Test 4: Permutations of mixed whitespace characters
	auto whitespaces2 = whitespaces;
	do {
		const std::string permutation
		      = std::string(whitespaces2.begin(), whitespaces2.end());
		l = Lexer(permutation, "", &e);
		tokens = l.lex();
		EXPECT_EQ(tokens.size(), 1);
		EXPECT_TRUE(dynamic_cast<EndOfFile *>(tokens[0].get()));
	} while (std::next_permutation(whitespaces2.begin(), whitespaces2.end()));
}

std::optional<Operator::Type> getOperator(const std::string &s) {
	for (const auto &pair : operators) {
		if (pair.first == s) {
			return pair.second;
		}
	}
	return std::nullopt;
}

/**
 * @brief Ensure that single punctuations get correctly lexed into an Operator if
 * possible, otherwise a Punctuation
 *
 */
TEST_F(TestLexer, testSinglePunctuation) {
	for (const auto &pair : punctuations) {
		const char c = pair.first;
		const Punctuation::Type punctuation = pair.second;
		const std::string program = std::string(1, c);
		l = Lexer(program, "", &e);
		tokens = l.lex();
		EXPECT_EQ(tokens.size(), 2);
		std::optional<Operator::Type> op = getOperator(std::string(1, c));
		if (op.has_value()) {
			EXPECT_EQ(dynamic_cast<Operator *>(tokens[0].get())->type, op.value());
			EXPECT_TRUE(dynamic_cast<EndOfFile *>(tokens[1].get()));
		} else {
			EXPECT_EQ(dynamic_cast<Punctuation *>(tokens[0].get())->type, punctuation);
			EXPECT_TRUE(dynamic_cast<EndOfFile *>(tokens[1].get()));
		}
	}
}

/**
 * @brief Ensure that two consecutive punctuations get correctly lexed into a combined
 * Operator if possible, or two distinct Operators/Punctuations otherwise
 *
 */
TEST_F(TestLexer, testTwoPunctuation) {
	// Test 2: 2 consecutive punctuation characters
	for (const auto &pair1 : punctuations) {
		const char c1 = pair1.first;
		const Punctuation::Type punctuation1 = pair1.second;
		for (const auto &pair2 : punctuations) {
			const char c2 = pair2.first;
			const Punctuation::Type punctuation2 = pair2.second;
			const std::string combo = std::string(1, c1) + std::string(1, c2);
			l = Lexer(combo, "", &e);
			tokens = l.lex();
			const std::optional<Operator::Type> op = getOperator(combo);
			if (op.has_value()) {
				EXPECT_EQ(tokens.size(), 2);
				EXPECT_EQ(dynamic_cast<Operator *>(tokens[0].get())->type, op.value());
				EXPECT_TRUE(dynamic_cast<EndOfFile *>(tokens[1].get()));
			} else {
				EXPECT_EQ(tokens.size(), 3);
				const std::optional<Operator::Type> op1 = getOperator(std::string(1, c1));
				if (op1.has_value()) {
					EXPECT_EQ(dynamic_cast<Operator *>(tokens[0].get())->type,
					      op1.value());
				} else {
					EXPECT_EQ(dynamic_cast<Punctuation *>(tokens[0].get())->type,
					      punctuation1);
				}
				const std::optional<Operator::Type> op2 = getOperator(std::string(1, c2));
				if (op2.has_value()) {
					EXPECT_EQ(dynamic_cast<Operator *>(tokens[1].get())->type,
					      op2.value());
				} else {
					EXPECT_EQ(dynamic_cast<Punctuation *>(tokens[1].get())->type,
					      punctuation2);
				}
				EXPECT_TRUE(dynamic_cast<EndOfFile *>(tokens[2].get()));
			}
		}
	}
}

/**
 * @brief Ensure that three consecutive punctuations get correctly lexed into a combined
 * Operator if possible, or that the first two are combined and the third separate, or the
 * first separate and the last two combined, or all three separate
 *
 */
TEST_F(TestLexer, testThreePunctuation) {
	for (const auto &pair1 : punctuations) {
		const char c1 = pair1.first;
		const Punctuation::Type punctuation1 = pair1.second;
		for (const auto &pair2 : punctuations) {
			const char c2 = pair2.first;
			const Punctuation::Type punctuation2 = pair2.second;
			for (const auto &pair3 : punctuations) {
				const char c3 = pair3.first;
				const Punctuation::Type punctuation3 = pair3.second;
				const std::string combo
				      = std::string(1, c1) + std::string(1, c2) + std::string(1, c3);
				l = Lexer(combo, "", &e);
				tokens = l.lex();
				const std::optional<Operator::Type> op = getOperator(combo);
				if (op.has_value()) {
					EXPECT_EQ(tokens.size(), 2);
					EXPECT_EQ(dynamic_cast<Operator *>(tokens[0].get())->type,
					      op.value());
					EXPECT_TRUE(dynamic_cast<EndOfFile *>(tokens[1].get()));
				} else {
					const std::optional<Operator::Type> op1
					      = getOperator(std::string(1, c1) + std::string(1, c2));
					if (op1.has_value()) {
						EXPECT_EQ(tokens.size(), 3);
						EXPECT_EQ(dynamic_cast<Operator *>(tokens[0].get())->type,
						      op1.value());
						const std::optional<Operator::Type> op2
						      = getOperator(std::string(1, c3));
						if (op2.has_value()) {
							EXPECT_EQ(dynamic_cast<Operator *>(tokens[1].get())->type,
							      op2.value());
						} else {
							EXPECT_EQ(dynamic_cast<Punctuation *>(tokens[1].get())->type,
							      punctuation3);
						}
						EXPECT_TRUE(dynamic_cast<EndOfFile *>(tokens[2].get()));
					} else {
						const std::optional<Operator::Type> op2
						      = getOperator(std::string(1, c2) + std::string(1, c3));
						if (op2.has_value()) {
							EXPECT_EQ(tokens.size(), 3);
							const std::optional<Operator::Type> op1
							      = getOperator(std::string(1, c1));
							if (op1.has_value()) {
								EXPECT_EQ(dynamic_cast<Operator *>(tokens[0].get())->type,
								      op1.value());
							} else {
								EXPECT_EQ(
								      dynamic_cast<Punctuation *>(tokens[0].get())->type,
								      punctuation1);
							}
							EXPECT_EQ(dynamic_cast<Operator *>(tokens[1].get())->type,
							      op2.value());
							EXPECT_TRUE(dynamic_cast<EndOfFile *>(tokens[2].get()));
						} else {
							EXPECT_EQ(tokens.size(), 4);
							const std::optional<Operator::Type> op1
							      = getOperator(std::string(1, c1));
							if (op1.has_value()) {
								EXPECT_EQ(dynamic_cast<Operator *>(tokens[0].get())->type,
								      op1.value());
							} else {
								EXPECT_EQ(
								      dynamic_cast<Punctuation *>(tokens[0].get())->type,
								      punctuation1);
							}
							const std::optional<Operator::Type> op2
							      = getOperator(std::string(1, c2));
							if (op2.has_value()) {
								EXPECT_EQ(dynamic_cast<Operator *>(tokens[1].get())->type,
								      op2.value());
							} else {
								EXPECT_EQ(
								      dynamic_cast<Punctuation *>(tokens[1].get())->type,
								      punctuation2);
							}
							const std::optional<Operator::Type> op3
							      = getOperator(std::string(1, c3));
							if (op3.has_value()) {
								EXPECT_EQ(dynamic_cast<Operator *>(tokens[2].get())->type,
								      op3.value());
							} else {
								EXPECT_EQ(
								      dynamic_cast<Punctuation *>(tokens[2].get())->type,
								      punctuation3);
							}
							EXPECT_TRUE(dynamic_cast<EndOfFile *>(tokens[3].get()));
						}
					}
				}
			}
		}
	}
}

/**
 * @brief Test punctuation sequences with whitespace prefixes and suffixes
 *
 */
TEST_F(TestLexer, testPunctuationWhitespace) {
	for (const auto &pair : punctuations) {
		const char c = pair.first;
		const Punctuation::Type punctuation = pair.second;
		for (const auto ws : whitespaces) {
			std::string program = std::string(1, c) + std::string(1, ws);
			l = Lexer(program, "", &e);
			tokens = l.lex();
			EXPECT_EQ(tokens.size(), 2);
			const std::optional<Operator::Type> op1 = getOperator(std::string(1, c));
			if (op1.has_value()) {
				EXPECT_EQ(dynamic_cast<Operator *>(tokens[0].get())->type, op1.value());
			} else {
				EXPECT_EQ(dynamic_cast<Punctuation *>(tokens[0].get())->type,
				      punctuation);
			}
			EXPECT_TRUE(dynamic_cast<EndOfFile *>(tokens[1].get()));
			program = std::string(1, ws) + std::string(1, c);
			l = Lexer(program, "", &e);
			tokens = l.lex();
			EXPECT_EQ(tokens.size(), 2);
			const std::optional<Operator::Type> op2 = getOperator(std::string(1, c));
			if (op2.has_value()) {
				EXPECT_EQ(dynamic_cast<Operator *>(tokens[0].get())->type, op2.value());
			} else {
				EXPECT_EQ(dynamic_cast<Punctuation *>(tokens[0].get())->type,
				      punctuation);
			}
			EXPECT_TRUE(dynamic_cast<EndOfFile *>(tokens[1].get()));
			program = std::string(1, ws) + std::string(1, c) + std::string(1, ws);
			l = Lexer(program, "", &e);
			tokens = l.lex();
			EXPECT_EQ(tokens.size(), 2);
			const std::optional<Operator::Type> op3 = getOperator(std::string(1, c));
			if (op3.has_value()) {
				EXPECT_EQ(dynamic_cast<Operator *>(tokens[0].get())->type, op3.value());
			} else {
				EXPECT_EQ(dynamic_cast<Punctuation *>(tokens[0].get())->type,
				      punctuation);
			}
			EXPECT_TRUE(dynamic_cast<EndOfFile *>(tokens[1].get()));
		}
	}
}

/**
 * @brief Tests that whitespace between punctuation prevents them from combining into a
 * multi-character operator
 *
 */
TEST_F(TestLexer, testMulticharOperators) {
	for (const auto &pair : operators) {
		const std::string op = pair.first;
		if (op.size() == 1) {
			continue;
		}
		l = Lexer(op, "", &e);
		tokens = l.lex();
		EXPECT_EQ(tokens.size(), 2);
		EXPECT_EQ(dynamic_cast<Operator *>(tokens[0].get())->type, pair.second);
		EXPECT_TRUE(dynamic_cast<EndOfFile *>(tokens[1].get()));
		for (const auto ws : whitespaces) {
			std::string program = op.substr(0, 1);
			for (size_t i = 1; i < op.size(); i++) {
				program += ws + op.substr(i, 1);
			}
			l = Lexer(program, "", &e);
			tokens = l.lex();
			EXPECT_EQ(tokens.size(), op.size() + 1);
			for (size_t i = 0; i < op.size(); i++) {
				EXPECT_TRUE(dynamic_cast<Punctuation *>(tokens[i].get())
				            || dynamic_cast<Operator *>(tokens[i].get()));
			}
			EXPECT_TRUE(dynamic_cast<EndOfFile *>(tokens[op.size()].get()));
		}
	}
}

TEST_P(TestLexerKeywords, testKeywords) {
	const std::string keyword_str = GetParam().first;
	const Keyword::Type keyword_type = GetParam().second;

	// Test 1: substring prefixes
	for (size_t i = 0; i < keyword_str.size() - 1; i++) {
		const std::string program = keyword_str.substr(0, i + 1);
		l = Lexer(program, "", &e);
		tokens = l.lex();
		EXPECT_EQ(tokens.size(), 2);
		EXPECT_FALSE(dynamic_cast<Keyword *>(tokens[0].get()));
		EXPECT_TRUE(dynamic_cast<EndOfFile *>(tokens[1].get()));
	}

	// Test 2: actual keyword
	l = Lexer(keyword_str, "", &e);
	tokens = l.lex();
	EXPECT_EQ(tokens.size(), 2);
	EXPECT_EQ(dynamic_cast<Keyword *>(tokens[0].get())->type, keyword_type);
	EXPECT_TRUE(dynamic_cast<EndOfFile *>(tokens[1].get()));

	// Test 3: alphanumeric suffixes
	for (const auto &suffix : {"x", "0"}) {
		const std::string program = keyword_str + suffix;
		l = Lexer(program, "", &e);
		tokens = l.lex();
		EXPECT_EQ(tokens.size(), 2);
		EXPECT_FALSE(dynamic_cast<Keyword *>(tokens[0].get()));
		EXPECT_TRUE(dynamic_cast<EndOfFile *>(tokens[1].get()));
	}

	// Test 4: whitespace suffixes
	for (const auto suffix : whitespaces) {
		const std::string program = keyword_str + suffix;
		l = Lexer(program, "", &e);
		tokens = l.lex();
		EXPECT_EQ(tokens.size(), 2);
		EXPECT_EQ(dynamic_cast<Keyword *>(tokens[0].get())->type, keyword_type);
		EXPECT_TRUE(dynamic_cast<EndOfFile *>(tokens[1].get()));
	}

	// Test 5: whitespace prefixes
	for (const auto prefix : whitespaces) {
		const std::string program = prefix + keyword_str;
		l = Lexer(program, "", &e);
		tokens = l.lex();
		EXPECT_EQ(tokens.size(), 2);
		EXPECT_EQ(dynamic_cast<Keyword *>(tokens[0].get())->type, keyword_type);
		EXPECT_TRUE(dynamic_cast<EndOfFile *>(tokens[1].get()));
	}

	// Test 6: punctuation suffixes
	for (const auto &suffix : punctuations) {
		const char c = suffix.first;
		const std::string program = keyword_str + c;
		l = Lexer(program, "", &e);
		tokens = l.lex();
		EXPECT_EQ(tokens.size(), 3);
		EXPECT_EQ(dynamic_cast<Keyword *>(tokens[0].get())->type, keyword_type);
		EXPECT_TRUE(dynamic_cast<Punctuation *>(tokens[1].get())
		            || dynamic_cast<Operator *>(tokens[1].get()));
		EXPECT_TRUE(dynamic_cast<EndOfFile *>(tokens[2].get()));
	}

	// Test 7: punctuation prefixes
	for (const auto &prefix : punctuations) {
		const std::string program = prefix.first + keyword_str;
		l = Lexer(program, "", &e);
		tokens = l.lex();
		EXPECT_EQ(tokens.size(), 3);
		EXPECT_TRUE(dynamic_cast<Punctuation *>(tokens[0].get())
		            || dynamic_cast<Operator *>(tokens[0].get()));
		EXPECT_EQ(dynamic_cast<Keyword *>(tokens[1].get())->type, keyword_type);
		EXPECT_TRUE(dynamic_cast<EndOfFile *>(tokens[2].get()));
	}
}

INSTANTIATE_TEST_SUITE_P(testKeywords, TestLexerKeywords,
      testing::Values(std::pair{"return", Keyword::Type::RETURN},
            std::pair{"let", Keyword::Type::LET}, std::pair{"fun", Keyword::Type::FUN},
            std::pair{"if", Keyword::Type::IF}, std::pair{"else", Keyword::Type::ELSE},
            std::pair{"while", Keyword::Type::WHILE}));

TEST_P(TestLexerSymbols, testSymbols) {
	const std::string symbol = GetParam();
	// Test 1: Single symbol
	l = Lexer(symbol, "", &e);
	tokens = l.lex();
	EXPECT_EQ(tokens.size(), 2);
	EXPECT_EQ(dynamic_cast<Symbol *>(tokens[0].get())->s.contents, symbol);
	EXPECT_TRUE(dynamic_cast<EndOfFile *>(tokens[1].get()));

	// Test 2: whitespace suffixes
	for (const auto suffix : whitespaces) {
		const std::string program = symbol + suffix;
		l = Lexer(program, "", &e);
		tokens = l.lex();
		EXPECT_EQ(tokens.size(), 2);
		EXPECT_EQ(dynamic_cast<Symbol *>(tokens[0].get())->s.contents, symbol);
		EXPECT_TRUE(dynamic_cast<EndOfFile *>(tokens[1].get()));
	}

	// Test 3: whitespace prefixes
	for (const auto prefix : whitespaces) {
		const std::string program = prefix + symbol;
		l = Lexer(program, "", &e);
		tokens = l.lex();
		EXPECT_EQ(tokens.size(), 2);
		EXPECT_EQ(dynamic_cast<Symbol *>(tokens[0].get())->s.contents, symbol);
		EXPECT_TRUE(dynamic_cast<EndOfFile *>(tokens[1].get()));
	}

	// Test 4: punctuation suffixes
	for (const auto &suffix : punctuations) {
		const std::string program = symbol + suffix.first;
		l = Lexer(program, "", &e);
		tokens = l.lex();
		EXPECT_EQ(tokens.size(), 3);
		EXPECT_EQ(dynamic_cast<Symbol *>(tokens[0].get())->s.contents, symbol);
		EXPECT_TRUE(dynamic_cast<Punctuation *>(tokens[1].get())
		            || dynamic_cast<Operator *>(tokens[1].get()));
		EXPECT_TRUE(dynamic_cast<EndOfFile *>(tokens[2].get()));
	}

	// Test 5: punctuation prefixes
	for (const auto &prefix : punctuations) {
		const std::string program = prefix.first + symbol;
		l = Lexer(program, "", &e);
		tokens = l.lex();
		EXPECT_EQ(tokens.size(), 3);
		EXPECT_TRUE(dynamic_cast<Punctuation *>(tokens[0].get())
		            || dynamic_cast<Operator *>(tokens[0].get()));
		EXPECT_EQ(dynamic_cast<Symbol *>(tokens[1].get())->s.contents, symbol);
		EXPECT_TRUE(dynamic_cast<EndOfFile *>(tokens[2].get()));
	}
}

INSTANTIATE_TEST_SUITE_P(testSymbols, TestLexerSymbols,
      testing::Values("x", "x0", "abcd", "abcd1234", "a1b2c3d4", "abcd1234abcd",
            "abcd1234abcd", "a1b2c3d4abcd"));

TEST_P(TestLexerLiterals, testLiterals) {
	// Test 1: Single literals
	const std::string literal_str = GetParam().first;
	const IntegerLiteral expected = GetParam().second;
	l = Lexer(literal_str, "", &e);
	tokens = l.lex();
	EXPECT_EQ(tokens.size(), 2);
	EXPECT_EQ(*dynamic_cast<IntegerLiteral *>(tokens[0].get()), expected);
	EXPECT_TRUE(dynamic_cast<EndOfFile *>(tokens[1].get()));

	// Test 2: whitespace suffixes
	for (const auto suffix : whitespaces) {
		const std::string program = literal_str + suffix;
		l = Lexer(program, "", &e);
		tokens = l.lex();
		EXPECT_EQ(tokens.size(), 2);
		EXPECT_EQ(*dynamic_cast<IntegerLiteral *>(tokens[0].get()), expected);
		EXPECT_TRUE(dynamic_cast<EndOfFile *>(tokens[1].get()));
	}

	// Test 3: whitespace prefixes
	for (const auto prefix : whitespaces) {
		const std::string program = prefix + literal_str;
		l = Lexer(program, "", &e);
		tokens = l.lex();
		EXPECT_EQ(tokens.size(), 2);
		EXPECT_EQ(*dynamic_cast<IntegerLiteral *>(tokens[0].get()), expected);
		EXPECT_TRUE(dynamic_cast<EndOfFile *>(tokens[1].get()));
	}

	// Test 4: punctuation suffixes
	for (const auto &suffix : punctuations) {
		const std::string program = literal_str + suffix.first;
		l = Lexer(program, "", &e);
		tokens = l.lex();
		EXPECT_EQ(tokens.size(), 3);
		EXPECT_EQ(*dynamic_cast<IntegerLiteral *>(tokens[0].get()), expected);
		EXPECT_TRUE(dynamic_cast<Punctuation *>(tokens[1].get())
		            || dynamic_cast<Operator *>(tokens[1].get()));
		EXPECT_TRUE(dynamic_cast<EndOfFile *>(tokens[2].get()));
	}

	// Test 5: punctuation prefixes
	for (const auto &prefix : punctuations) {
		const std::string program = prefix.first + literal_str;
		l = Lexer(program, "", &e);
		tokens = l.lex();
		EXPECT_EQ(tokens.size(), 3);
		EXPECT_TRUE(dynamic_cast<Punctuation *>(tokens[0].get())
		            || dynamic_cast<Operator *>(tokens[0].get()));
		EXPECT_EQ(*dynamic_cast<IntegerLiteral *>(tokens[1].get()), expected);
		EXPECT_TRUE(dynamic_cast<EndOfFile *>(tokens[2].get()));
	}
}

class DummyToken : public Token {
public:
	DummyToken() noexcept : Token(Slice("", "", 0, 0)) {
	}

	void print([[maybe_unused]] std::ostream &os) const override {
	}
};

static const DummyToken dummy;

INSTANTIATE_TEST_SUITE_P(testDecimalLiterals, TestLexerLiterals,
      testing::Values(
            std::pair("1", IntegerLiteral(dummy, IntegerLiteral::Type::I32, 1UL)),
            std::pair("18", IntegerLiteral(dummy, IntegerLiteral::Type::I32, 18UL)),
            std::pair("0", IntegerLiteral(dummy, IntegerLiteral::Type::I32, 0UL)),
            std::pair("00", IntegerLiteral(dummy, IntegerLiteral::Type::I32, 0UL)),
            std::pair("01", IntegerLiteral(dummy, IntegerLiteral::Type::I32, 1UL)),
            std::pair("001", IntegerLiteral(dummy, IntegerLiteral::Type::I32, 1UL)),
            std::pair("2147483647",
                  IntegerLiteral(dummy, IntegerLiteral::Type::I32, 2'147'483'647UL)),
            std::pair("02147483647",
                  IntegerLiteral(dummy, IntegerLiteral::Type::I32, 2'147'483'647UL)),
            std::pair("128i8",
                  IntegerLiteral(dummy, IntegerLiteral::Type::I8, 128UL)), // i8_MIN
            std::pair("127i8", IntegerLiteral(dummy, IntegerLiteral::Type::I8, 127UL)),
            std::pair("0127i8", IntegerLiteral(dummy, IntegerLiteral::Type::I8, 127UL)),
            std::pair("0u8", IntegerLiteral(dummy, IntegerLiteral::Type::U8, 0UL)),
            std::pair("00u8", IntegerLiteral(dummy, IntegerLiteral::Type::U8, 0UL)),
            std::pair("255u8", IntegerLiteral(dummy, IntegerLiteral::Type::U8, 255UL)),
            std::pair("0255u8", IntegerLiteral(dummy, IntegerLiteral::Type::U8, 255UL)),
            std::pair("32768i16",
                  IntegerLiteral(dummy, IntegerLiteral::Type::I16, 32768UL)), // i16_MIN
            std::pair("32767i16",
                  IntegerLiteral(dummy, IntegerLiteral::Type::I16, 32767UL)),
            std::pair("032767i16",
                  IntegerLiteral(dummy, IntegerLiteral::Type::I16, 32767UL)),
            std::pair("0u16", IntegerLiteral(dummy, IntegerLiteral::Type::U16, 0UL)),
            std::pair("00u16", IntegerLiteral(dummy, IntegerLiteral::Type::U16, 0UL)),
            std::pair("65535u16",
                  IntegerLiteral(dummy, IntegerLiteral::Type::U16, 65535UL)),
            std::pair("065535u16",
                  IntegerLiteral(dummy, IntegerLiteral::Type::U16, 65535UL)),
            std::pair("2147483648i32", IntegerLiteral(dummy, IntegerLiteral::Type::I32,
                                             2'147'483'648UL)), // i32_MIN
            std::pair("2147483647i32",
                  IntegerLiteral(dummy, IntegerLiteral::Type::I32, 2'147'483'647UL)),
            std::pair("02147483647i32",
                  IntegerLiteral(dummy, IntegerLiteral::Type::I32, 2'147'483'647UL)),
            std::pair("0u32", IntegerLiteral(dummy, IntegerLiteral::Type::U32, 0UL)),
            std::pair("00u32", IntegerLiteral(dummy, IntegerLiteral::Type::U32, 0UL)),
            std::pair("4294967295u32",
                  IntegerLiteral(dummy, IntegerLiteral::Type::U32, 4'294'967'295UL)),
            std::pair("04294967295u32",
                  IntegerLiteral(dummy, IntegerLiteral::Type::U32, 4'294'967'295UL)),
            std::pair("9223372036854775808i64",
                  IntegerLiteral(dummy, IntegerLiteral::Type::I64,
                        9'223'372'036'854'775'808UL)), // i64_MIN
            std::pair("9223372036854775807i64",
                  IntegerLiteral(dummy, IntegerLiteral::Type::I64,
                        9'223'372'036'854'775'807UL)),
            std::pair("09223372036854775807i64",
                  IntegerLiteral(dummy, IntegerLiteral::Type::I64,
                        9'223'372'036'854'775'807UL)),
            std::pair("0u64", IntegerLiteral(dummy, IntegerLiteral::Type::U64, 0UL)),
            std::pair("00u64", IntegerLiteral(dummy, IntegerLiteral::Type::U64, 0UL)),
            std::pair("18446744073709551615u64",
                  IntegerLiteral(dummy, IntegerLiteral::Type::U64,
                        18'446'744'073'709'551'615UL)),
            std::pair("018446744073709551615u64",
                  IntegerLiteral(dummy, IntegerLiteral::Type::U64,
                        18'446'744'073'709'551'615UL))));
INSTANTIATE_TEST_SUITE_P(testHexLiterals, TestLexerLiterals,
      testing::Values(
            std::pair("0x1", IntegerLiteral(dummy, IntegerLiteral::Type::I32, 1UL)),
            std::pair("0x12", IntegerLiteral(dummy, IntegerLiteral::Type::I32, 18UL)),
            std::pair("0x0", IntegerLiteral(dummy, IntegerLiteral::Type::I32, 0UL)),
            std::pair("0x00", IntegerLiteral(dummy, IntegerLiteral::Type::I32, 0UL)),
            std::pair("0x01", IntegerLiteral(dummy, IntegerLiteral::Type::I32, 1UL)),
            std::pair("0x001", IntegerLiteral(dummy, IntegerLiteral::Type::I32, 1UL)),
            std::pair("0xa", IntegerLiteral(dummy, IntegerLiteral::Type::I32, 10UL)),
            std::pair("0xA", IntegerLiteral(dummy, IntegerLiteral::Type::I32, 10UL)),
            std::pair("0xf", IntegerLiteral(dummy, IntegerLiteral::Type::I32, 15UL)),
            std::pair("0xF", IntegerLiteral(dummy, IntegerLiteral::Type::I32, 15UL)),
            std::pair("0x7FFFFFFF",
                  IntegerLiteral(dummy, IntegerLiteral::Type::I32, 2'147'483'647UL)),
            std::pair("0x07FFFFFFF",
                  IntegerLiteral(dummy, IntegerLiteral::Type::I32, 2'147'483'647UL)),
            std::pair("0x80i8",
                  IntegerLiteral(dummy, IntegerLiteral::Type::I8, 128UL)), // i8_MIN
            std::pair("0x7Fi8", IntegerLiteral(dummy, IntegerLiteral::Type::I8, 127UL)),
            std::pair("0x07Fi8", IntegerLiteral(dummy, IntegerLiteral::Type::I8, 127UL)),
            std::pair("0x0u8", IntegerLiteral(dummy, IntegerLiteral::Type::U8, 0UL)),
            std::pair("0x00u8", IntegerLiteral(dummy, IntegerLiteral::Type::U8, 0UL)),
            std::pair("0xFFu8", IntegerLiteral(dummy, IntegerLiteral::Type::U8, 255UL)),
            std::pair("0x0FFu8", IntegerLiteral(dummy, IntegerLiteral::Type::U8, 255UL)),
            std::pair("0x8000i16",
                  IntegerLiteral(dummy, IntegerLiteral::Type::I16, 32768UL)), // i16_MIN
            std::pair("0x7FFFi16",
                  IntegerLiteral(dummy, IntegerLiteral::Type::I16, 32767UL)),
            std::pair("0x07FFFi16",
                  IntegerLiteral(dummy, IntegerLiteral::Type::I16, 32767UL)),
            std::pair("0x0u16", IntegerLiteral(dummy, IntegerLiteral::Type::U16, 0UL)),
            std::pair("0x00u16", IntegerLiteral(dummy, IntegerLiteral::Type::U16, 0UL)),
            std::pair("0xFFFFu16",
                  IntegerLiteral(dummy, IntegerLiteral::Type::U16, 65535UL)),
            std::pair("0x0FFFFu16",
                  IntegerLiteral(dummy, IntegerLiteral::Type::U16, 65535UL)),
            std::pair("0x80000000i32", IntegerLiteral(dummy, IntegerLiteral::Type::I32,
                                             2'147'483'648UL)), // i32_MIN
            std::pair("0x7FFFFFFFi32",
                  IntegerLiteral(dummy, IntegerLiteral::Type::I32, 2'147'483'647UL)),
            std::pair("0x07FFFFFFFi32",
                  IntegerLiteral(dummy, IntegerLiteral::Type::I32, 2'147'483'647UL)),
            std::pair("0x0u32", IntegerLiteral(dummy, IntegerLiteral::Type::U32, 0UL)),
            std::pair("0x00u32", IntegerLiteral(dummy, IntegerLiteral::Type::U32, 0UL)),
            std::pair("0xFFFFFFFFu32",
                  IntegerLiteral(dummy, IntegerLiteral::Type::U32, 4'294'967'295UL)),
            std::pair("0x0FFFFFFFFu32",
                  IntegerLiteral(dummy, IntegerLiteral::Type::U32, 4'294'967'295UL)),
            std::pair("0x8000000000000000i64",
                  IntegerLiteral(dummy, IntegerLiteral::Type::I64,
                        9'223'372'036'854'775'808UL)), // i64_MIN
            std::pair("0x7FFFFFFFFFFFFFFFi64",
                  IntegerLiteral(dummy, IntegerLiteral::Type::I64,
                        9'223'372'036'854'775'807UL)),
            std::pair("0x07FFFFFFFFFFFFFFFi64",
                  IntegerLiteral(dummy, IntegerLiteral::Type::I64,
                        9'223'372'036'854'775'807UL)),
            std::pair("0x0u64", IntegerLiteral(dummy, IntegerLiteral::Type::U64, 0UL)),
            std::pair("0x00u64", IntegerLiteral(dummy, IntegerLiteral::Type::U64, 0UL)),
            std::pair("0xFFFFFFFFFFFFFFFFu64",
                  IntegerLiteral(dummy, IntegerLiteral::Type::U64,
                        18'446'744'073'709'551'615UL)),
            std::pair("0x0FFFFFFFFFFFFFFFFu64",
                  IntegerLiteral(dummy, IntegerLiteral::Type::U64,
                        18'446'744'073'709'551'615UL))));
INSTANTIATE_TEST_SUITE_P(testOctalLiterals, TestLexerLiterals,
      testing::Values(
            std::pair("0o1", IntegerLiteral(dummy, IntegerLiteral::Type::I32, 1UL)),
            std::pair("0o22", IntegerLiteral(dummy, IntegerLiteral::Type::I32, 18UL)),
            std::pair("0o0", IntegerLiteral(dummy, IntegerLiteral::Type::I32, 0UL)),
            std::pair("0o00", IntegerLiteral(dummy, IntegerLiteral::Type::I32, 0UL)),
            std::pair("0o01", IntegerLiteral(dummy, IntegerLiteral::Type::I32, 1UL)),
            std::pair("0o001", IntegerLiteral(dummy, IntegerLiteral::Type::I32, 1UL)),
            std::pair("0o17777777777",
                  IntegerLiteral(dummy, IntegerLiteral::Type::I32, 2'147'483'647UL)),
            std::pair("0o017777777777",
                  IntegerLiteral(dummy, IntegerLiteral::Type::I32, 2'147'483'647UL)),
            std::pair("0o200i8",
                  IntegerLiteral(dummy, IntegerLiteral::Type::I8, 128UL)), // i8_MIN
            std::pair("0o177i8", IntegerLiteral(dummy, IntegerLiteral::Type::I8, 127UL)),
            std::pair("0o0177i8", IntegerLiteral(dummy, IntegerLiteral::Type::I8, 127UL)),
            std::pair("0o0u8", IntegerLiteral(dummy, IntegerLiteral::Type::U8, 0UL)),
            std::pair("0o00u8", IntegerLiteral(dummy, IntegerLiteral::Type::U8, 0UL)),
            std::pair("0o377u8", IntegerLiteral(dummy, IntegerLiteral::Type::U8, 255UL)),
            std::pair("0o0377u8", IntegerLiteral(dummy, IntegerLiteral::Type::U8, 255UL)),
            std::pair("0o100000i16",
                  IntegerLiteral(dummy, IntegerLiteral::Type::I16, 32768UL)), // i16_MIN
            std::pair("0o77777i16",
                  IntegerLiteral(dummy, IntegerLiteral::Type::I16, 32767UL)),
            std::pair("0o077777i16",
                  IntegerLiteral(dummy, IntegerLiteral::Type::I16, 32767UL)),
            std::pair("0o0u16", IntegerLiteral(dummy, IntegerLiteral::Type::U16, 0UL)),
            std::pair("0o00u16", IntegerLiteral(dummy, IntegerLiteral::Type::U16, 0UL)),
            std::pair("0o177777u16",
                  IntegerLiteral(dummy, IntegerLiteral::Type::U16, 65535UL)),
            std::pair("0o0177777u16",
                  IntegerLiteral(dummy, IntegerLiteral::Type::U16, 65535UL)),
            std::pair("0o20000000000i32", IntegerLiteral(dummy, IntegerLiteral::Type::I32,
                                                2'147'483'648UL)), // i32_MIN
            std::pair("0o17777777777i32",
                  IntegerLiteral(dummy, IntegerLiteral::Type::I32, 2'147'483'647UL)),
            std::pair("0o017777777777i32",
                  IntegerLiteral(dummy, IntegerLiteral::Type::I32, 2'147'483'647UL)),
            std::pair("0o0u32", IntegerLiteral(dummy, IntegerLiteral::Type::U32, 0UL)),
            std::pair("0o00u32", IntegerLiteral(dummy, IntegerLiteral::Type::U32, 0UL)),
            std::pair("0o37777777777u32",
                  IntegerLiteral(dummy, IntegerLiteral::Type::U32, 4'294'967'295UL)),
            std::pair("0o037777777777u32",
                  IntegerLiteral(dummy, IntegerLiteral::Type::U32, 4'294'967'295UL)),
            std::pair("0o1000000000000000000000i64",
                  IntegerLiteral(dummy, IntegerLiteral::Type::I64,
                        9'223'372'036'854'775'808UL)), // i64_MIN
            std::pair("0o777777777777777777777i64",
                  IntegerLiteral(dummy, IntegerLiteral::Type::I64,
                        9'223'372'036'854'775'807UL)),
            std::pair("0o0777777777777777777777i64",
                  IntegerLiteral(dummy, IntegerLiteral::Type::I64,
                        9'223'372'036'854'775'807UL)),
            std::pair("0o0u64", IntegerLiteral(dummy, IntegerLiteral::Type::U64, 0UL)),
            std::pair("0o00u64", IntegerLiteral(dummy, IntegerLiteral::Type::U64, 0UL)),
            std::pair("0o1777777777777777777777u64",
                  IntegerLiteral(dummy, IntegerLiteral::Type::U64,
                        18'446'744'073'709'551'615UL)),
            std::pair("0o01777777777777777777777u64",
                  IntegerLiteral(dummy, IntegerLiteral::Type::U64,
                        18'446'744'073'709'551'615UL))));
INSTANTIATE_TEST_SUITE_P(testBinaryLiterals, TestLexerLiterals,
      testing::Values(
            std::pair("0b1", IntegerLiteral(dummy, IntegerLiteral::Type::I32, 1UL)),
            std::pair("0b10010", IntegerLiteral(dummy, IntegerLiteral::Type::I32, 18UL)),
            std::pair("0b0", IntegerLiteral(dummy, IntegerLiteral::Type::I32, 0UL)),
            std::pair("0b00", IntegerLiteral(dummy, IntegerLiteral::Type::I32, 0UL)),
            std::pair("0b01", IntegerLiteral(dummy, IntegerLiteral::Type::I32, 1UL)),
            std::pair("0b001", IntegerLiteral(dummy, IntegerLiteral::Type::I32, 1UL)),
            std::pair("0b1111111111111111111111111111111",
                  IntegerLiteral(dummy, IntegerLiteral::Type::I32, 2'147'483'647UL)),
            std::pair("0b01111111111111111111111111111111",
                  IntegerLiteral(dummy, IntegerLiteral::Type::I32, 2'147'483'647UL)),
            std::pair("0b10000000i8",
                  IntegerLiteral(dummy, IntegerLiteral::Type::I8, 128UL)), // i8_MIN
            std::pair("0b1111111i8",
                  IntegerLiteral(dummy, IntegerLiteral::Type::I8, 127UL)),
            std::pair("0b01111111i8",
                  IntegerLiteral(dummy, IntegerLiteral::Type::I8, 127UL)),
            std::pair("0b0u8", IntegerLiteral(dummy, IntegerLiteral::Type::U8, 0UL)),
            std::pair("0b00u8", IntegerLiteral(dummy, IntegerLiteral::Type::U8, 0UL)),
            std::pair("0b11111111u8",
                  IntegerLiteral(dummy, IntegerLiteral::Type::U8, 255UL)),
            std::pair("0b011111111u8",
                  IntegerLiteral(dummy, IntegerLiteral::Type::U8, 255UL)),
            std::pair("0b1000000000000000i16",
                  IntegerLiteral(dummy, IntegerLiteral::Type::I16, 32768UL)), // i16_MIN
            std::pair("0b111111111111111i16",
                  IntegerLiteral(dummy, IntegerLiteral::Type::I16, 32767UL)),
            std::pair("0b0111111111111111i16",
                  IntegerLiteral(dummy, IntegerLiteral::Type::I16, 32767UL)),
            std::pair("0b0u16", IntegerLiteral(dummy, IntegerLiteral::Type::U16, 0UL)),
            std::pair("0b00u16", IntegerLiteral(dummy, IntegerLiteral::Type::U16, 0UL)),
            std::pair("0b1111111111111111u16",
                  IntegerLiteral(dummy, IntegerLiteral::Type::U16, 65535UL)),
            std::pair("0b01111111111111111u16",
                  IntegerLiteral(dummy, IntegerLiteral::Type::U16, 65535UL)),
            std::pair("0b10000000000000000000000000000000i32",
                  IntegerLiteral(dummy, IntegerLiteral::Type::I32,
                        2'147'483'648UL)), // i32_MIN
            std::pair("0b1111111111111111111111111111111i32",
                  IntegerLiteral(dummy, IntegerLiteral::Type::I32, 2'147'483'647UL)),
            std::pair("0b01111111111111111111111111111111i32",
                  IntegerLiteral(dummy, IntegerLiteral::Type::I32, 2'147'483'647UL)),
            std::pair("0b0u32", IntegerLiteral(dummy, IntegerLiteral::Type::U32, 0UL)),
            std::pair("0b00u32", IntegerLiteral(dummy, IntegerLiteral::Type::U32, 0UL)),
            std::pair("0b11111111111111111111111111111111u32",
                  IntegerLiteral(dummy, IntegerLiteral::Type::U32, 4'294'967'295UL)),
            std::pair("0b011111111111111111111111111111111u32",
                  IntegerLiteral(dummy, IntegerLiteral::Type::U32, 4'294'967'295UL)),
            std::pair(
                  "0b1000000000000000000000000000000000000000000000000000000000000000i64",
                  IntegerLiteral(dummy, IntegerLiteral::Type::I64,
                        9'223'372'036'854'775'808UL)), // i64_MIN
            std::pair(
                  "0b111111111111111111111111111111111111111111111111111111111111111i64",
                  IntegerLiteral(dummy, IntegerLiteral::Type::I64,
                        9'223'372'036'854'775'807UL)),
            std::pair(
                  "0b0111111111111111111111111111111111111111111111111111111111111111i64",
                  IntegerLiteral(dummy, IntegerLiteral::Type::I64,
                        9'223'372'036'854'775'807UL)),
            std::pair("0b0u64", IntegerLiteral(dummy, IntegerLiteral::Type::U64, 0UL)),
            std::pair("0b00u64", IntegerLiteral(dummy, IntegerLiteral::Type::U64, 0UL)),
            std::pair(
                  "0b1111111111111111111111111111111111111111111111111111111111111111u64",
                  IntegerLiteral(dummy, IntegerLiteral::Type::U64,
                        18'446'744'073'709'551'615UL)),
            std::pair("0b0111111111111111111111111111111111111111111111111111111111111111"
                      "1u64",
                  IntegerLiteral(dummy, IntegerLiteral::Type::U64,
                        18'446'744'073'709'551'615UL))));

TEST_P(TestLexerInvalidLiterals, testLiteralErrors) {
	const std::string literal_str = GetParam().first;
	const auto &expect = GetParam().second;
	l = Lexer(literal_str, "", &e);
	tokens = l.lex();
	EXPECT_EQ(tokens.size(), 2);
	EXPECT_TRUE(dynamic_cast<EndOfFile *>(tokens[1].get()));
	std::queue<std::tuple<std::filesystem::path, size_t, size_t, std::string>> expected;
	expected.push(expect);
	e.checkErrors(expected);
}

INSTANTIATE_TEST_SUITE_P(testInvalidDecimalLiterals, TestLexerInvalidLiterals,
      Values(std::pair("1q", std::tuple("", 1, 1, "Invalid integer literal")),
            std::pair("18485h746", std::tuple("", 1, 1, "Invalid integer literal")),
            std::pair("1a", std::tuple("", 1, 1, "Invalid integer literal")),
            std::pair("1f", std::tuple("", 1, 1, "Invalid integer literal")),
            std::pair("1i", std::tuple("", 1, 1, "Invalid integer literal")),
            std::pair("1u", std::tuple("", 1, 1, "Invalid integer literal")),
            std::pair("1i1", std::tuple("", 1, 1, "Invalid integer literal")),
            std::pair("1u1", std::tuple("", 1, 1, "Invalid integer literal")),
            std::pair("1i3", std::tuple("", 1, 1, "Invalid integer literal")),
            std::pair("1u3", std::tuple("", 1, 1, "Invalid integer literal")),
            std::pair("1i6", std::tuple("", 1, 1, "Invalid integer literal")),
            std::pair("1u6", std::tuple("", 1, 1, "Invalid integer literal")),
            std::pair("1i80", std::tuple("", 1, 1, "Invalid integer literal")),
            std::pair("1u80", std::tuple("", 1, 1, "Invalid integer literal")),
            std::pair("1i160", std::tuple("", 1, 1, "Invalid integer literal")),
            std::pair("1u160", std::tuple("", 1, 1, "Invalid integer literal")),
            std::pair("1i320", std::tuple("", 1, 1, "Invalid integer literal")),
            std::pair("1u320", std::tuple("", 1, 1, "Invalid integer literal")),
            std::pair("1i640", std::tuple("", 1, 1, "Invalid integer literal")),
            std::pair("1u640", std::tuple("", 1, 1, "Invalid integer literal"))));
INSTANTIATE_TEST_SUITE_P(testInvalidHexLiterals, TestLexerInvalidLiterals,
      Values(std::pair("0xq", std::tuple("", 1, 1, "Invalid integer literal")),
            std::pair("0x1q", std::tuple("", 1, 1, "Invalid integer literal")),
            std::pair("0xg", std::tuple("", 1, 1, "Invalid integer literal")),
            std::pair("0x1i1", std::tuple("", 1, 1, "Invalid integer literal")),
            std::pair("0x1u1", std::tuple("", 1, 1, "Invalid integer literal")),
            std::pair("0x1i3", std::tuple("", 1, 1, "Invalid integer literal")),
            std::pair("0x1u3", std::tuple("", 1, 1, "Invalid integer literal")),
            std::pair("0x1i6", std::tuple("", 1, 1, "Invalid integer literal")),
            std::pair("0x1u6", std::tuple("", 1, 1, "Invalid integer literal")),
            std::pair("0x1i80", std::tuple("", 1, 1, "Invalid integer literal")),
            std::pair("0x1u80", std::tuple("", 1, 1, "Invalid integer literal")),
            std::pair("0x1i160", std::tuple("", 1, 1, "Invalid integer literal")),
            std::pair("0x1u160", std::tuple("", 1, 1, "Invalid integer literal")),
            std::pair("0x1i320", std::tuple("", 1, 1, "Invalid integer literal")),
            std::pair("0x1u320", std::tuple("", 1, 1, "Invalid integer literal")),
            std::pair("0x1i640", std::tuple("", 1, 1, "Invalid integer literal")),
            std::pair("0x1u640", std::tuple("", 1, 1, "Invalid integer literal"))));
INSTANTIATE_TEST_SUITE_P(testInvalidOctalLiterals, TestLexerInvalidLiterals,
      Values(std::pair("0oq", std::tuple("", 1, 1, "Invalid integer literal")),
            std::pair("0o1q", std::tuple("", 1, 1, "Invalid integer literal")),
            std::pair("0o8", std::tuple("", 1, 1, "Invalid integer literal")),
            std::pair("0o9", std::tuple("", 1, 1, "Invalid integer literal")),
            std::pair("0oa", std::tuple("", 1, 1, "Invalid integer literal")),
            std::pair("0of", std::tuple("", 1, 1, "Invalid integer literal")),
            std::pair("0o1i1", std::tuple("", 1, 1, "Invalid integer literal")),
            std::pair("0o1u1", std::tuple("", 1, 1, "Invalid integer literal")),
            std::pair("0o1i3", std::tuple("", 1, 1, "Invalid integer literal")),
            std::pair("0o1u3", std::tuple("", 1, 1, "Invalid integer literal")),
            std::pair("0o1i6", std::tuple("", 1, 1, "Invalid integer literal")),
            std::pair("0o1u6", std::tuple("", 1, 1, "Invalid integer literal")),
            std::pair("0o1i80", std::tuple("", 1, 1, "Invalid integer literal")),
            std::pair("0o1u80", std::tuple("", 1, 1, "Invalid integer literal")),
            std::pair("0o1i160", std::tuple("", 1, 1, "Invalid integer literal")),
            std::pair("0o1u160", std::tuple("", 1, 1, "Invalid integer literal")),
            std::pair("0o1i320", std::tuple("", 1, 1, "Invalid integer literal")),
            std::pair("0o1u320", std::tuple("", 1, 1, "Invalid integer literal")),
            std::pair("0o1i640", std::tuple("", 1, 1, "Invalid integer literal")),
            std::pair("0o1u640", std::tuple("", 1, 1, "Invalid integer literal"))));
INSTANTIATE_TEST_SUITE_P(testInvalidBinaryLiterals, TestLexerInvalidLiterals,
      Values(std::pair("0bq", std::tuple("", 1, 1, "Invalid integer literal")),
            std::pair("0b1q", std::tuple("", 1, 1, "Invalid integer literal")),
            std::pair("0b2", std::tuple("", 1, 1, "Invalid integer literal")),
            std::pair("0b7", std::tuple("", 1, 1, "Invalid integer literal")),
            std::pair("0b8", std::tuple("", 1, 1, "Invalid integer literal")),
            std::pair("0b9", std::tuple("", 1, 1, "Invalid integer literal")),
            std::pair("0ba", std::tuple("", 1, 1, "Invalid integer literal")),
            std::pair("0bf", std::tuple("", 1, 1, "Invalid integer literal")),
            std::pair("0b1i1", std::tuple("", 1, 1, "Invalid integer literal")),
            std::pair("0b1u1", std::tuple("", 1, 1, "Invalid integer literal")),
            std::pair("0b1i3", std::tuple("", 1, 1, "Invalid integer literal")),
            std::pair("0b1u3", std::tuple("", 1, 1, "Invalid integer literal")),
            std::pair("0b1i6", std::tuple("", 1, 1, "Invalid integer literal")),
            std::pair("0b1u6", std::tuple("", 1, 1, "Invalid integer literal")),
            std::pair("0b1i80", std::tuple("", 1, 1, "Invalid integer literal")),
            std::pair("0b1u80", std::tuple("", 1, 1, "Invalid integer literal")),
            std::pair("0b1i160", std::tuple("", 1, 1, "Invalid integer literal")),
            std::pair("0b1u160", std::tuple("", 1, 1, "Invalid integer literal")),
            std::pair("0b1i320", std::tuple("", 1, 1, "Invalid integer literal")),
            std::pair("0b1u320", std::tuple("", 1, 1, "Invalid integer literal")),
            std::pair("0b1i640", std::tuple("", 1, 1, "Invalid integer literal")),
            std::pair("0b1u640", std::tuple("", 1, 1, "Invalid integer literal"))));

TEST_F(TestLexer, testNewlines) {
	std::string program = "x";
	l = Lexer(program, "", &e);
	tokens = l.lex();
	EXPECT_EQ(tokens.size(), 2);
	EXPECT_EQ(tokens[0]->s.row, 1);
	EXPECT_EQ(tokens[0]->s.col, 1);
	EXPECT_TRUE(dynamic_cast<EndOfFile *>(tokens[1].get()));
	program = "x\n";
	l = Lexer(program, "", &e);
	tokens = l.lex();
	EXPECT_EQ(tokens.size(), 2);
	EXPECT_EQ(tokens[0]->s.row, 1);
	EXPECT_EQ(tokens[0]->s.col, 1);
	EXPECT_TRUE(dynamic_cast<EndOfFile *>(tokens[1].get()));
	program = "x\n\n";
	l = Lexer(program, "", &e);
	tokens = l.lex();
	EXPECT_EQ(tokens.size(), 2);
	EXPECT_EQ(tokens[0]->s.row, 1);
	EXPECT_EQ(tokens[0]->s.col, 1);
	EXPECT_TRUE(dynamic_cast<EndOfFile *>(tokens[1].get()));

	program = "\nx";
	l = Lexer(program, "", &e);
	tokens = l.lex();
	EXPECT_EQ(tokens.size(), 2);
	EXPECT_EQ(tokens[0]->s.row, 2);
	EXPECT_EQ(tokens[0]->s.col, 1);
	EXPECT_TRUE(dynamic_cast<EndOfFile *>(tokens[1].get()));
	program = "\n\nx";
	l = Lexer(program, "", &e);
	tokens = l.lex();
	EXPECT_EQ(tokens.size(), 2);
	EXPECT_EQ(tokens[0]->s.row, 3);
	EXPECT_EQ(tokens[0]->s.col, 1);
	EXPECT_TRUE(dynamic_cast<EndOfFile *>(tokens[1].get()));

	program = "x\ny";
	l = Lexer(program, "", &e);
	tokens = l.lex();
	EXPECT_EQ(tokens.size(), 3);
	EXPECT_EQ(tokens[0]->s.row, 1);
	EXPECT_EQ(tokens[0]->s.col, 1);
	EXPECT_EQ(tokens[1]->s.row, 2);
	EXPECT_EQ(tokens[1]->s.col, 1);
	EXPECT_TRUE(dynamic_cast<EndOfFile *>(tokens[2].get()));
	program = "x\n\ny";
	l = Lexer(program, "", &e);
	tokens = l.lex();
	EXPECT_EQ(tokens.size(), 3);
	EXPECT_EQ(tokens[0]->s.row, 1);
	EXPECT_EQ(tokens[0]->s.col, 1);
	EXPECT_EQ(tokens[1]->s.row, 3);
	EXPECT_EQ(tokens[1]->s.col, 1);
	EXPECT_TRUE(dynamic_cast<EndOfFile *>(tokens[2].get()));
	program = "x\ny\nz";
	l = Lexer(program, "", &e);
	tokens = l.lex();
	EXPECT_EQ(tokens.size(), 4);
	EXPECT_EQ(tokens[0]->s.row, 1);
	EXPECT_EQ(tokens[0]->s.col, 1);
	EXPECT_EQ(tokens[1]->s.row, 2);
	EXPECT_EQ(tokens[1]->s.col, 1);
	EXPECT_EQ(tokens[2]->s.row, 3);
	EXPECT_EQ(tokens[2]->s.col, 1);
	EXPECT_TRUE(dynamic_cast<EndOfFile *>(tokens[3].get()));

	program = "\nx\ny";
	l = Lexer(program, "", &e);
	tokens = l.lex();
	EXPECT_EQ(tokens.size(), 3);
	EXPECT_EQ(tokens[0]->s.row, 2);
	EXPECT_EQ(tokens[0]->s.col, 1);
	EXPECT_EQ(tokens[1]->s.row, 3);
	EXPECT_EQ(tokens[1]->s.col, 1);
	EXPECT_TRUE(dynamic_cast<EndOfFile *>(tokens[2].get()));
}

TEST_F(TestLexer, testCarriageReturns) {
	std::string program = "x";
	l = Lexer(program, "", &e);
	tokens = l.lex();
	EXPECT_EQ(tokens.size(), 2);
	EXPECT_EQ(tokens[0]->s.row, 1);
	EXPECT_EQ(tokens[0]->s.col, 1);
	EXPECT_TRUE(dynamic_cast<EndOfFile *>(tokens[1].get()));
	program = "x\r";
	l = Lexer(program, "", &e);
	tokens = l.lex();
	EXPECT_EQ(tokens.size(), 2);
	EXPECT_EQ(tokens[0]->s.row, 1);
	EXPECT_EQ(tokens[0]->s.col, 1);
	EXPECT_TRUE(dynamic_cast<EndOfFile *>(tokens[1].get()));
	program = "x\r\r";
	l = Lexer(program, "", &e);
	tokens = l.lex();
	EXPECT_EQ(tokens.size(), 2);
	EXPECT_EQ(tokens[0]->s.row, 1);
	EXPECT_EQ(tokens[0]->s.col, 1);
	EXPECT_TRUE(dynamic_cast<EndOfFile *>(tokens[1].get()));

	program = "\rx";
	l = Lexer(program, "", &e);
	tokens = l.lex();
	EXPECT_EQ(tokens.size(), 2);
	EXPECT_EQ(tokens[0]->s.row, 2);
	EXPECT_EQ(tokens[0]->s.col, 1);
	EXPECT_TRUE(dynamic_cast<EndOfFile *>(tokens[1].get()));
	program = "\r\rx";
	l = Lexer(program, "", &e);
	tokens = l.lex();
	EXPECT_EQ(tokens.size(), 2);
	EXPECT_EQ(tokens[0]->s.row, 3);
	EXPECT_EQ(tokens[0]->s.col, 1);
	EXPECT_TRUE(dynamic_cast<EndOfFile *>(tokens[1].get()));

	program = "x\ry";
	l = Lexer(program, "", &e);
	tokens = l.lex();
	EXPECT_EQ(tokens.size(), 3);
	EXPECT_EQ(tokens[0]->s.row, 1);
	EXPECT_EQ(tokens[0]->s.col, 1);
	EXPECT_EQ(tokens[1]->s.row, 2);
	EXPECT_EQ(tokens[1]->s.col, 1);
	EXPECT_TRUE(dynamic_cast<EndOfFile *>(tokens[2].get()));
	program = "x\r\ry";
	l = Lexer(program, "", &e);
	tokens = l.lex();
	EXPECT_EQ(tokens.size(), 3);
	EXPECT_EQ(tokens[0]->s.row, 1);
	EXPECT_EQ(tokens[0]->s.col, 1);
	EXPECT_EQ(tokens[1]->s.row, 3);
	EXPECT_EQ(tokens[1]->s.col, 1);
	EXPECT_TRUE(dynamic_cast<EndOfFile *>(tokens[2].get()));
	program = "x\ry\rz";
	l = Lexer(program, "", &e);
	tokens = l.lex();
	EXPECT_EQ(tokens.size(), 4);
	EXPECT_EQ(tokens[0]->s.row, 1);
	EXPECT_EQ(tokens[0]->s.col, 1);
	EXPECT_EQ(tokens[1]->s.row, 2);
	EXPECT_EQ(tokens[1]->s.col, 1);
	EXPECT_EQ(tokens[2]->s.row, 3);
	EXPECT_EQ(tokens[2]->s.col, 1);
	EXPECT_TRUE(dynamic_cast<EndOfFile *>(tokens[3].get()));

	program = "\rx\ry";
	l = Lexer(program, "", &e);
	tokens = l.lex();
	EXPECT_EQ(tokens.size(), 3);
	EXPECT_EQ(tokens[0]->s.row, 2);
	EXPECT_EQ(tokens[0]->s.col, 1);
	EXPECT_EQ(tokens[1]->s.row, 3);
	EXPECT_EQ(tokens[1]->s.col, 1);
	EXPECT_TRUE(dynamic_cast<EndOfFile *>(tokens[2].get()));
}

TEST_F(TestLexer, testWindowsLineEndings) {
	std::string program = "x";
	l = Lexer(program, "", &e);
	tokens = l.lex();
	EXPECT_EQ(tokens.size(), 2);
	EXPECT_EQ(tokens[0]->s.row, 1);
	EXPECT_EQ(tokens[0]->s.col, 1);
	EXPECT_TRUE(dynamic_cast<EndOfFile *>(tokens[1].get()));
	program = "x\r\n";
	l = Lexer(program, "", &e);
	tokens = l.lex();
	EXPECT_EQ(tokens.size(), 2);
	EXPECT_EQ(tokens[0]->s.row, 1);
	EXPECT_EQ(tokens[0]->s.col, 1);
	EXPECT_TRUE(dynamic_cast<EndOfFile *>(tokens[1].get()));
	program = "x\r\n\r\n";
	l = Lexer(program, "", &e);
	tokens = l.lex();
	EXPECT_EQ(tokens.size(), 2);
	EXPECT_EQ(tokens[0]->s.row, 1);
	EXPECT_EQ(tokens[0]->s.col, 1);
	EXPECT_TRUE(dynamic_cast<EndOfFile *>(tokens[1].get()));

	program = "\r\nx";
	l = Lexer(program, "", &e);
	tokens = l.lex();
	EXPECT_EQ(tokens.size(), 2);
	EXPECT_EQ(tokens[0]->s.row, 2);
	EXPECT_EQ(tokens[0]->s.col, 1);
	EXPECT_TRUE(dynamic_cast<EndOfFile *>(tokens[1].get()));
	program = "\r\n\r\nx";
	l = Lexer(program, "", &e);
	tokens = l.lex();
	EXPECT_EQ(tokens.size(), 2);
	EXPECT_EQ(tokens[0]->s.row, 3);
	EXPECT_EQ(tokens[0]->s.col, 1);
	EXPECT_TRUE(dynamic_cast<EndOfFile *>(tokens[1].get()));

	program = "x\r\ny";
	l = Lexer(program, "", &e);
	tokens = l.lex();
	EXPECT_EQ(tokens.size(), 3);
	EXPECT_EQ(tokens[0]->s.row, 1);
	EXPECT_EQ(tokens[0]->s.col, 1);
	EXPECT_EQ(tokens[1]->s.row, 2);
	EXPECT_EQ(tokens[1]->s.col, 1);
	EXPECT_TRUE(dynamic_cast<EndOfFile *>(tokens[2].get()));
	program = "x\r\n\r\ny";
	l = Lexer(program, "", &e);
	tokens = l.lex();
	EXPECT_EQ(tokens.size(), 3);
	EXPECT_EQ(tokens[0]->s.row, 1);
	EXPECT_EQ(tokens[0]->s.col, 1);
	EXPECT_EQ(tokens[1]->s.row, 3);
	EXPECT_EQ(tokens[1]->s.col, 1);
	EXPECT_TRUE(dynamic_cast<EndOfFile *>(tokens[2].get()));
	program = "x\r\ny\r\nz";
	l = Lexer(program, "", &e);
	tokens = l.lex();
	EXPECT_EQ(tokens.size(), 4);
	EXPECT_EQ(tokens[0]->s.row, 1);
	EXPECT_EQ(tokens[0]->s.col, 1);
	EXPECT_EQ(tokens[1]->s.row, 2);
	EXPECT_EQ(tokens[1]->s.col, 1);
	EXPECT_EQ(tokens[2]->s.row, 3);
	EXPECT_EQ(tokens[2]->s.col, 1);
	EXPECT_TRUE(dynamic_cast<EndOfFile *>(tokens[3].get()));

	program = "\r\nx\r\ny";
	l = Lexer(program, "", &e);
	tokens = l.lex();
	EXPECT_EQ(tokens.size(), 3);
	EXPECT_EQ(tokens[0]->s.row, 2);
	EXPECT_EQ(tokens[0]->s.col, 1);
	EXPECT_EQ(tokens[1]->s.row, 3);
	EXPECT_EQ(tokens[1]->s.col, 1);
	EXPECT_TRUE(dynamic_cast<EndOfFile *>(tokens[2].get()));
}

TEST_F(TestLexer, testColumnNumbering) {
	std::string program = "x";
	l = Lexer(program, "", &e);
	tokens = l.lex();
	EXPECT_EQ(tokens.size(), 2);
	EXPECT_EQ(tokens[0]->s.row, 1);
	EXPECT_EQ(tokens[0]->s.col, 1);
	EXPECT_TRUE(dynamic_cast<EndOfFile *>(tokens[1].get()));
	program = " x";
	l = Lexer(program, "", &e);
	tokens = l.lex();
	EXPECT_EQ(tokens.size(), 2);
	EXPECT_EQ(tokens[0]->s.row, 1);
	EXPECT_EQ(tokens[0]->s.col, 2);
	EXPECT_TRUE(dynamic_cast<EndOfFile *>(tokens[1].get()));
	program = "  x";
	l = Lexer(program, "", &e);
	tokens = l.lex();
	EXPECT_EQ(tokens.size(), 2);
	EXPECT_EQ(tokens[0]->s.row, 1);
	EXPECT_EQ(tokens[0]->s.col, 3);
	EXPECT_TRUE(dynamic_cast<EndOfFile *>(tokens[1].get()));
	program = "   x";
	l = Lexer(program, "", &e);
	tokens = l.lex();
	EXPECT_EQ(tokens.size(), 2);
	EXPECT_EQ(tokens[0]->s.row, 1);
	EXPECT_EQ(tokens[0]->s.col, 4);
	EXPECT_TRUE(dynamic_cast<EndOfFile *>(tokens[1].get()));

	program = "\nx";
	l = Lexer(program, "", &e);
	tokens = l.lex();
	EXPECT_EQ(tokens.size(), 2);
	EXPECT_EQ(tokens[0]->s.row, 2);
	EXPECT_EQ(tokens[0]->s.col, 1);
	EXPECT_TRUE(dynamic_cast<EndOfFile *>(tokens[1].get()));
	program = "\n x";
	l = Lexer(program, "", &e);
	tokens = l.lex();
	EXPECT_EQ(tokens.size(), 2);
	EXPECT_EQ(tokens[0]->s.row, 2);
	EXPECT_EQ(tokens[0]->s.col, 2);
	EXPECT_TRUE(dynamic_cast<EndOfFile *>(tokens[1].get()));
	program = "\n  x";
	l = Lexer(program, "", &e);
	tokens = l.lex();
	EXPECT_EQ(tokens.size(), 2);
	EXPECT_EQ(tokens[0]->s.row, 2);
	EXPECT_EQ(tokens[0]->s.col, 3);
	EXPECT_TRUE(dynamic_cast<EndOfFile *>(tokens[1].get()));
	program = "\n   x";
	l = Lexer(program, "", &e);
	tokens = l.lex();
	EXPECT_EQ(tokens.size(), 2);
	EXPECT_EQ(tokens[0]->s.row, 2);
	EXPECT_EQ(tokens[0]->s.col, 4);
	EXPECT_TRUE(dynamic_cast<EndOfFile *>(tokens[1].get()));

	program = "x\ny";
	l = Lexer(program, "", &e);
	tokens = l.lex();
	EXPECT_EQ(tokens.size(), 3);
	EXPECT_EQ(tokens[0]->s.row, 1);
	EXPECT_EQ(tokens[0]->s.col, 1);
	EXPECT_EQ(tokens[1]->s.row, 2);
	EXPECT_EQ(tokens[1]->s.col, 1);
	EXPECT_TRUE(dynamic_cast<EndOfFile *>(tokens[2].get()));
	program = " x\ny";
	l = Lexer(program, "", &e);
	tokens = l.lex();
	EXPECT_EQ(tokens.size(), 3);
	EXPECT_EQ(tokens[0]->s.row, 1);
	EXPECT_EQ(tokens[0]->s.col, 2);
	EXPECT_EQ(tokens[1]->s.row, 2);
	EXPECT_EQ(tokens[1]->s.col, 1);
	EXPECT_TRUE(dynamic_cast<EndOfFile *>(tokens[2].get()));
	program = "x\n y";
	l = Lexer(program, "", &e);
	tokens = l.lex();
	EXPECT_EQ(tokens.size(), 3);
	EXPECT_EQ(tokens[0]->s.row, 1);
	EXPECT_EQ(tokens[0]->s.col, 1);
	EXPECT_EQ(tokens[1]->s.row, 2);
	EXPECT_EQ(tokens[1]->s.col, 2);
	EXPECT_TRUE(dynamic_cast<EndOfFile *>(tokens[2].get()));
	program = " x\n y";
	l = Lexer(program, "", &e);
	tokens = l.lex();
	EXPECT_EQ(tokens.size(), 3);
	EXPECT_EQ(tokens[0]->s.row, 1);
	EXPECT_EQ(tokens[0]->s.col, 2);
	EXPECT_EQ(tokens[1]->s.row, 2);
	EXPECT_EQ(tokens[1]->s.col, 2);
	EXPECT_TRUE(dynamic_cast<EndOfFile *>(tokens[2].get()));
	program = "x \ny";
	l = Lexer(program, "", &e);
	tokens = l.lex();
	EXPECT_EQ(tokens.size(), 3);
	EXPECT_EQ(tokens[0]->s.row, 1);
	EXPECT_EQ(tokens[0]->s.col, 1);
	EXPECT_EQ(tokens[1]->s.row, 2);
	EXPECT_EQ(tokens[1]->s.col, 1);
	EXPECT_TRUE(dynamic_cast<EndOfFile *>(tokens[2].get()));
	program = "x\n y\nz";
	l = Lexer(program, "", &e);
	tokens = l.lex();
	EXPECT_EQ(tokens.size(), 4);
	EXPECT_EQ(tokens[0]->s.row, 1);
	EXPECT_EQ(tokens[0]->s.col, 1);
	EXPECT_EQ(tokens[1]->s.row, 2);
	EXPECT_EQ(tokens[1]->s.col, 2);
	EXPECT_EQ(tokens[2]->s.row, 3);
	EXPECT_EQ(tokens[2]->s.col, 1);
	EXPECT_TRUE(dynamic_cast<EndOfFile *>(tokens[3].get()));

	program = "123 567 90";
	l = Lexer(program, "", &e);
	tokens = l.lex();
	EXPECT_EQ(tokens.size(), 4);
	EXPECT_EQ(tokens[0]->s.row, 1);
	EXPECT_EQ(tokens[0]->s.col, 1);
	EXPECT_EQ(tokens[1]->s.row, 1);
	EXPECT_EQ(tokens[1]->s.col, 5);
	EXPECT_EQ(tokens[2]->s.row, 1);
	EXPECT_EQ(tokens[2]->s.col, 9);
	EXPECT_TRUE(dynamic_cast<EndOfFile *>(tokens[3].get()));
	program = "12  56  90";
	l = Lexer(program, "", &e);
	tokens = l.lex();
	EXPECT_EQ(tokens.size(), 4);
	EXPECT_EQ(tokens[0]->s.row, 1);
	EXPECT_EQ(tokens[0]->s.col, 1);
	EXPECT_EQ(tokens[1]->s.row, 1);
	EXPECT_EQ(tokens[1]->s.col, 5);
	EXPECT_EQ(tokens[2]->s.row, 1);
	EXPECT_EQ(tokens[2]->s.col, 9);
	EXPECT_TRUE(dynamic_cast<EndOfFile *>(tokens[3].get()));
	program = "1    6  9";
	l = Lexer(program, "", &e);
	tokens = l.lex();
	EXPECT_EQ(tokens.size(), 4);
	EXPECT_EQ(tokens[0]->s.row, 1);
	EXPECT_EQ(tokens[0]->s.col, 1);
	EXPECT_EQ(tokens[1]->s.row, 1);
	EXPECT_EQ(tokens[1]->s.col, 6);
	EXPECT_EQ(tokens[2]->s.row, 1);
	EXPECT_EQ(tokens[2]->s.col, 9);
	EXPECT_TRUE(dynamic_cast<EndOfFile *>(tokens[3].get()));

	program = "1;2,3-4*5!6&7||8::9";
	l = Lexer(program, "", &e);
	tokens = l.lex();
	EXPECT_EQ(tokens.size(), 18);
	// 1
	EXPECT_EQ(tokens[0]->s.row, 1);
	EXPECT_EQ(tokens[0]->s.col, 1);
	// ;
	EXPECT_EQ(tokens[1]->s.row, 1);
	EXPECT_EQ(tokens[1]->s.col, 2);
	// 2
	EXPECT_EQ(tokens[2]->s.row, 1);
	EXPECT_EQ(tokens[2]->s.col, 3);
	// ,
	EXPECT_EQ(tokens[3]->s.row, 1);
	EXPECT_EQ(tokens[3]->s.col, 4);
	// 3
	EXPECT_EQ(tokens[4]->s.row, 1);
	EXPECT_EQ(tokens[4]->s.col, 5);
	// -
	EXPECT_EQ(tokens[5]->s.row, 1);
	EXPECT_EQ(tokens[5]->s.col, 6);
	// 4
	EXPECT_EQ(tokens[6]->s.row, 1);
	EXPECT_EQ(tokens[6]->s.col, 7);
	// *
	EXPECT_EQ(tokens[7]->s.row, 1);
	EXPECT_EQ(tokens[7]->s.col, 8);
	// 5
	EXPECT_EQ(tokens[8]->s.row, 1);
	EXPECT_EQ(tokens[8]->s.col, 9);
	// !
	EXPECT_EQ(tokens[9]->s.row, 1);
	EXPECT_EQ(tokens[9]->s.col, 10);
	// 6
	EXPECT_EQ(tokens[10]->s.row, 1);
	EXPECT_EQ(tokens[10]->s.col, 11);
	// &
	EXPECT_EQ(tokens[11]->s.row, 1);
	EXPECT_EQ(tokens[11]->s.col, 12);
	// 7
	EXPECT_EQ(tokens[12]->s.row, 1);
	EXPECT_EQ(tokens[12]->s.col, 13);
	// ||
	EXPECT_EQ(tokens[13]->s.row, 1);
	EXPECT_EQ(tokens[13]->s.col, 14);
	// 8
	EXPECT_EQ(tokens[14]->s.row, 1);
	EXPECT_EQ(tokens[14]->s.col, 16);
	// ::
	EXPECT_EQ(tokens[15]->s.row, 1);
	EXPECT_EQ(tokens[15]->s.col, 17);
	// 9
	EXPECT_EQ(tokens[16]->s.row, 1);
	EXPECT_EQ(tokens[16]->s.col, 19);
}

TEST_F(TestLexer, testTabSizeConfiguration) {
	// Default tab width (4)
	const std::string program = "\tx";
	l = Lexer(program, "", &e);
	tokens = l.lex();
	EXPECT_EQ(tokens.size(), 2);
	EXPECT_EQ(tokens[0]->s.col, 5);
	EXPECT_TRUE(dynamic_cast<EndOfFile *>(tokens[1].get()));

	// Customizable tab width
	for (uint32_t tabSize = 1; tabSize <= 10; tabSize++) {
		std::string program = "\tx";
		for (uint32_t i = 0; i < tabSize; i++) {
			l = Lexer(program, "", &e, tabSize);
			tokens = l.lex();
			EXPECT_EQ(tokens.size(), 2);
			EXPECT_EQ(tokens[0]->s.col, tabSize + 1);
			EXPECT_TRUE(dynamic_cast<EndOfFile *>(tokens[1].get()));
			program = " " + program;
		}
		l = Lexer(program, "", &e, tabSize);
		tokens = l.lex();
		EXPECT_EQ(tokens.size(), 2);
		EXPECT_EQ(tokens[0]->s.col, 2 * tabSize + 1);
		EXPECT_TRUE(dynamic_cast<EndOfFile *>(tokens[1].get()));
	}
}

TEST_F(TestLexer, testTrueLiteral) {
	std::string program = "true";
	l = Lexer(program, "", &e);
	tokens = l.lex();
	EXPECT_EQ(tokens.size(), 2);
	EXPECT_EQ(*dynamic_cast<BoolLiteral *>(tokens[0].get()), BoolLiteral(dummy, true));
	EXPECT_TRUE(dynamic_cast<EndOfFile *>(tokens[1].get()));
}

TEST_F(TestLexer, testFalseLiteral) {
	std::string program = "false";
	l = Lexer(program, "", &e);
	tokens = l.lex();
	EXPECT_EQ(tokens.size(), 2);
	EXPECT_EQ(*dynamic_cast<BoolLiteral *>(tokens[0].get()), BoolLiteral(dummy, false));
	EXPECT_TRUE(dynamic_cast<EndOfFile *>(tokens[1].get()));
}

TEST_F(TestLexer, testCharacterLiteral) {
	std::string program = "'a'";
	l = Lexer(program, "", &e);
	tokens = l.lex();
	EXPECT_EQ(tokens.size(), 2);
	EXPECT_EQ(*dynamic_cast<CharacterLiteral *>(tokens[0].get()),
	      CharacterLiteral(dummy, 'a'));
	EXPECT_TRUE(dynamic_cast<EndOfFile *>(tokens[1].get()));
}

TEST_F(TestLexer, testCharacterLiteralEscapeSequences) {
	for (const auto &escape : escapeSequences) {
		std::string program = std::string("'") + escape.first + "'";
		l = Lexer(program, "", &e);
		tokens = l.lex();
		EXPECT_EQ(tokens.size(), 2);
		EXPECT_EQ(*dynamic_cast<CharacterLiteral *>(tokens[0].get()),
		      CharacterLiteral(dummy, escape.second));
		EXPECT_TRUE(dynamic_cast<EndOfFile *>(tokens[1].get()));
	}
}
