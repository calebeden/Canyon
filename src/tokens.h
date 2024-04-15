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
	~Slice() = default;
};

std::ostream &operator<<(std::ostream &os, const Slice &slice);

struct Token {
	std::filesystem::path source;
	size_t row;
	size_t col;

	virtual void print(std::ostream &os) const = 0;
	virtual ~Token() = default;
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
	virtual ~Keyword() = default;
};

struct Primitive : public Token {
	Type type;
	Primitive(const Slice &s, Type type);
	virtual void print(std::ostream &os) const;
	void compile(std::ostream &outfile) const;
	static void compile(std::ostream &outfile, Type t);
	virtual ~Primitive() = default;
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
	virtual ~Punctuation() = default;
};

struct Identifier : public Token {
	std::string_view s;
	explicit Identifier(const Slice &s);
	explicit Identifier(const Identifier &id);
	virtual void print(std::ostream &os) const;
	void compile(std::ostream &outfile) const;
	static void compile(std::ostream &outfile, std::string_view s);
	virtual ~Identifier() = default;
};

#endif
