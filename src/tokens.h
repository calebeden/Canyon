#ifndef TOKENS_H
#define TOKENS_H

#include <string_view>

#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <typeinfo>
#include <vector>

#ifdef DEBUG_TEST_MODE
#	define mockable virtual
#else
#	define mockable
#endif

enum class Type {
	INT,
	BYTE,
	SHORT,
	LONG,
	FLOAT,
	DOUBLE,
	BOOL,
	CHAR,
	VOID,
	UNKNOWN,
};

std::ostream &operator<<(std::ostream &os, Type type);

struct Slice {
	std::string_view contents;
	std::filesystem::path source;
	size_t row;
	size_t col;
	Slice(std::string_view contents, std::filesystem::path source, size_t row,
	      size_t col);
};

std::ostream &operator<<(std::ostream &os, const Slice &slice);

struct Token {
	std::filesystem::path source;
	size_t row;
	size_t col;

	virtual void print(std::ostream &os) const = 0;
protected:
	Token(std::filesystem::path source, size_t row, size_t col);
};

struct Keyword : public Token {
	enum class Type {
		VOID,
		RETURN
	};
	Type type;
	Keyword(const Slice &s, Type type);
	virtual void print(std::ostream &os) const;
};

struct Primitive : public Token {
	Type type;
	Primitive(const Slice &s, Type type);
	virtual void print(std::ostream &os) const;
	void compile(std::ostream &outfile) const;
	static void compile(std::ostream &outfile, Type t);
};

struct Punctuation : public Token {
	enum class Type {
		OpenParen,
		CloseParen,
		Semicolon,
		OpenBrace,
		CloseBrace,
		Comma,
		Equals,
		Plus,
		Minus,
		Times,
		Divide,
		Mod
	};
	Type type;
	Punctuation(const Slice &s, Type type);
	virtual void print(std::ostream &os) const;
};

struct Identifier : public Token {
	std::string_view s;
	explicit Identifier(const Slice &s);
	virtual void print(std::ostream &os) const;
	void compile(std::ostream &outfile) const;
};

struct Hasher {
	std::size_t operator()(Identifier *const id) const;
};

struct Comparator {
	bool operator()(Identifier *a, Identifier *b) const;
};

#endif
