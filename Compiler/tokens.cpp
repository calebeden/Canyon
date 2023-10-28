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
    printf("Slice: %s\n", buf);
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

bool Primitive::isPrimitive(Slice s) {
    return s == "int";
}

bool Operator::isOperator(Slice s) {
    return s == "+";
}

Punctuation::Punctuation(Slice s) {
    if (s == "(") {
        type = Type::OpenParen;
    } else if (s == ")") {
        type == Type::CloseParen;
    } else if (s == ";") {
        type = Type::Semicolon;
    } else if (s == "{") {
        type = Type::OpenBrace;
    } else if (s == "}") {
        type = Type::CloseBrace;
    } else {
        throw std::invalid_argument(
              "Unknown slice argument: " + static_cast<std::string>(s));
    }
}

bool Punctuation::isPunctuation(Slice s) {
    return s == "(" || s == ")" || s == ";" || s == "{" || s == "}";
}
