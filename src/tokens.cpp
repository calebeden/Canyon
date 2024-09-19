#include "tokens.h"

#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string_view>

Slice::Slice(std::string_view contents, std::filesystem::path source, size_t row,
      size_t col)
    : contents(contents), source(std::move(source)), row(row), col(col) {
}

Slice Slice::merge(const Slice &start, const Slice &end) {
	if (start.source != end.source) {
		std::cerr << "Cannot merge slices from different sources";
		exit(EXIT_FAILURE);
	}
	return Slice(
	      std::string_view(start.contents.data(),
	            (end.contents.data() - start.contents.data()) + end.contents.size()),
	      start.source, start.row, start.col);
}

std::ostream &operator<<(std::ostream &os, const Slice &slice) {
	os << slice.contents;
	return os;
}

Token::Token(const Slice &s) : s(s) {
}

Keyword::Keyword(const Slice &s, Type type) : Token(s), type(type) {
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

Punctuation::Punctuation(const Slice &s, Type type) : Token(s), type(type) {
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

Operator::Operator(const Token &t, Type type) : Token(t.s), type(type) {
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

SymbolOrLiteral::SymbolOrLiteral(const Slice &s) : Token(s) {
}

void SymbolOrLiteral::print(std::ostream &os) const {
	os << s;
}

Symbol::Symbol(const Slice &s) : Token(s) {
}

Symbol::Symbol(const SymbolOrLiteral *const s) : Token(s->s) {
}

Symbol::Symbol(const Symbol &s) : Token(s.s) {
}

void Symbol::print(std::ostream &os) const {
	os << s;
}

IntegerLiteral::IntegerLiteral(const Token &t, Type type, uint64_t value)
    : Token(t.s), type(type), value(value) {
}

void IntegerLiteral::print(std::ostream &os) const {
	os << value << typeToStringView(type);
}

std::string_view IntegerLiteral::typeToStringView(Type type) {
	switch (type) {
		case Type::I8: {
			return "i8";
		}
		case Type::I16: {
			return "i16";
		}
		case Type::I32: {
			return "i32";
		}
		case Type::I64: {
			return "i64";
		}
		case Type::U8: {
			return "u8";
		}
		case Type::U16: {
			return "u16";
		}
		case Type::U32: {
			return "u32";
		}
		case Type::U64: {
			return "u64";
		}
		default: {
			std::cerr << "Unknown type";
			exit(EXIT_FAILURE);
		}
	}
}

bool operator==(const IntegerLiteral &lhs, const IntegerLiteral &rhs) {
	return (lhs.type == rhs.type) && (lhs.value == rhs.value);
}

Whitespace::Whitespace(const Slice &s) : Token(s) {
}

void Whitespace::print(std::ostream &os) const {
	os << " ";
}

EndOfFile::EndOfFile(const Slice &s) : Token(s) {
}

void EndOfFile::print(std::ostream &os) const {
	os << "EOF";
}
