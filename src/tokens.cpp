#include "tokens.h"

#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string_view>

std::ostream &operator<<(std::ostream &os, Type type) {
	switch (type) {
		case Type::I8: {
			return os << "i8";
		}
		case Type::I16: {
			return os << "i16";
		}
		case Type::I32: {
			return os << "i32";
		}
		case Type::I64: {
			return os << "i64";
		}
		case Type::U8: {
			return os << "u8";
		}
		case Type::U16: {
			return os << "u16";
		}
		case Type::U32: {
			return os << "u32";
		}
		case Type::U64: {
			return os << "u64";
		}
		case Type::F32: {
			return os << "f32";
		}
		case Type::F64: {
			return os << "f64";
		}
		case Type::C8: {
			return os << "c8";
		}
		case Type::C16: {
			return os << "c16";
		}
		case Type::C32: {
			return os << "c32";
		}
		case Type::BOOL: {
			return os << "bool";
		}
		case Type::UNIT: {
			return os << "()";
		}
		case Type::UNKNOWN: {
			return os << "UNKNOWN";
		}
		default: {
			std::cerr << "Unknown type";
			exit(EXIT_FAILURE);
		}
	}
}

Slice::Slice(std::string_view contents, std::filesystem::path source, size_t row,
      size_t col)
    : contents(contents), source(std::move(source)), row(row), col(col) {
}

std::ostream &operator<<(std::ostream &os, const Slice &slice) {
	os << slice.contents;
	return os;
}

Token::Token(std::filesystem::path source, size_t row, size_t col)
    : source(std::move(source)), row(row), col(col) {
}

Keyword::Keyword(const Slice &s, Type type) : Token(s.source, s.row, s.col), type(type) {
}

void Keyword::print(std::ostream &os) const {
	os << "Keyword: ";
	switch (type) {
		case Type::RETURN: {
			os << "return";
			break;
		}
		case Type::LET: {
			os << "let";
			break;
		}
		case Type::FUN: {
			os << "fun";
			break;
		}
	}
}

Primitive::Primitive(const Slice &s, Type type)
    : Token(s.source, s.row, s.col), type(type) {
}

void Primitive::print(std::ostream &os) const {
	os << "Primitive: " << type;
}

void Primitive::compile(std::ostream &outfile) const {
	return Primitive::compile(outfile, type);
}

void Primitive::compile(std::ostream &outfile, Type type) {
	switch (type) {
		case Type::I8: {
			outfile << "int8_t";
			return;
		}
		case Type::I16: {
			outfile << "int16_t";
			return;
		}
		case Type::I32: {
			outfile << "int32_t";
			return;
		}
		case Type::I64: {
			outfile << "int64_t";
			return;
		}
		case Type::U8: {
			outfile << "uint8_t";
			return;
		}
		case Type::U16: {
			outfile << "uint16_t";
			return;
		}
		case Type::U32: {
			outfile << "uint32_t";
			return;
		}
		case Type::U64: {
			outfile << "uint64_t";
			return;
		}
		case Type::F32: {
			outfile << "_Float32";
			return;
		}
		case Type::F64: {
			outfile << "_Float64";
			return;
		}
		case Type::C8: {
			outfile << "char8_t";
			return;
		}
		case Type::C16: {
			outfile << "char16_t";
			return;
		}
		case Type::C32: {
			outfile << "chat32_t";
			return;
		}
		case Type::BOOL: {
			outfile << "bool";
			return;
		}
		case Type::UNIT: {
			outfile << "void";
			return;
		}
		case Type::UNKNOWN: {
			std::cerr << "Unknown type in Primitive::compile";
			exit(EXIT_FAILURE);
		}
		default: {
			std::cerr << "Unknown type case in Primitive::compile";
			exit(EXIT_FAILURE);
		}
	}
}

Punctuation::Punctuation(const Slice &s, Type type)
    : Token(s.source, s.row, s.col), type(type) {
}

void Punctuation::print(std::ostream &os) const {
	switch (type) {
		case Type::OpenParen: {
			os << '(';
			break;
		}
		case Type::CloseParen: {
			os << ')';
			break;
		}
		case Type::Semicolon: {
			os << ';';
			break;
		}
		case Type::OpenBrace: {
			os << '{';
			break;
		}
		case Type::CloseBrace: {
			os << '}';
			break;
		}
		case Type::Comma: {
			os << ',';
			break;
		}
		case Type::Equals: {
			os << '=';
			break;
		}
		case Type::Colon: {
			os << ':';
			break;
		}
		case Type::Plus: {
			os << '+';
			break;
		}
		case Type::Hyphen: {
			os << '-';
			break;
		}
		case Type::Asterisk: {
			os << '*';
			break;
		}
		case Type::ForwardSlash: {
			os << '/';
			break;
		}
		case Type::Percent: {
			os << '%';
			break;
		}
		case Type::Exclamation: {
			os << '!';
			break;
		}
		case Type::LessThan: {
			os << '<';
			break;
		}
		case Type::GreaterThan: {
			os << '>';
			break;
		}
		case Type::Ampersand: {
			os << '&';
			break;
		}
		case Type::VerticalBar: {
			os << '|';
			break;
		}
		case Type::Tilde: {
			os << '~';
			break;
		}
		case Type::Caret: {
			os << '^';
			break;
		}
		case Type::Period: {
			os << '.';
			break;
		}
	}
}

Operator::Operator(Token *t, Type type) : Token(t->source, t->row, t->col), type(type) {
}

void Operator::print(std::ostream &os) const {
	switch (type) {
		case Type::Comma: {
			os << ',';
			break;
		}
		case Type::Assignment: {
			os << '=';
			break;
		}
		case Type::Equality: {
			os << "==";
			break;
		}
		case Type::Inequality: {
			os << "!=";
			break;
		}
		case Type::LessThan: {
			os << '<';
			break;
		}
		case Type::LessThanOrEqual: {
			os << "<=";
			break;
		}
		case Type::GreaterThan: {
			os << '>';
			break;
		}
		case Type::GreaterThanOrEqual: {
			os << ">=";
			break;
		}
		case Type::Addition: {
			os << '+';
			break;
		}
		case Type::Subtraction: {
			os << '-';
			break;
		}
		case Type::Multiplication: {
			os << '*';
			break;
		}
		case Type::Division: {
			os << '/';
			break;
		}
		case Type::Modulus: {
			os << '%';
			break;
		}
		case Type::Scope: {
			os << "::";
			break;
		}
		case Type::LogicalNot: {
			os << '!';
			break;
		}
		case Type::LogicalAnd: {
			os << "&&";
			break;
		}
		case Type::LogicalOr: {
			os << "||";
			break;
		}
		case Type::BitwiseNot: {
			os << '~';
			break;
		}
		case Type::BitwiseAnd: {
			os << '&';
			break;
		}
		case Type::BitwiseOr: {
			os << '|';
			break;
		}
		case Type::BitwiseXor: {
			os << '^';
			break;
		}
		case Type::BitwiseShiftLeft: {
			os << "<<";
			break;
		}
		case Type::BitwiseShiftRight: {
			os << ">>";
			break;
		}
	}
}

SymbolOrLiteral::SymbolOrLiteral(const Slice &s)
    : Token(s.source, s.row, s.col), s(s.contents) {
}

SymbolOrLiteral::SymbolOrLiteral(const SymbolOrLiteral &s)
    : Token(s.source, s.row, s.col), s(s.s) {
}

void SymbolOrLiteral::print(std::ostream &os) const {
	os << s;
}

void SymbolOrLiteral::compile(std::ostream &outfile) const {
	outfile << s;
}

void SymbolOrLiteral::compile(std::ostream &outfile, const std::string_view s) {
	outfile << s;
}

Symbol::Symbol(const Slice &s) : Token(s.source, s.row, s.col), s(s.contents) {
}

Symbol::Symbol(const SymbolOrLiteral *const s)
    : Token(s->source, s->row, s->col), s(s->s) {
}

Symbol::Symbol(const Symbol &s) : Token(s.source, s.row, s.col), s(s.s) {
}

void Symbol::print(std::ostream &os) const {
	os << s;
}

void Symbol::compile(std::ostream &outfile) const {
	outfile << s;
}

void Symbol::compile(std::ostream &outfile, const std::string_view s) {
	outfile << s;
}

IntegerLiteral::IntegerLiteral(const Token *t, Type type, uint64_t value)
    : Token(t->source, t->row, t->col), type(type), value(value) {
}

void IntegerLiteral::print(std::ostream &os) const {
	os << "IntegerLiteral: " << value << '\n';
}

bool operator==(const IntegerLiteral &lhs, const IntegerLiteral &rhs) {
	return (lhs.type == rhs.type) && (lhs.value == rhs.value);
}

Whitespace::Whitespace(const Slice &s) : Token(s.source, s.row, s.col) {
}

void Whitespace::print(std::ostream &os) const {
	os << " ";
}

EndOfFile::EndOfFile() : Token("", 0, 0) {
}

void EndOfFile::print(std::ostream &os) const {
	os << "EOF";
}
