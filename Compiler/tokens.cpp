#include "tokens.h"

#include <stdexcept>

Slice::Slice(char *start, int len) : start(start), len(len) {
}

Slice::Slice(char *start, char *end) : start(start), len(end - start + 1) {
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
    if (Operator::isOperator(s)) {
        return new Operator(s);
    }
    if (Punctuation::isPunctuation(s)) {
        return new Punctuation(s);
    }
    return new Identifier(s);
}

Keyword::Keyword(Slice s) {
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

Primitive::Primitive(Slice s) {
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
    }
}

bool Primitive::isPrimitive(Slice s) {
    return s == "int" || s == "byte" || s == "short" || s == "long" || s == "float"
           || s == "double" || s == "bool" || s == "char";
}

bool Operator::isOperator(Slice s) {
    return s == "+";
}

Operator::Operator(Slice s) {
    throw std::invalid_argument("Unknown slice argument: " + static_cast<std::string>(s));
}

Punctuation::Punctuation(Slice s) {
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
    } else {
        throw std::invalid_argument(
              "Unknown slice argument: " + static_cast<std::string>(s));
    }
}

bool Punctuation::isPunctuation(Slice s) {
    return s == "(" || s == ")" || s == ";" || s == "{" || s == "}" || s == "=";
}

void Identifier::show() {
    s.show();
}
