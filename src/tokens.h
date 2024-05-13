#ifndef TOKENS_H
#define TOKENS_H

#include <cstdint>
#include <filesystem>
#include <iostream>
#include <string_view>

enum class Type {
	/** 8 bit signed integer */
	I8,
	/** 16 bit signed integer */
	I16,
	/** 32 bit signed integer */
	I32,
	/** 64 bit signed integer */
	I64,
	/** 8 bit unsigned integer */
	U8,
	/** 16 bit unsigned integer */
	U16,
	/** 32 bit unsigned integer */
	U32,
	/** 64 bit unsigned integer */
	U64,
	/** 32 bit floating point number */
	F32,
	/** 64 bit floating point number */
	F64,
	/** 8 bit character value */
	C8,
	/** 16 bit character value */
	C16,
	/** 32 bit character value */
	C32,
	/** boolean true or false */
	BOOL,
	/** unit type */
	UNIT,
	/** unknown type */
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
	enum Type {
		RETURN,
		LET,
		FUN,
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
	static void compile(std::ostream &outfile, Type type);
	virtual ~Primitive() = default;
};

struct Punctuation : public Token {
	enum class Type {
		OpenParen,
		CloseParen,
		Semicolon,
		OpenBrace,
		CloseBrace,
		Period,
		Comma,
		Equals,
		Colon,
		Plus,
		Hyphen,
		Asterisk,
		ForwardSlash,
		Percent,
		Exclamation,
		LessThan,
		GreaterThan,
		Ampersand,
		VerticalBar,
		Tilde,
		Caret,
	};
	Type type;
	Punctuation(const Slice &s, Type type);
	virtual void print(std::ostream &os) const;
	virtual ~Punctuation() = default;
};

struct Operator : public Token {
	enum class Type {
		Comma,
		Assignment,
		Equality,
		Inequality,
		LessThan,
		LessThanOrEqual,
		GreaterThan,
		GreaterThanOrEqual,
		Addition,
		Subtraction,
		Multiplication,
		Division,
		Modulus,
		Scope,
		LogicalNot,
		LogicalAnd,
		LogicalOr,
		BitwiseNot,
		BitwiseAnd,
		BitwiseOr,
		BitwiseXor,
		BitwiseShiftLeft,
		BitwiseShiftRight,
	};
	Type type;
	explicit Operator(Token *t, Type type);
	virtual void print(std::ostream &os) const;
	virtual ~Operator() = default;
};

struct SymbolOrLiteral : public Token {
	std::string_view s;
	explicit SymbolOrLiteral(const Slice &s);
	explicit SymbolOrLiteral(const SymbolOrLiteral &s);
	virtual void print(std::ostream &os) const;
	void compile(std::ostream &outfile) const;
	static void compile(std::ostream &outfile, const std::string_view s);
	virtual ~SymbolOrLiteral() = default;
};

struct Symbol : public Token {
	std::string_view s;
	explicit Symbol(const Slice &s);
	explicit Symbol(const SymbolOrLiteral *const s);
	explicit Symbol(const Symbol &s);
	virtual void print(std::ostream &os) const;
	void compile(std::ostream &outfile) const;
	static void compile(std::ostream &outfile, const std::string_view s);
	virtual ~Symbol() = default;
};

struct IntegerLiteral : public Token {
	Type type;
	uint64_t value;
	explicit IntegerLiteral(const Token *t, Type type, uint64_t value);
	virtual void print(std::ostream &os) const;
	virtual ~IntegerLiteral() = default;
};

bool operator==(const IntegerLiteral &lhs, const IntegerLiteral &rhs);

struct Whitespace : public Token {
	explicit Whitespace(const Slice &s);
	virtual void print(std::ostream &os) const;
	virtual ~Whitespace() = default;
};

struct EndOfFile : public Token {
	EndOfFile();
	virtual void print(std::ostream &os) const;
	virtual ~EndOfFile() = default;
};

#endif
