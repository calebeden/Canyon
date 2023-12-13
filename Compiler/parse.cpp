#include "parse.h"

#include "ast.h"
#include "parseRvalue.h"
#include "tokens.h"
#include <unordered_map>

#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <stddef.h>
#include <stdexcept>
#include <stdlib.h>
#include <typeinfo>
#include <vector>

static char *start;
static char *current;

/**
 * @brief Determines whether a character is a token separator.
 * A character is a token separator if it is contained within {' ', '\t', '\n', '(', ')',
 * ';', '{', '}'}
 *
 * @param c the character to test
 * @returns if `c` is a token separator
 */
static inline bool isSep(char c);

/**
 * @brief Converts tokenized Slices into an abstract syntax tree representing the current
 * module
 *
 * @param slices the tokens to parse
 */
static Function *parseMain(std::vector<Slice> slices);

/**
 * @brief Parses tokens into a code block
 *
 * @param slices the vector of tokens to parse from
 * @param i a reference to the index in the vector corresponding to the token IMMEDIATELY
 * AFTER the opening curly brace. By the end of the function it will contain the index
 * corresponding to the token IMMEDIATELY AFTER the closing curly brace
 * @return the parsed CodeBlock
 */
static CodeBlock *parseBlock(std::vector<Token *> tokens, size_t &i);
/**
 * @brief Parses tokens into a statement
 *
 * @param slices the vector of tokens to parse from
 * @param i a reference to the index in the vector corresponding to the first token of the
 * statement. By the end of the function it will contain the index corresponding to the
 * token IMMEDIATELY AFTER the statement
 * @param locals the mapping of variables and types for the current code block, will
 * insert into this map if the statement is a declaration
 * @return the parsed Statement
 */
static Statement *parseStatement(std::vector<Token *> tokens, size_t &i,
      std::unordered_map<Identifier *, Primitive *, Hasher, Comparator> *locals);

AST *tokenize(char *program, off_t size, char *source) {
    std::vector<Slice> slices;
    start = current = program;
    size_t line = 1;
    size_t col = 1;

    // Tokenize the input source file into Slices, skipping over whitespace
    while (current - start < size) {
        while (*current == ' ' || *current == '\t' || *current == '\n') {
            if (*current == '\n') {
                line++;
                col = 1;
            } else if (*current == '\t') {
                col += TAB_WIDTH;
            } else {
                col++;
            }
            current++;
        }
        if (current - start >= size) {
            break;
        }
        char *tokenStart = current;
        size_t startCol = col;
        do {
            current++;
            col++;
        } while (!isSep(*current) && current - start < size);
        slices.emplace_back(tokenStart, current - 1, source, line, startCol);
    }

    if (slices.size() == 0) {
        fprintf(stderr, "File does not contain any source code");
        exit(1);
    }

    Function *canyonMain = parseMain(slices);
    AST *ast = new AST;
    ast->functions.push_back(canyonMain);
    return ast;
}

// TODO generalize to multiple functions
static Function *parseMain(std::vector<Slice> slices) {
    std::vector<Token *> tokens;
    for (Slice s : slices) {
        s.show();
        fprintf(stderr, "\n");
        tokens.push_back(Token::createToken(s));
    }

    for (Token *t : tokens) {
        t->show();
        fprintf(stderr, "\n");
    }

    size_t i = 5;
    CodeBlock *block = parseBlock(tokens, i);
    for (Statement *s : block->statements) {
        s->show();
    }
    Function *canyonMain = new Function;
    canyonMain->body = block;
    return canyonMain;
}

static CodeBlock *parseBlock(std::vector<Token *> tokens, size_t &i) {
    CodeBlock *block = new CodeBlock();
    while (dynamic_cast<Punctuation *>(tokens[i]) == nullptr
           || static_cast<Punctuation *>(tokens[i])->type
                    != Punctuation::Type::CloseBrace) {
        Statement *s = parseStatement(tokens, i, block->locals);
        if (s != nullptr) {
            block->statements.emplace_back(s);
        }
    }
    return block;
}

static Statement *parseStatement(std::vector<Token *> tokens, size_t &i,
      std::unordered_map<Identifier *, Primitive *, Hasher, Comparator> *locals) {
    while (typeid(*tokens[i]) == typeid(Punctuation)
           && static_cast<Punctuation *>(tokens[i])->type
                    != Punctuation::Type::Semicolon) {
        i++;
    }

    rvalue *rval;
    if (typeid(*tokens[i]) == typeid(Primitive)) {
        Primitive *type = static_cast<Primitive *>(tokens[i]);
        i++;
        if (typeid(*tokens[i]) == typeid(Identifier)) {
            Identifier *id = static_cast<Identifier *>(tokens[i]);
            if (locals->find(id) != locals->end()) {
                tokens[i]->parse_error("Re-declaration of variable %.*s", id->s.len,
                      id->s.start);
            }
            locals->insert({id, type});
            size_t i2 = i;
            rval = parseRvalue(tokens, i, locals);
            if (typeid(*rval) != typeid(Assignment)) {
                i = i2;
                return nullptr;
            }
        }
    } else if (typeid(*tokens[i]) == typeid(Keyword)
               && static_cast<Keyword *>(tokens[i])->type == Keyword::Type::RETURN) {
        i++;
        rval = parseRvalue(tokens, i, locals);
        if (typeid(*tokens[i]) != typeid(Punctuation)
              || static_cast<Punctuation *>(tokens[i])->type
                       != Punctuation::Type::Semicolon) {
            tokens[i]->parse_error("Expected ';' after statement");
        }
        return new Return(rval);
    } else {
        rval = parseRvalue(tokens, i, locals);
    }

    if (typeid(*tokens[i]) != typeid(Punctuation)
          || static_cast<Punctuation *>(tokens[i])->type
                   != Punctuation::Type::Semicolon) {
        tokens[i]->parse_error("Expected ';' after statement");
    }
    if (rval != nullptr) {
        return new Expression(rval);
    }
    return nullptr;
}

static inline bool isSep(char c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '(' || c == ')' || c == ';'
           || c == '{' || c == '}' || c == '=';
}
