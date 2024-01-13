#ifndef DEBUG_TEST_MODE
#	error "DEBUG_TEST_MODE not defined"
#endif

#include "tokens.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include <string>

TEST(test_tokens, test_Slice_eq) {
	// no EXPECT_NE since I don't currently have a need to implement operator!=

	Slice s = Slice("abcd", 4, "", 0, 0);

	// Test 1: std::string
	EXPECT_EQ(s, std::string("abcd"));
	EXPECT_FALSE(s == std::string("abc"));
	EXPECT_FALSE(s == std::string("abc") + "de");
	EXPECT_FALSE(s == std::string("ABCD"));
	EXPECT_FALSE(s == std::string(" abcd"));
	EXPECT_FALSE(s == std::string("abcd "));
	EXPECT_FALSE(s == std::string());

	// Test 2: Slice
	EXPECT_EQ(s, s);
	EXPECT_EQ(s, Slice("abcde", 4, "", 1, 1));
	EXPECT_FALSE(s == Slice("abc", 3, "", 0, 0));
	EXPECT_FALSE(s == Slice("abcde", 5, "", 0, 0));
	EXPECT_FALSE(s == Slice("ABCD", 4, "", 0, 0));
	EXPECT_FALSE(s == Slice(" abcd", 5, "", 0, 0));
	EXPECT_FALSE(s == Slice("abcd ", 5, "", 0, 0));
	EXPECT_FALSE(s == Slice("", static_cast<size_t>(0), "", 0, 0));

	// Test 3: char*
	// Heap allocating to ensure the slice points to different chars
	char *volatile heap_str = new char[5];
	heap_str[0] = 'a';
	heap_str[1] = 'b';
	heap_str[2] = 'c';
	heap_str[3] = 'd';
	heap_str[4] = '\0';
	EXPECT_EQ(s, heap_str);
	EXPECT_FALSE(s == "abc");
	EXPECT_FALSE(s == "abcde");
	EXPECT_FALSE(s == "ABCD");
	EXPECT_FALSE(s == " abcd");
	EXPECT_FALSE(s == "abcd ");
	EXPECT_FALSE(s == "");

	// Test 4: empty Slice
	// 4a: std::string
	s = Slice("", static_cast<size_t>(0), "", 0, 0);
	EXPECT_EQ(s, std::string(""));
	EXPECT_EQ(s, std::string());
	EXPECT_FALSE(s == std::string("abc"));
	// Test 4b: Slice
	EXPECT_EQ(s, s);
	EXPECT_EQ(s, Slice("", static_cast<size_t>(0), "", 1, 1));
	EXPECT_FALSE(s == Slice("abc", 3, "", 0, 0));
	// Test 4c: char*
	// Heap allocating to ensure the slice points to different char
	char *volatile heap_char = new char('\0');
	EXPECT_EQ(s, heap_char);
	EXPECT_FALSE(s == "abc");
}

// TODO Hasher and Comparator?

// TODO: compile methods? Sort of tested by the end to end tests so are they necessary?
