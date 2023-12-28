#include "lexer.h"

#include "ast.h"

#include <stdexcept>
#include <vector>

Lexer::Lexer(const char *const program, off_t size, const char *const source,
      uint32_t tabSize)
    : program(program), size(size), source(source), tabSize(tabSize) {
    if (tabSize == 0) {
        throw std::invalid_argument("Tab size must be greater than 0");
    }
}

std::vector<Token *> *Lexer::tokenize() {
    slice();

    std::vector<Token *> *tokens = new std::vector<Token *>();
    for (Slice s : slices) {
        s.show();
        fprintf(stderr, "\n");
        Token *token;
        Keyword *keyword = createKeyword(s);
        if (keyword) {
            token = keyword;
        } else {
            Primitive *primitive = createPrimitive(s);
            if (primitive) {
                token = primitive;
            } else {
                Punctuation *punctuation = createPunctuation(s);
                if (punctuation) {
                    token = punctuation;
                } else {
                    token = createIdentifier(s);
                }
            }
        }
        tokens->push_back(token);
    }

    for (Token *t : *tokens) {
        t->show();
        fprintf(stderr, "\n");
    }

    return tokens;
}

void Lexer::slice() {
    current = program;
    size_t line = 1;
    size_t col = 1;

    while (current - program < size) {
        while (isspace(*current)) {
            if (*current == '\n') {
                line++;
                col = 1;
            } else if (*current == '\r') {
                line++;
                col = 1;
                if (current + 1 - program < size && current[1] == '\n') {
                    current++;
                }
            } else if (*current == '\t') {
                col = ((col + tabSize - 1) / tabSize) * tabSize + 1;
            } else {
                col++;
            }
            current++;
        }
        if (current - program >= size) {
            break;
        }
        const char *tokenStart = current;
        size_t startCol = col;
        do {
            current++;
            col++;
        } while (current - program < size && !isSep(current));
        slices.emplace_back(tokenStart, current - 1, source, line, startCol);
    }
}

bool Lexer::isSep(const char *const c) {
    // If c is any of these characters, it is by default a separator
    if (isspace(c[0]) || c[0] == '(' || c[0] == ')' || c[0] == ';' || c[0] == '{'
          || c[0] == '}' || c[0] == ',' || c[0] == '+' || c[0] == '-' || c[0] == '*'
          || c[0] == '/' || c[0] == '%' || c[0] == '=') {
        return true;
    }

    // If c is alnum, it is sep as long as prev is not alnum
    if (isalnum(c[0]) && !isalnum(c[-1])) {
        return true;
    }

    return false;
}

Keyword *Lexer::createKeyword(Slice s) {
    if (s == "void") {
        return new Keyword(s, Keyword::Type::VOID);
    }
    if (s == "return") {
        return new Keyword(s, Keyword::Type::RETURN);
    }
    return nullptr;
}

Primitive *Lexer::createPrimitive(Slice s) {
    if (s == "int") {
        return new Primitive(s, Type::INT);
    }
    if (s == "byte") {
        return new Primitive(s, Type::BYTE);
    }
    if (s == "short") {
        return new Primitive(s, Type::SHORT);
    }
    if (s == "long") {
        return new Primitive(s, Type::LONG);
    }
    if (s == "float") {
        return new Primitive(s, Type::FLOAT);
    }
    if (s == "double") {
        return new Primitive(s, Type::DOUBLE);
    }
    if (s == "bool") {
        return new Primitive(s, Type::BOOL);
    }
    if (s == "char") {
        return new Primitive(s, Type::CHAR);
    }
    return nullptr;
}

Punctuation *Lexer::createPunctuation(Slice s) {
    if (s == "(") {
        return new Punctuation(s, Punctuation::Type::OpenParen);
    }
    if (s == ")") {
        return new Punctuation(s, Punctuation::Type::CloseParen);
    }
    if (s == ";") {
        return new Punctuation(s, Punctuation::Type::Semicolon);
    }
    if (s == "{") {
        return new Punctuation(s, Punctuation::Type::OpenBrace);
    }
    if (s == "}") {
        return new Punctuation(s, Punctuation::Type::CloseBrace);
    }
    if (s == ",") {
        return new Punctuation(s, Punctuation::Type::Comma);
    }
    if (s == "=") {
        return new Punctuation(s, Punctuation::Type::Equals);
    }
    if (s == "+") {
        return new Punctuation(s, Punctuation::Type::Plus);
    }
    if (s == "-") {
        return new Punctuation(s, Punctuation::Type::Minus);
    }
    if (s == "*") {
        return new Punctuation(s, Punctuation::Type::Times);
    }
    if (s == "/") {
        return new Punctuation(s, Punctuation::Type::Divide);
    }
    if (s == "%") {
        return new Punctuation(s, Punctuation::Type::Mod);
    }
    return nullptr;
}

Identifier *Lexer::createIdentifier(Slice s) {
    return new Identifier(s);
}
