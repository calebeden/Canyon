#ifndef TOKENS_H
#define TOKENS_H

#include <cstdint>
#include <filesystem>
#include <iostream>
#include <string_view>

/// Various classes used to represent lexed source code, as well as general tools to
/// manage source code in memory

struct Slice {
	std::string_view contents;
	std::filesystem::path source;
	size_t row;
	size_t col;
	Slice(std::string_view contents, std::filesystem::path source, size_t row,
	      size_t col);
	Slice(const Slice &s) = default;
	~Slice() = default;
	/**
	 * @brief Generates a new Slice beginning at start through end, picking up any
	 * intermediate characters from the underlying source code, regardless of whether they
	 * were in either of the parameter Slices.
	 *
	 * @param start the Slice to start from
	 * @param end the Slice to end at
	 * @return a new Slice containing the characters from the beginning of ` to the
	 * end of `end`
	 */
	static Slice merge(const Slice &start, const Slice &end);
};

std::ostream &operator<<(std::ostream &os, const Slice &slice);

struct Token {
	Slice s;

	virtual void print(std::ostream &os) const = 0;
	virtual ~Token() = default;
protected:
	Token(const Slice &s);
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
	explicit Operator(const Token &t, Type type);
	virtual void print(std::ostream &os) const;
	virtual ~Operator() = default;
};

struct SymbolOrLiteral : public Token {
	explicit SymbolOrLiteral(const Slice &s);
	explicit SymbolOrLiteral(const SymbolOrLiteral &s) = default;
	virtual void print(std::ostream &os) const;
	virtual ~SymbolOrLiteral() = default;
};

struct Symbol : public Token {
	explicit Symbol(const Slice &s);
	explicit Symbol(const SymbolOrLiteral *const s);
	explicit Symbol(const Symbol &s);
	virtual void print(std::ostream &os) const;
	virtual ~Symbol() = default;
};

struct IntegerLiteral : public Token {
	enum class Type {
		I8,
		I16,
		I32,
		I64,
		U8,
		U16,
		U32,
		U64,
	};
	Type type;
	uint64_t value;
	explicit IntegerLiteral(const Token &t, Type type, uint64_t value);
	virtual void print(std::ostream &os) const;
	static std::string_view typeToStringView(Type type);
	virtual ~IntegerLiteral() = default;
};

bool operator==(const IntegerLiteral &lhs, const IntegerLiteral &rhs);

struct BoolLiteral : public Token {
	bool value;
	explicit BoolLiteral(const Token &t, bool value);
	virtual void print(std::ostream &os) const;
	virtual ~BoolLiteral() = default;
};

bool operator==(const BoolLiteral &lhs, const BoolLiteral &rhs);

struct Whitespace : public Token {
	explicit Whitespace(const Slice &s);
	virtual void print(std::ostream &os) const;
	virtual ~Whitespace() = default;
};

struct EndOfFile : public Token {
	EndOfFile(const Slice &s);
	virtual void print(std::ostream &os) const;
	virtual ~EndOfFile() = default;
};

#endif
