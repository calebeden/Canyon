#ifndef DEBUG_TEST_MODE
#  error "DEBUG_TEST_MODE not defined"
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
