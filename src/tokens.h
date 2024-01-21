#ifndef TOKENS_H
#define TOKENS_H

#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
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

std::ostream &operator<<(std::ostream &os, const Type &type);

struct Slice {
	std::string_view contents;
	const char *source;
	size_t row;
	size_t col;
	Slice(std::string_view contents, const char *source, size_t row, size_t col);
};

std::ostream &operator<<(std::ostream &os, const Slice &slice);

struct Token {
	const char *source;
	size_t row;
	size_t col;

	virtual void print(std::ostream &os) const = 0;
	mockable void error(const char *const error, ...) const __attribute__((noreturn));
protected:
	Token(const char *const source, size_t row, size_t col);
};

struct Keyword : public Token {
	enum class Type {
		VOID,
		RETURN
	};
	Type type;
	Keyword(Slice s, Type type);
	virtual void print(std::ostream &os) const;
};

struct Primitive : public Token {
	Type type;
	Primitive(Slice s, Type type);
	virtual void print(std::ostream &os) const;
	void compile(std::ostream &outfie);
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
	Punctuation(Slice s, Type type);
	virtual void print(std::ostream &os) const;
};

struct Identifier : public Token {
	std::string_view s;
	explicit Identifier(Slice s);
	virtual void print(std::ostream &os) const;
	void compile(std::ostream &outfile);
};

struct Hasher {
	std::size_t operator()(Identifier *const id) const;
};

struct Comparator {
	bool operator()(Identifier *a, Identifier *b) const;
};

#endif
