#include "tokens.h"

#include <stdarg.h>
#include <stdexcept>
#include <string>

const char *const typeStr(Type type) {
    switch (type) {
        case Type::INT: {
            return "int";
        }
        case Type::BYTE: {
            return "byte";
        }
        case Type::SHORT: {
            return "short";
        }
        case Type::LONG: {
            return "long";
        }
        case Type::FLOAT: {
            return "float";
        }
        case Type::DOUBLE: {
            return "double";
        }
        case Type::BOOL: {
            return "bool";
        }
        case Type::CHAR: {
            return "char";
        }
        case Type::VOID: {
            return "void";
        }
        default: {
            return "UNKNOWN";
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

void Slice::show() const {
    char buf[len + 1];
    strncpy(buf, start, len);
    buf[len] = '\0';
    fprintf(stderr, "%s", buf);
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

void Token::show() const {
    fprintf(stderr, "Class: %s\n", typeid(*this).name());
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

Primitive::Primitive(Slice s, Type type) : Token(s.source, s.row, s.col), type(type) {
}

void Primitive::show() {
    fprintf(stderr, "Primitive: %s", typeStr(type));
}

void Primitive::compile(FILE *outfile) {
    switch (type) {
        case Type::INT: {
            fprintf(outfile, "int");
            return;
        }
        case Type::BYTE: {
            fprintf(outfile, "signed char");
            return;
        }
        case Type::SHORT: {
            fprintf(outfile, "short");
            return;
        }
        case Type::LONG: {
            fprintf(outfile, "long");
            return;
        }
        case Type::FLOAT: {
            fprintf(outfile, "float");
            return;
        }
        case Type::DOUBLE: {
            fprintf(outfile, "double");
            return;
        }
        case Type::BOOL: {
            fprintf(outfile, "bool");
            return;
        }
        case Type::CHAR: {
            fprintf(outfile, "unsigned char");
            return;
        }
        case Type::VOID: {
            fprintf(outfile, "void");
            return;
        }
        default: {
            fprintf(stderr, "Unknown type");
            exit(EXIT_FAILURE);
        }
    }
}

void Primitive::show(Type t) {
    fprintf(stderr, "Primitive: %s", typeStr(t));
}

void Primitive::compile(FILE *outfile, Type t) {
    switch (t) {
        case Type::INT: {
            fprintf(outfile, "int");
            return;
        }
        case Type::BYTE: {
            fprintf(outfile, "signed char");
            return;
        }
        case Type::SHORT: {
            fprintf(outfile, "short");
            return;
        }
        case Type::LONG: {
            fprintf(outfile, "long");
            return;
        }
        case Type::FLOAT: {
            fprintf(outfile, "float");
            return;
        }
        case Type::DOUBLE: {
            fprintf(outfile, "double");
            return;
        }
        case Type::BOOL: {
            fprintf(outfile, "bool");
            return;
        }
        case Type::CHAR: {
            fprintf(outfile, "unsigned char");
            return;
        }
        case Type::VOID: {
            fprintf(outfile, "void");
            return;
        }
        default: {
            fprintf(stderr, "Unknown type");
            exit(EXIT_FAILURE);
        }
    }
}

Punctuation::Punctuation(Slice s, Type type) : Token(s.source, s.row, s.col), type(type) {
}

Identifier::Identifier(Slice s) : Token(s.source, s.row, s.col), s(s) {
}

void Identifier::show() {
    fprintf(stderr, "Identifier: ");
    s.show();
    fprintf(stderr, "\n");
}

void Identifier::compile(FILE *outfile) {
    fprintf(outfile, "%.*s", (int) s.len, s.start);
}

bool Identifier::operator==(const Identifier &other) {
    return this->s == other.s;
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
