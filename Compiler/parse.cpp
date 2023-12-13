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

using namespace AST;

static char *start;
static char *current;

/**
 * @brief Determines whether the character pointed to by c represents the start of a new
 * token
 *
 * @param c the character to test at
 * @returns whether `c` is at the start of a new token
 */
static inline bool isSep(char *c);

/**
 * @brief Converts tokenized Slices into an abstract syntax tree representing the current
 * module
 *
 * @param slices the tokens to parse
 * @return the AST of the module
 */
static AST::AST *parse(std::vector<Slice> slices);

/**
 * @brief Converts tokens to the AST representation of all functions in the current module
 *
 * @param tokens the stream of tokens representing the current module
 * @param ast the AST to place the parsed functions inside of
 */
static void parseFunctions(std::vector<Token *> tokens, AST::AST *ast);

/**
 * @brief Converts tokens to the AST representation of a function
 *
 * @param tokens the stream of tokens representing the current function
 * @param i a reference to the index in the vector corresponding to the token which is
 * expected to be the function type. By the end of the function it will contain the index
 * corresponding to the token IMMEDIATELY AFTER the closing curly brace
 * @param ast the AST to place the parsed function inside of
 */
static void parseFunction(std::vector<Token *> tokens, size_t &i, AST::AST *ast);

/**
 * @brief Parses tokens into a code block
 *
 * @param slices the vector of tokens to parse from
 * @param i a reference to the index in the vector corresponding to the token IMMEDIATELY
 * AFTER the opening curly brace. By the end of the block it will contain the index
 * corresponding to the token IMMEDIATELY AFTER the closing curly brace
 * @param ast the global context
 * @return the parsed CodeBlock
 */
static CodeBlock *parseBlock(std::vector<Token *> tokens, size_t &i, AST::AST *ast);
/**
 * @brief Parses tokens into a statement
 *
 * @param slices the vector of tokens to parse from
 * @param i a reference to the index in the vector corresponding to the first token of the
 * statement. By the end of the function it will contain the index corresponding to the
 * token IMMEDIATELY AFTER the statement
 * @param context the most specific context that the statement is a part of
 * @return the parsed Statement
 */
static Statement *parseStatement(std::vector<Token *> tokens, size_t &i,
      CodeBlock *context);

AST::AST *tokenize(char *program, off_t size, char *source) {
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
        } while (!isSep(current) && current - start < size);
        slices.emplace_back(tokenStart, current - 1, source, line, startCol);
    }

    if (slices.size() == 0) {
        fprintf(stderr, "File does not contain any source code");
        exit(1);
    }

    return parse(slices);
}

static AST::AST *parse(std::vector<Slice> slices) {
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

    AST::AST *ast = new AST::AST;
    parseFunctions(tokens, ast);
    if (ast->functions.find("canyonMain") == ast->functions.end()) {
        fprintf(stderr, "Parse error: no main function\n");
        exit(EXIT_FAILURE);
    }
    ast->resolve();
    return ast;
}

static void parseFunctions(std::vector<Token *> tokens, AST::AST *ast) {
    size_t i = 0;
    while (i < tokens.size()) {
        parseFunction(tokens, i, ast);
    }
}

static void parseFunction(std::vector<Token *> tokens, size_t &i, AST::AST *ast) {
    Primitive::Type type;
    if (typeid(*tokens[i]) == typeid(Primitive)) {
        type = static_cast<Primitive *>(tokens[i])->type;
    } else if (typeid(*tokens[i]) == typeid(Keyword)
               && static_cast<Keyword *>(tokens[i])->type == Keyword::Type::VOID) {
        type = Primitive::Type::VOID;
    } else {
        tokens[i]->error("Expected function type");
    }
    i++;
    if (typeid(*tokens[i]) != typeid(Identifier)) {
        tokens[i]->error("Expected identifier");
    }
    std::string name = static_cast<Identifier *>(tokens[i])->s;
    if (name == "main") {
        name = "canyonMain";
    }
    i++;
    if (typeid(*tokens[i]) != typeid(Punctuation)
          && static_cast<Punctuation *>(tokens[i])->type
                   != Punctuation::Type::OpenParen) {
        tokens[i]->error("Expected '('");
    }
    i++;
    if (typeid(*tokens[i]) != typeid(Punctuation)
          && static_cast<Punctuation *>(tokens[i])->type
                   != Punctuation::Type::CloseParen) {
        tokens[i]->error("Expected ')'");
    }
    i++;
    if (typeid(*tokens[i]) != typeid(Punctuation)
          && static_cast<Punctuation *>(tokens[i])->type
                   != Punctuation::Type::OpenBrace) {
        tokens[i]->error("Expected '{'");
    }
    i++;

    CodeBlock *block = parseBlock(tokens, i, ast);

    if (typeid(*tokens[i]) != typeid(Punctuation)
          && static_cast<Punctuation *>(tokens[i])->type
                   != Punctuation::Type::CloseBrace) {
        tokens[i]->error("Expected '}'");
    }
    i++;

    for (Statement *s : block->statements) {
        s->show();
    }
    Function *canyonMain = new Function;
    canyonMain->body = block;
    canyonMain->type = type;
    ast->functions[name] = canyonMain;
}

static CodeBlock *parseBlock(std::vector<Token *> tokens, size_t &i, AST::AST *ast) {
    CodeBlock *block = new CodeBlock(ast);
    while (typeid(*tokens[i]) != typeid(Punctuation)
           || static_cast<Punctuation *>(tokens[i])->type
                    != Punctuation::Type::CloseBrace) {
        Statement *s = parseStatement(tokens, i, block);
        if (s != nullptr) {
            block->statements.push_back(s);
        }
    }
    return block;
}

static Statement *parseStatement(std::vector<Token *> tokens, size_t &i,
      CodeBlock *context) {
    while (typeid(*tokens[i]) == typeid(Punctuation)
           && static_cast<Punctuation *>(tokens[i])->type
                    == Punctuation::Type::Semicolon) {
        i++;
    }
    if (typeid(*tokens[i]) == typeid(Punctuation)
          && static_cast<Punctuation *>(tokens[i])->type
                   == Punctuation::Type::CloseBrace) {
        return nullptr;
    }

    rvalue *rval;
    if (typeid(*tokens[i]) == typeid(Primitive)) {
        Primitive *type = static_cast<Primitive *>(tokens[i]);
        i++;
        if (typeid(*tokens[i]) == typeid(Identifier)) {
            Identifier *id = static_cast<Identifier *>(tokens[i]);
            if (context->locals->find(id) != context->locals->end()) {
                tokens[i]->error("Re-declaration of variable %.*s", id->s.len,
                      id->s.start);
            }
            context->locals->insert({id, type});
            size_t i2 = i;
            rval = parseRvalue(tokens, i, context);
            if (rval == nullptr) {
                fprintf(stderr, "HELP\n");
                exit(EXIT_FAILURE);
            }
            if (typeid(*rval) != typeid(Assignment)) {
                i = i2 + 1;
                return nullptr;
            }
        } else {
            tokens[i]->error("Unexpected token following primitive");
        }
    } else if (typeid(*tokens[i]) == typeid(Keyword)
               && static_cast<Keyword *>(tokens[i])->type == Keyword::Type::RETURN) {
        i++;
        rval = parseRvalue(tokens, i, context);
        if (typeid(*tokens[i]) != typeid(Punctuation)
              || static_cast<Punctuation *>(tokens[i])->type
                       != Punctuation::Type::Semicolon) {
            tokens[i]->error("Expected ';' after statement");
        }
        return new Return(rval);
    } else {
        rval = parseRvalue(tokens, i, context);
    }

    if (typeid(*tokens[i]) != typeid(Punctuation)
          || static_cast<Punctuation *>(tokens[i])->type
                   != Punctuation::Type::Semicolon) {
        tokens[i]->error("Expected ';' after statement");
    }
    if (rval != nullptr) {
        return new Expression(rval);
    }
    return nullptr;
}

static inline bool isSep(char *c) {
    // If c is any of these characters, it is by default a separator
    if (c[0] == ' ' || c[0] == '\t' || c[0] == '\n' || c[0] == '(' || c[0] == ')'
          || c[0] == ';' || c[0] == '{' || c[0] == '}' || c[0] == '+' || c[0] == '-'
          || c[0] == '*' || c[0] == '/' || c[0] == '%' || c[0] == '=') {
        return true;
    }

    // If c is alnum, it is sep as long as prev is not alnum
    if (isalnum(c[0]) && !isalnum(c[-1])) {
        return true;
    }

    return false;
}
