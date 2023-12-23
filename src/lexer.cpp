#include "lexer.h"

#include "ast.h"

#include <vector>

Lexer::Lexer(const char *const program, off_t size, const char *const source)
    : program(program), size(size), source(source) {
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
        while (*current == ' ' || *current == '\t' || *current == '\n') {
            if (*current == '\n') {
                line++;
                col = 1;
            } else if (*current == '\t') {
                col = ((col + TAB_WIDTH - 1) / TAB_WIDTH) * TAB_WIDTH + 1;
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
        } while (!isSep(current) && current - program < size);
        slices.emplace_back(tokenStart, current - 1, source, line, startCol);
    }

    if (slices.size() == 0) {
        fprintf(stderr, "File does not contain any source code");
        exit(1);
    }
}

bool Lexer::isSep(const char *const c) {
    // If c is any of these characters, it is by default a separator
    if (c[0] == ' ' || c[0] == '\t' || c[0] == '\n' || c[0] == '(' || c[0] == ')'
          || c[0] == ';' || c[0] == '{' || c[0] == '}' || c[0] == ',' || c[0] == '+'
          || c[0] == '-' || c[0] == '*' || c[0] == '/' || c[0] == '%' || c[0] == '=') {
        return true;
    }

    // If c is alnum, it is sep as long as prev is not alnum
    if (isalnum(c[0]) && !isalnum(c[-1])) {
        return true;
    }

    return false;
}
