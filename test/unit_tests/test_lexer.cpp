#ifndef DEBUG_TEST_MODE
#	error "DEBUG_TEST_MODE not defined"
#endif

#include "lexer.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include <cstring>
#include <stdio.h>
#include <string>
#include <vector>

TEST(test_lexer, test_constructor) {
	// If tabSize is 0, the constructor should throw invalid_argument
	EXPECT_THROW(Lexer("", 0, "", 0), std::invalid_argument);
}

TEST(test_lexer, test_isSep) {
	Lexer l = Lexer("", 0, "");

	// Test 1: Standalone separator characters
	EXPECT_TRUE(l.isSep(" "));
	EXPECT_TRUE(l.isSep("\n"));
	EXPECT_TRUE(l.isSep("\t"));
	EXPECT_TRUE(l.isSep("\v"));
	EXPECT_TRUE(l.isSep("\f"));
	EXPECT_TRUE(l.isSep("\r"));
	EXPECT_TRUE(l.isSep("("));
	EXPECT_TRUE(l.isSep(")"));
	EXPECT_TRUE(l.isSep(";"));
	EXPECT_TRUE(l.isSep("{"));
	EXPECT_TRUE(l.isSep("}"));
	EXPECT_TRUE(l.isSep(","));
	EXPECT_TRUE(l.isSep("+"));
	EXPECT_TRUE(l.isSep("-"));
	EXPECT_TRUE(l.isSep("*"));
	EXPECT_TRUE(l.isSep("/"));
	EXPECT_TRUE(l.isSep("%"));
	EXPECT_TRUE(l.isSep("="));

	// Test 2: Consecutive alphanumeric not sep
	EXPECT_FALSE(l.isSep(&"Aa"[1]));
	EXPECT_FALSE(l.isSep(&"aa"[1]));
	EXPECT_FALSE(l.isSep(&"AA"[1]));
	EXPECT_FALSE(l.isSep(&"aA"[1]));
	EXPECT_FALSE(l.isSep(&"0a"[1]));
	EXPECT_FALSE(l.isSep(&"0A"[1]));
	EXPECT_FALSE(l.isSep(&"A0"[1]));
	EXPECT_FALSE(l.isSep(&"a0"[1]));

	// Test 3: Alphanumeric after non-alphanumeric is sep
	EXPECT_TRUE(l.isSep(&";a"[1]));
	EXPECT_TRUE(l.isSep(&";A"[1]));
	EXPECT_TRUE(l.isSep(&";0"[1]));
}

TEST(test_lexer, test_slice) {
	const char *program;
	Lexer l = Lexer("", 0, "");

	// Test 1: Whitespace skipped
	program = " x";
	l = Lexer(program, strlen(program), "");
	l.slice();
	EXPECT_EQ(l.slices.size(), 1);
	if (l.slices.size() >= 1) {
		EXPECT_EQ(l.slices[0], "x");
	}

	program = "x ";
	l = Lexer(program, strlen(program), "");
	l.slice();
	EXPECT_EQ(l.slices.size(), 1);
	if (l.slices.size() >= 1) {
		EXPECT_EQ(l.slices[0], "x");
	}

	for (char whitespace : {' ', '\n', '\t', '\v', '\f', '\r'}) {
		char program[] = {whitespace, 'x', whitespace};
		l = Lexer(program, 3, "");
		l.slice();
		EXPECT_EQ(l.slices.size(), 1);
		if (l.slices.size() >= 1) {
			EXPECT_EQ(l.slices[0], "x");
		}
		char program2[] = {whitespace, whitespace, 'x', whitespace, whitespace};
		l = Lexer(program2, 5, "");
		l.slice();
		EXPECT_EQ(l.slices.size(), 1);
		if (l.slices.size() >= 1) {
			EXPECT_EQ(l.slices[0], "x");
		}
	}

	// Test 2: Correctly slice based on isSep
	class MockLexer : public Lexer {
	public:
		MockLexer(const char *const program) : Lexer(program, strlen(program), "") {
		}

		MOCK_METHOD(bool, isSep, (const char *const c));
	} mocked("123456789");

	// 123 45 6 7 89
	// XFF TF T T TF
	EXPECT_CALL(mocked, isSep)
	      .WillOnce(::testing::Return(false))
	      .WillOnce(::testing::Return(false))
	      .WillOnce(::testing::Return(true))
	      .WillOnce(::testing::Return(false))
	      .WillOnce(::testing::Return(true))
	      .WillOnce(::testing::Return(true))
	      .WillOnce(::testing::Return(true))
	      .WillOnce(::testing::Return(false));
	mocked.slice();
	EXPECT_EQ(mocked.slices.size(), 5);
	if (mocked.slices.size() >= 5) {
		EXPECT_EQ(mocked.slices[0], "123");
		EXPECT_EQ(mocked.slices[1], "45");
		EXPECT_EQ(mocked.slices[2], "6");
		EXPECT_EQ(mocked.slices[3], "7");
		EXPECT_EQ(mocked.slices[4], "89");
	}

	// Test 3: Make sure line numbers are correct
	program = "x";
	l = Lexer(program, strlen(program), "");
	l.slice();
	EXPECT_EQ(l.slices.size(), 1);
	if (l.slices.size() >= 1) {
		EXPECT_EQ(l.slices[0].row, 1);
	}

	program = "\nx";
	l = Lexer(program, strlen(program), "");
	l.slice();
	EXPECT_EQ(l.slices.size(), 1);
	if (l.slices.size() >= 1) {
		EXPECT_EQ(l.slices[0].row, 2);
	}

	program = "x\n";
	l = Lexer(program, strlen(program), "");
	l.slice();
	EXPECT_EQ(l.slices.size(), 1);
	if (l.slices.size() >= 1) {
		EXPECT_EQ(l.slices[0].row, 1);
	}

	program = "\n\nx";
	l = Lexer(program, strlen(program), "");
	l.slice();
	EXPECT_EQ(l.slices.size(), 1);
	if (l.slices.size() >= 1) {
		EXPECT_EQ(l.slices[0].row, 3);
	}

	program = "x\n\nx";
	l = Lexer(program, strlen(program), "");
	l.slice();
	EXPECT_EQ(l.slices.size(), 2);
	if (l.slices.size() >= 2) {
		EXPECT_EQ(l.slices[0].row, 1);
		EXPECT_EQ(l.slices[1].row, 3);
	}

	program = "\rx";
	l = Lexer(program, strlen(program), "");
	l.slice();
	EXPECT_EQ(l.slices.size(), 1);
	if (l.slices.size() >= 1) {
		EXPECT_EQ(l.slices[0].row, 2);
	}

	program = "x\r";
	l = Lexer(program, strlen(program), "");
	l.slice();
	EXPECT_EQ(l.slices.size(), 1);
	if (l.slices.size() >= 1) {
		EXPECT_EQ(l.slices[0].row, 1);
	}

	program = "\r\rx";
	l = Lexer(program, strlen(program), "");
	l.slice();
	EXPECT_EQ(l.slices.size(), 1);
	if (l.slices.size() >= 1) {
		EXPECT_EQ(l.slices[0].row, 3);
	}

	program = "x\r\rx";
	l = Lexer(program, strlen(program), "");
	l.slice();
	EXPECT_EQ(l.slices.size(), 2);
	if (l.slices.size() >= 2) {
		EXPECT_EQ(l.slices[0].row, 1);
		EXPECT_EQ(l.slices[1].row, 3);
	}

	// Special case \r\n only counts as one newline
	program = "\r\nx";
	l = Lexer(program, strlen(program), "");
	l.slice();
	EXPECT_EQ(l.slices.size(), 1);
	if (l.slices.size() >= 1) {
		EXPECT_EQ(l.slices[0].row, 2);
	}

	// Test 4: Make sure column numbers are correct
	program = "x";
	l = Lexer(program, strlen(program), "");
	l.slice();
	EXPECT_EQ(l.slices.size(), 1);
	if (l.slices.size() >= 1) {
		EXPECT_EQ(l.slices[0].col, 1);
	}

	program = "xyz";
	l = Lexer(program, strlen(program), "");
	l.slice();
	EXPECT_EQ(l.slices.size(), 1);
	if (l.slices.size() >= 1) {
		EXPECT_EQ(l.slices[0].col, 1);
	}

	program = " xyz";
	l = Lexer(program, strlen(program), "");
	l.slice();
	EXPECT_EQ(l.slices.size(), 1);
	if (l.slices.size() >= 1) {
		EXPECT_EQ(l.slices[0].col, 2);
	}

	program = "a;3";
	l = Lexer(program, strlen(program), "");
	l.slice();
	EXPECT_EQ(l.slices.size(), 3);
	if (l.slices.size() >= 3) {
		EXPECT_EQ(l.slices[0].col, 1);
		EXPECT_EQ(l.slices[1].col, 2);
		EXPECT_EQ(l.slices[2].col, 3);
	}

	program = "abc 123";
	l = Lexer(program, strlen(program), "");
	l.slice();
	EXPECT_EQ(l.slices.size(), 2);
	if (l.slices.size() >= 2) {
		EXPECT_EQ(l.slices[0].col, 1);
		EXPECT_EQ(l.slices[1].col, 5);
	}

	// Testing that tab size is configurable
	for (uint32_t tabSize = 1; tabSize <= 10; tabSize++) {
		std::string program2 = "\tx";
		for (uint32_t i = 0; i < tabSize; i++) {
			l = Lexer(program2.c_str(), strlen(program2.c_str()), "", tabSize);
			l.slice();
			EXPECT_EQ(l.slices.size(), 1);
			if (l.slices.size() >= 1) {
				EXPECT_EQ(l.slices[0].col, tabSize + 1);
			}
			program2 = " " + program2;
		}
		l = Lexer(program2.c_str(), strlen(program2.c_str()), "", tabSize);
		l.slice();
		EXPECT_EQ(l.slices.size(), 1);
		if (l.slices.size() >= 1) {
			EXPECT_EQ(l.slices[0].col, 2 * tabSize + 1);
		}
	}
}

const char *const keywords[] = {"void", "return"};

TEST(test_lexer, test_createKeyword) {
	Lexer l = Lexer("", 0, "");
	Keyword *keyword;
	keyword = l.createKeyword(Slice("void", 4, "", 0, 0));
	ASSERT_NE(keyword, nullptr);
	EXPECT_EQ(keyword->type, Keyword::Type::VOID);
	keyword = l.createKeyword(Slice("return", 6, "", 0, 0));
	ASSERT_NE(keyword, nullptr);
	EXPECT_EQ(keyword->type, Keyword::Type::RETURN);
}

const char *const primitives[]
      = {"int", "byte", "short", "long", "float", "double", "bool", "char"};

TEST(test_lexer, test_createPrimitive) {
	Lexer l = Lexer("", 0, "");
	Primitive *primitive;
	primitive = l.createPrimitive(Slice("int", 3, "", 0, 0));
	ASSERT_NE(primitive, nullptr);
	EXPECT_EQ(primitive->type, Type::INT);
	primitive = l.createPrimitive(Slice("byte", 4, "", 0, 0));
	ASSERT_NE(primitive, nullptr);
	EXPECT_EQ(primitive->type, Type::BYTE);
	primitive = l.createPrimitive(Slice("short", 5, "", 0, 0));
	ASSERT_NE(primitive, nullptr);
	EXPECT_EQ(primitive->type, Type::SHORT);
	primitive = l.createPrimitive(Slice("long", 4, "", 0, 0));
	ASSERT_NE(primitive, nullptr);
	EXPECT_EQ(primitive->type, Type::LONG);
	primitive = l.createPrimitive(Slice("float", 5, "", 0, 0));
	ASSERT_NE(primitive, nullptr);
	EXPECT_EQ(primitive->type, Type::FLOAT);
	primitive = l.createPrimitive(Slice("double", 6, "", 0, 0));
	ASSERT_NE(primitive, nullptr);
	EXPECT_EQ(primitive->type, Type::DOUBLE);
	primitive = l.createPrimitive(Slice("bool", 4, "", 0, 0));
	ASSERT_NE(primitive, nullptr);
	EXPECT_EQ(primitive->type, Type::BOOL);
	primitive = l.createPrimitive(Slice("char", 4, "", 0, 0));
	ASSERT_NE(primitive, nullptr);
	EXPECT_EQ(primitive->type, Type::CHAR);

	// Type enum values that shouldn't be created by createPrimitive
	primitive = l.createPrimitive(Slice("void", 4, "", 0, 0));
	ASSERT_EQ(primitive, nullptr);
	primitive = l.createPrimitive(Slice("unknown", 7, "", 0, 0));
	ASSERT_EQ(primitive, nullptr);
}

const char *const punctuations[]
      = {"(", ")", ";", "{", "}", ",", "=", "+", "-", "*", "/", "%"};

TEST(test_lexer, test_createPunctuation) {
	Lexer l = Lexer("", 0, "");
	Punctuation *punctuation;
	punctuation = l.createPunctuation(Slice("(", 1, "", 0, 0));
	ASSERT_NE(punctuation, nullptr);
	EXPECT_EQ(punctuation->type, Punctuation::Type::OpenParen);
	punctuation = l.createPunctuation(Slice(")", 1, "", 0, 0));
	ASSERT_NE(punctuation, nullptr);
	EXPECT_EQ(punctuation->type, Punctuation::Type::CloseParen);
	punctuation = l.createPunctuation(Slice(";", 1, "", 0, 0));
	ASSERT_NE(punctuation, nullptr);
	EXPECT_EQ(punctuation->type, Punctuation::Type::Semicolon);
	punctuation = l.createPunctuation(Slice("{", 1, "", 0, 0));
	ASSERT_NE(punctuation, nullptr);
	EXPECT_EQ(punctuation->type, Punctuation::Type::OpenBrace);
	punctuation = l.createPunctuation(Slice("}", 1, "", 0, 0));
	ASSERT_NE(punctuation, nullptr);
	EXPECT_EQ(punctuation->type, Punctuation::Type::CloseBrace);
	punctuation = l.createPunctuation(Slice(",", 1, "", 0, 0));
	ASSERT_NE(punctuation, nullptr);
	EXPECT_EQ(punctuation->type, Punctuation::Type::Comma);
	punctuation = l.createPunctuation(Slice("=", 1, "", 0, 0));
	ASSERT_NE(punctuation, nullptr);
	EXPECT_EQ(punctuation->type, Punctuation::Type::Equals);
	punctuation = l.createPunctuation(Slice("+", 1, "", 0, 0));
	ASSERT_NE(punctuation, nullptr);
	EXPECT_EQ(punctuation->type, Punctuation::Type::Plus);
	punctuation = l.createPunctuation(Slice("-", 1, "", 0, 0));
	ASSERT_NE(punctuation, nullptr);
	EXPECT_EQ(punctuation->type, Punctuation::Type::Minus);
	punctuation = l.createPunctuation(Slice("*", 1, "", 0, 0));
	ASSERT_NE(punctuation, nullptr);
	EXPECT_EQ(punctuation->type, Punctuation::Type::Times);
	punctuation = l.createPunctuation(Slice("/", 1, "", 0, 0));
	ASSERT_NE(punctuation, nullptr);
	EXPECT_EQ(punctuation->type, Punctuation::Type::Divide);
	punctuation = l.createPunctuation(Slice("%", 1, "", 0, 0));
	ASSERT_NE(punctuation, nullptr);
	EXPECT_EQ(punctuation->type, Punctuation::Type::Mod);
}

TEST(test_lexer, test_createIdentifier) {
	Lexer l = Lexer("", 0, "");
	Identifier *id;
	id = l.createIdentifier(Slice("x", 1, "", 0, 0));
	ASSERT_NE(id, nullptr);
	EXPECT_EQ(id->s, "x");
	id = l.createIdentifier(Slice("abcd", 4, "", 0, 0));
	ASSERT_NE(id, nullptr);
	EXPECT_EQ(id->s, "abcd");
	id = l.createIdentifier(Slice("1234", 4, "", 0, 0));
	ASSERT_NE(id, nullptr);
	EXPECT_EQ(id->s, "1234");

	// It shouldn't care whether the "identifier" is another token type
	id = l.createIdentifier(Slice("int", 3, "", 0, 0));
	ASSERT_NE(id, nullptr);
	EXPECT_EQ(id->s, "int");
	id = l.createIdentifier(Slice("void", 4, "", 0, 0));
	ASSERT_NE(id, nullptr);
	EXPECT_EQ(id->s, "void");
	id = l.createIdentifier(Slice("return", 6, "", 0, 0));
	ASSERT_NE(id, nullptr);
	EXPECT_EQ(id->s, "return");
	id = l.createIdentifier(Slice("float", 5, "", 0, 0));
	ASSERT_NE(id, nullptr);
	EXPECT_EQ(id->s, "float");
}

TEST(test_lexer, test_tokenize) {
	class MockLexer : public Lexer {
	public:
		MockLexer() : Lexer("", 0, "") {
		}

		MOCK_METHOD(void, slice, ());
	};

	// Test 1: Keywords
	MockLexer l1;
	EXPECT_CALL(l1, slice).WillOnce(::testing::Invoke([&l1] {
		for (const char *const keyword : keywords) {
			l1.slices.emplace_back(keyword, strlen(keyword), "", 0, 0);
		}
	}));
	std::vector<Token *> *tokens = l1.tokenize();
	ASSERT_EQ(tokens->size(), sizeof(keywords) / sizeof(*keywords));
	for (size_t i = 0; i < sizeof(keywords) / sizeof(*keywords); i++) {
		EXPECT_TRUE(dynamic_cast<Keyword *>((*tokens)[i]));
	}

	// Test 2: Primitives
	MockLexer l2;
	EXPECT_CALL(l2, slice).WillOnce(::testing::Invoke([&l2] {
		for (const char *const primitive : primitives) {
			l2.slices.emplace_back(primitive, strlen(primitive), "", 0, 0);
		}
	}));
	tokens = l2.tokenize();
	ASSERT_EQ(tokens->size(), sizeof(primitives) / sizeof(*primitives));
	for (size_t i = 0; i < sizeof(primitives) / sizeof(*primitives); i++) {
		EXPECT_TRUE(dynamic_cast<Primitive *>((*tokens)[i]));
	}

	// Test 3: Punctuation
	MockLexer l3;
	EXPECT_CALL(l3, slice).WillOnce(::testing::Invoke([&l3] {
		for (const char *const punctuation : punctuations) {
			l3.slices.emplace_back(punctuation, strlen(punctuation), "", 0, 0);
		}
	}));
	tokens = l3.tokenize();
	ASSERT_EQ(tokens->size(), sizeof(punctuations) / sizeof(*punctuations));
	for (size_t i = 0; i < sizeof(punctuations) / sizeof(*punctuations); i++) {
		EXPECT_TRUE(dynamic_cast<Punctuation *>((*tokens)[i]));
	}
}
