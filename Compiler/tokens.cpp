#include "tokens.h"

#include <stdarg.h>
#include <stdexcept>

Slice::Slice(char *start, char *end, char *source, size_t row, size_t col)
    : start(start), len(end - start + 1), row(row), col(col), source(source) {
}

void Slice::show() {
    char buf[len + 1];
    strncpy(buf, start, len);
    buf[len] = '\0';
    fprintf(stderr, "%s", buf);
}

bool Slice::operator==(const std::string &rhs) {
    for (size_t i = 0; i < this->len; i++) {
        if (rhs[i] != this->start[i]) {
            return false;
        }
    }
    return rhs[this->len] == '\0';
}

bool Slice::operator==(const Slice &other) {
    size_t length = std::min(this->len, other.len);
    for (size_t i = 0; i < length; i++) {
        if (other.start[i] != this->start[i]) {
            return false;
        }
    }
    return this->len == other.len;
}

Slice::operator std::string() {
    return std::string(start, len);
}

Token *Token::createToken(Slice s) {
    if (Keyword::isKeyword(s)) {
        return new Keyword(s);
    }
    if (Primitive::isPrimitive(s)) {
        return new Primitive(s);
    }
    if (Punctuation::isPunctuation(s)) {
        return new Punctuation(s);
    }
    return new Identifier(s);
}

void Token::show() {
    fprintf(stderr, "Class: %s\n", typeid(*this).name());
}

Token::Token(char *source, size_t row, size_t col) : source(source), row(row), col(col) {
}

void Token::parse_error(const char *const format, ...) {
    fprintf(stderr, "Parse error at %s:%ld:%ld: ", source, row, col);
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fprintf(stderr, "\n");
    exit(EXIT_FAILURE);
}

Keyword::Keyword(Slice s) : Token(s.source, s.row, s.col) {
    if (s == "void") {
        type = Type::VOID;
    } else if (s == "return") {
        type = Type::RETURN;
    } else {
        throw std::invalid_argument(
              "Unknown slice argument: " + static_cast<std::string>(s));
    }
}

bool Keyword::isKeyword(Slice s) {
    return s == "void" || s == "return";
}

Primitive::Primitive(Slice s) : Token(s.source, s.row, s.col) {
    if (s == "int") {
        type = Type::INT;
    } else if (s == "byte") {
        type = Type::BYTE;
    } else if (s == "short") {
        type = Type::SHORT;
    } else if (s == "long") {
        type = Type::LONG;
    } else if (s == "float") {
        type = Type::FLOAT;
    } else if (s == "double") {
        type = Type::DOUBLE;
    } else if (s == "bool") {
        type = Type::BOOL;
    } else if (s == "char") {
        type = Type::CHAR;
    } else {
        throw std::invalid_argument(
              "Unknown slice argument: " + static_cast<std::string>(s));
    }
}

void Primitive::show() {
    fprintf(stderr, "Primitive: ");
    switch (type) {
        case Type::INT: {
            fprintf(stderr, "int");
            return;
        }
        case Type::BYTE: {
            fprintf(stderr, "byte");
            return;
        }
        case Type::SHORT: {
            fprintf(stderr, "short");
            return;
        }
        case Type::LONG: {
            fprintf(stderr, "long");
            return;
        }
        case Type::FLOAT: {
            fprintf(stderr, "float");
            return;
        }
        case Type::DOUBLE: {
            fprintf(stderr, "double");
            return;
        }
        case Type::BOOL: {
            fprintf(stderr, "bool");
            return;
        }
        case Type::CHAR: {
            fprintf(stderr, "char");
            return;
        }
        case Type::VOID: {
            fprintf(stderr, "void");
            return;
        }
    }
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
    }
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
    }
}

bool Primitive::isPrimitive(Slice s) {
    return s == "int" || s == "byte" || s == "short" || s == "long" || s == "float"
           || s == "double" || s == "bool" || s == "char";
}

Punctuation::Punctuation(Slice s) : Token(s.source, s.row, s.col) {
    if (s == "(") {
        type = Type::OpenParen;
    } else if (s == ")") {
        type = Type::CloseParen;
    } else if (s == ";") {
        type = Type::Semicolon;
    } else if (s == "{") {
        type = Type::OpenBrace;
    } else if (s == "}") {
        type = Type::CloseBrace;
    } else if (s == "=") {
        type = Type::Equals;
    } else if (s == "+") {
        type = Type::Plus;
    } else if (s == "-") {
        type = Type::Minus;
    } else if (s == "*") {
        type = Type::Times;
    } else if (s == "/") {
        type = Type::Divide;
    } else if (s == "%") {
        type = Type::Mod;
    } else {
        throw std::invalid_argument(
              "Unknown slice argument: " + static_cast<std::string>(s));
    }
}

bool Punctuation::isPunctuation(Slice s) {
    return s == "(" || s == ")" || s == ";" || s == "{" || s == "}" || s == "="
           || s == "+" || s == "-" || s == "*" || s == "/" || s == "%";
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
