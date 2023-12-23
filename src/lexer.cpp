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
        tokens->push_back(Token::createToken(s));
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

    if (slices.size() == 0) {
        fprintf(stderr, "File does not contain any source code");
        exit(1);
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
