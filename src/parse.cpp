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
 * @param it a reference to the iterator of Tokens to use. Expected to point to the canyon
 * function type when this function is called. By the end of the function it will point to
 * the token IMMEDIATELY AFTER the closing curly brace
 * @param ast the AST to place the parsed function inside of
 */
static void parseFunction(std::vector<Token *>::iterator &it, AST::AST *ast);

/**
 * @brief Converts tokens to the AST representation of a function parameter list
 *
 * @param it a reference to the iterator of Tokens to use. Expected to point to the
 * opening parenthesis when this function is called. When this function returns, it will
 * point to the token IMMEDIATELY AFTER the closing parenthesis
 * @param function the Function to determine its parameters
 */
static void parseParameters(std::vector<Token *>::iterator &it, Function *function);

/**
 * @brief Parses tokens into a code block
 *
 * @param it a reference to the iterator of Tokens to use. Expected to point to the token
 * IMMEDIATELY AFTER the opening curly brace when this function is called. When this
 * function returns it will point to the token IMMEDIATELY AFTER the closing curly brace
 * @param context the "fresh" CodeBlock to fill its contents with
 */
static void parseBlock(std::vector<Token *>::iterator &it, CodeBlock *context);
/**
 * @brief Parses tokens into a statement
 *
 * @param it a reference to the iterator of Tokens to use. Expected to point to the first
 * token of the statement when this function is called. When this function returns it will
 * point to the token IMMEDIIATELY AFTER the statement
 * @param context the most specific context that the statement is a part of
 * @return the parsed Statement
 */
static Statement *parseStatement(std::vector<Token *>::iterator &it, CodeBlock *context);

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

    AST::AST *ast = new AST::AST();
    parseFunctions(tokens, ast);
    if (ast->functions.find("canyonMain") == ast->functions.end()) {
        fprintf(stderr, "Parse error: no main function\n");
        exit(EXIT_FAILURE);
    }
    ast->resolve();
    return ast;
}

static void parseFunctions(std::vector<Token *> tokens, AST::AST *ast) {
    std::vector<Token *>::iterator it = tokens.begin();
    while (it < tokens.end()) {
        parseFunction(it, ast);
    }
}

static void parseFunction(std::vector<Token *>::iterator &it, AST::AST *ast) {
    Type type;
    if (typeid(**it) == typeid(Primitive)) {
        type = static_cast<Primitive *>(*it)->type;
    } else if (typeid(**it) == typeid(Keyword)
               && static_cast<Keyword *>(*it)->type == Keyword::Type::VOID) {
        type = Type::VOID;
    } else {
        (*it)->error("Expected function type");
    }
    it++;
    if (typeid(**it) != typeid(Identifier)) {
        (*it)->error("Expected identifier");
    }
    std::string name = static_cast<Identifier *>(*it)->s;
    if (name == "main") {
        name = "canyonMain";
    }
    it++;
    Function *function = new Function(ast);
    parseParameters(it, function);
    if (typeid(**it) != typeid(Punctuation)
          && static_cast<Punctuation *>(*it)->type != Punctuation::Type::OpenBrace) {
        (*it)->error("Expected '{'");
    }
    it++;

    parseBlock(it, function->body);

    if (typeid(**it) != typeid(Punctuation)
          && static_cast<Punctuation *>(*it)->type != Punctuation::Type::CloseBrace) {
        (*it)->error("Expected '}'");
    }
    it++;

    for (Statement *s : function->body->statements) {
        s->show();
    }
    function->type = type;
    ast->functions[name] = function;
}

static void parseParameters(std::vector<Token *>::iterator &it, Function *function) {
    if (typeid(**it) != typeid(Punctuation)
          && static_cast<Punctuation *>(*it)->type != Punctuation::Type::OpenParen) {
        (*it)->error("Expected '('");
    }
    it++;

    CodeBlock *context = function->body;
    while (true) {
        if (typeid(**it) == typeid(Primitive)) {
            Primitive *type = static_cast<Primitive *>(*it);
            it++;
            if (typeid(**it) == typeid(Identifier)) {
                Identifier *id = static_cast<Identifier *>(*it);
                it++;
                if (context->locals->find(id) != context->locals->end()) {
                    (*it)->error("Re-declaration of parameter %.*s", id->s.len,
                          id->s.start);
                }
                context->locals->insert({
                      id, {type->type, true}
                });
                function->parameters.push_back({id, type->type});
            } else {
                (*it)->error("Unexpected token following primitive");
            }
        }
        if (typeid(**it) == typeid(Punctuation)) {
            if (static_cast<Punctuation *>(*it)->type == Punctuation::Type::CloseParen) {
                it++;
                return;
            } else if (static_cast<Punctuation *>(*it)->type
                       == Punctuation::Type::Comma) {
                it++;
                continue;
            }
        }
        (*it)->error("Unexpected token; expected ')' or ','");
    }

    if (typeid(**it) != typeid(Punctuation)
          && static_cast<Punctuation *>(*it)->type != Punctuation::Type::CloseParen) {
        (*it)->error("Expected ')'");
    }
    it++;
}

static void parseBlock(std::vector<Token *>::iterator &it, CodeBlock *context) {
    while (typeid(**it) != typeid(Punctuation)
           || static_cast<Punctuation *>(*it)->type != Punctuation::Type::CloseBrace) {
        Statement *s = parseStatement(it, context);
        if (s != nullptr) {
            context->statements.push_back(s);
        }
    }
}

static Statement *parseStatement(std::vector<Token *>::iterator &it, CodeBlock *context) {
    while (typeid(**it) == typeid(Punctuation)
           && static_cast<Punctuation *>(*it)->type == Punctuation::Type::Semicolon) {
        it++;
    }
    if (typeid(**it) == typeid(Punctuation)
          && static_cast<Punctuation *>(*it)->type == Punctuation::Type::CloseBrace) {
        return nullptr;
    }

    rvalue *rval;
    if (typeid(**it) == typeid(Primitive)) {
        Primitive *type = static_cast<Primitive *>(*it);
        it++;
        if (typeid(**it) == typeid(Identifier)) {
            Identifier *id = static_cast<Identifier *>(*it);
            if (context->locals->find(id) != context->locals->end()) {
                (*it)->error("Re-declaration of variable %.*s", id->s.len, id->s.start);
            }
            context->locals->insert({
                  id, {type->type, false}
            });
            std::vector<Token *>::iterator it2 = it;
            rval = parseRvalue(it, context);
            if (rval == nullptr) {
                fprintf(stderr, "HELP\n");
                exit(EXIT_FAILURE);
            }
            if (typeid(*rval) != typeid(Assignment)) {
                it = it2 + 1;
                return nullptr;
            }
        } else {
            (*it)->error("Unexpected token following primitive");
        }
    } else if (typeid(**it) == typeid(Keyword)
               && static_cast<Keyword *>(*it)->type == Keyword::Type::RETURN) {
        std::vector<Token *>::iterator return_it = it;
        it++;
        rval = parseRvalue(it, context);
        if (typeid(**it) != typeid(Punctuation)
              || static_cast<Punctuation *>(*it)->type != Punctuation::Type::Semicolon) {
            if (static_cast<Punctuation *>(*it)->type == Punctuation::Type::Equals) {
                (*it)->error("LHS of assignment is not a variable");
            }
            (*it)->error("Expected ';' after statement");
        }
        Return *ret = new Return(rval, *return_it);
        return ret;
    } else {
        rval = parseRvalue(it, context);
    }

    if (typeid(**it) != typeid(Punctuation)
          || static_cast<Punctuation *>(*it)->type != Punctuation::Type::Semicolon) {
        if (static_cast<Punctuation *>(*it)->type == Punctuation::Type::Equals) {
            (*it)->error("LHS of assignment is not a variable");
        }
        (*it)->error("Expected ';' after statement");
    }
    if (rval != nullptr) {
        return new Expression(rval);
    }
    return nullptr;
}

static inline bool isSep(char *c) {
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
