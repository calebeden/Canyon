#include "tokens.h"

#include <cstdarg>
#include <iostream>
#include <stdexcept>
#include <string>

std::ostream &operator<<(std::ostream &os, const Type &type) {
	switch (type) {
		case Type::INT: {
			os << "int";
			return os;
		}
		case Type::BYTE: {
			os << "byte";
			return os;
		}
		case Type::SHORT: {
			os << "short";
			return os;
		}
		case Type::LONG: {
			os << "long";
			return os;
		}
		case Type::FLOAT: {
			os << "float";
			return os;
		}
		case Type::DOUBLE: {
			os << "double";
			return os;
		}
		case Type::BOOL: {
			os << "bool";
			return os;
		}
		case Type::CHAR: {
			os << "char";
			return os;
		}
		case Type::VOID: {
			os << "void";
			return os;
		}
		default: {
			os << "UNKNOWN";
			return os;
		}
	}
}

Slice::Slice(const char *const start, const char *const end, const char *const source,
      size_t row, size_t col)
    : start(start), len(end - start + 1), row(row), col(col), source(source) {
}

Slice::Slice(const char *const start, size_t len, const char *const source, size_t row,
      size_t col)
    : start(start), len(len), row(row), col(col), source(source) {
}

bool Slice::operator==(const std::string &rhs) const {
	for (size_t i = 0; i < this->len; i++) {
		if (rhs[i] != this->start[i]) {
			return false;
		}
	}
	return rhs[this->len] == '\0';
}

bool Slice::operator==(const Slice &other) const {
	size_t length = std::min(this->len, other.len);
	if (this->len != other.len) {
		return false;
	}
	for (size_t i = 0; i < length; i++) {
		if (other.start[i] != this->start[i]) {
			return false;
		}
	}
	return true;
}

bool Slice::operator==(const char *const other) const {
	return strncmp(this->start, other, this->len) == 0 && other[this->len] == '\0';
}

Slice::operator std::string() const {
	return std::string(start, len);
}

std::ostream &operator<<(std::ostream &os, const Slice &slice) {
	os.write(slice.start, slice.len);
	return os;
}

Token::Token(const char *const source, size_t row, size_t col)
    : source(source), row(row), col(col) {
}

void Token::error(const char *const format, ...) const {
	fprintf(stderr, "Error at %s:%ld:%ld: ", source, row, col);
	va_list args;
	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);
	fprintf(stderr, "\n");
	exit(EXIT_FAILURE);
}

Keyword::Keyword(Slice s, Type type) : Token(s.source, s.row, s.col), type(type) {
}

void Keyword::print(std::ostream &os) const {
	os << "Keyword: ";
	switch (type) {
		case Type::VOID: {
			os << "void";
			return;
		}
		case Type::RETURN: {
			os << "return";
			return;
		}
		default: {
			os << "UNKNOWN";
			return;
		}
	}
}

Primitive::Primitive(Slice s, Type type) : Token(s.source, s.row, s.col), type(type) {
}

void Primitive::print(std::ostream &os) const {
	os << "Primitive: " << type;
}

void Primitive::compile(std::ostream &outfile) {
	switch (type) {
		case Type::INT: {
			outfile << "int";
			return;
		}
		case Type::BYTE: {
			outfile << "signed char";
			return;
		}
		case Type::SHORT: {
			outfile << "short";
			return;
		}
		case Type::LONG: {
			outfile << "long";
			return;
		}
		case Type::FLOAT: {
			outfile << "float";
			return;
		}
		case Type::DOUBLE: {
			outfile << "double";
			return;
		}
		case Type::BOOL: {
			outfile << "bool";
			return;
		}
		case Type::CHAR: {
			outfile << "unsigned char";
			return;
		}
		case Type::VOID: {
			outfile << "void";
			return;
		}
		default: {
			std::cerr << "Error compiling primitive: Unknown type\n";
			exit(EXIT_FAILURE);
		}
	}
}

void Primitive::compile(std::ostream &outfile, Type t) {
	switch (t) {
		case Type::INT: {
			outfile << "int";
			return;
		}
		case Type::BYTE: {
			outfile << "signed char";
			return;
		}
		case Type::SHORT: {
			outfile << "short";
			return;
		}
		case Type::LONG: {
			outfile << "long";
			return;
		}
		case Type::FLOAT: {
			outfile << "float";
			return;
		}
		case Type::DOUBLE: {
			outfile << "double";
			return;
		}
		case Type::BOOL: {
			outfile << "bool";
			return;
		}
		case Type::CHAR: {
			outfile << "unsigned char";
			return;
		}
		case Type::VOID: {
			outfile << "void";
			return;
		}
		default: {
			std::cerr << "Error compiling primitive: Unknown type\n";
			exit(EXIT_FAILURE);
		}
	}
}

Punctuation::Punctuation(Slice s, Type type) : Token(s.source, s.row, s.col), type(type) {
}

void Punctuation::print(std::ostream &os) const {
	switch (type) {
		case Type::OpenParen: {
			os << '(';
			return;
		}
		case Type::CloseParen: {
			os << ')';
			return;
		}
		case Type::Semicolon: {
			os << ';';
			return;
		}
		case Type::OpenBrace: {
			os << '{';
			return;
		}
		case Type::CloseBrace: {
			os << '}';
			return;
		}
		case Type::Comma: {
			os << ',';
			return;
		}
		case Type::Equals: {
			os << '=';
			return;
		}
		case Type::Plus: {
			os << '+';
			return;
		}
		case Type::Minus: {
			os << '-';
			return;
		}
		case Type::Times: {
			os << '*';
			return;
		}
		case Type::Divide: {
			os << '/';
			return;
		}
		case Type::Mod: {
			os << '%';
			return;
		}
		default: {
			os << "UNKNOWN PUNCTUATION";
			return;
		}
	}
}

Identifier::Identifier(Slice s) : Token(s.source, s.row, s.col), s(s) {
}

void Identifier::print(std::ostream &os) const {
	os << "Identifier: " << s << '\n';
}

void Identifier::compile(std::ostream &outfile) {
	outfile << s;
}

std::size_t Hasher::operator()(Identifier *const id) const {
	// djb2
	size_t hash = 5381;
	for (size_t i = 0; i < id->s.len; i++) {
		hash = hash * 33 + id->s.start[i];
	}
	return hash;
}

bool Comparator::operator()(Identifier *a, Identifier *b) const {
	return a->s == b->s;
}
