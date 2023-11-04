#include "parse.h"

#include "ast.h"
#include "tokens.h"

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
static CodeBlock *parseMain(std::vector<Slice> slices);

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
 * @return the parsed Statement
 */
static Statement *parseStatement(std::vector<Token *> tokens, size_t &i);

CodeBlock *tokenize(char *program, off_t size) {
    std::vector<Slice> slices;
    start = current = program;

    // Tokenize the input source file into Slices, skipping over whitespace
    while (current - start < size) {
        while (*current == ' ' || *current == '\t' || *current == '\n') {
            current++;
        }
        if (current - start >= size) {
            break;
        }
        char *tokenStart = current;
        do {
            current++;
        } while (!isSep(*current) && current - start < size);
        slices.emplace_back(tokenStart, current - 1);
    }

    if (slices.size() == 0) {
        fprintf(stderr, "File does not contain any source code");
        exit(1);
    }

    return parseMain(slices);
}

static CodeBlock *parseMain(std::vector<Slice> slices) {
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
    return block;
}

static CodeBlock *parseBlock(std::vector<Token *> tokens, size_t &i) {
    CodeBlock *block = new CodeBlock();
    while (dynamic_cast<Punctuation *>(tokens[i]) == nullptr
           || static_cast<Punctuation *>(tokens[i])->type
                    != Punctuation::Type::CloseBrace) {
        block->statements.emplace_back(parseStatement(tokens, i));
        i++;
    }
    return block;
}

static Statement *parseStatement(std::vector<Token *> tokens, size_t &i) {
    // while (dynamic_cast<Punctuation *>(tokens[i]) == nullptr
    //        || static_cast<Punctuation *>(tokens[i])->type
    //                 != Punctuation::Type::Semicolon) {
    //     // TODO
    //     i++;
    // }

    if (typeid(*tokens[i]) == typeid(Identifier)) {
        // TODO
        Identifier *id = static_cast<Identifier *>(tokens[i]);
        i++;
        if (id->s == "print") {
            if (typeid(*tokens[i]) == typeid(Identifier)) {
                Identifier *toPrint = static_cast<Identifier *>(tokens[i]);
                i++;
                bool isInt = true;
                for (size_t i = 0; i < toPrint->s.len; i++) {
                    if (!isdigit(toPrint->s.start[i])) {
                        isInt = false;
                        break;
                    }
                }
                if (isInt) {
                    return new Print(new Literal(*toPrint));
                } else {
                    return new Print(new Variable(*toPrint));
                }
            }
        } else {
            bool isInt = true;
            for (size_t i = 0; i < id->s.len; i++) {
                if (!isdigit(id->s.start[i])) {
                    isInt = false;
                    break;
                }
            }
            if (isInt) {
                throw std::invalid_argument("Cannot assign to a number");
            } else if (typeid(*tokens[i]) == typeid(Punctuation)
                       && static_cast<Punctuation *>(tokens[i])->type
                                == Punctuation::Type::Equals) {
                i++;
                if (typeid(*tokens[i]) == typeid(Identifier)) {
                    // Not really identifier but just trying to get something working for
                    // now
                    Literal *l = new Literal(*static_cast<Identifier *>(tokens[i]));
                    i++;
                    return new Assignment(id, l);
                }
            } else {
                throw std::invalid_argument("Expecting an assignment statement");
            }
        }
    } else if (typeid(*tokens[i]) == typeid(Keyword)
               && static_cast<Keyword *>(tokens[i])->type == Keyword::Type::RETURN) {
        i++;
        return new Return();
    } else if (typeid(*tokens[i]) == typeid(Primitive)) {
        Primitive *type = static_cast<Primitive *>(tokens[i]);
        i++;
        if (typeid(*tokens[i]) == typeid(Identifier)) {
            Identifier *id = static_cast<Identifier *>(tokens[i]);
            i++;
            return new Declaration(type, new Variable(*id));
        }
    }
    throw std::invalid_argument("Cannot handle whatever this case is");

    /*
    if (typeid(*tokens[i]) == typeid(Keyword)
          && static_cast<Keyword *>(tokens[i])->type == Keyword::Type::RETURN) {
        i++;
        return new Return();
    }
    if (typeid(*tokens[i]) != typeid(Identifier)) {
        exit(1);
    }
    Identifier *id = static_cast<Identifier *>(tokens[i]);
    i++;
    if (typeid(*tokens[i]) == typeid(Punctuation)
          && static_cast<Punctuation *>(tokens[i])->type == Punctuation::Type::Equals) {
        i++;
        if (typeid(*tokens[i]) == typeid(Identifier)) {
            // Not really identifier but just trying to get something working for now
            Literal *l = new Literal(*static_cast<Identifier *>(tokens[i]));
            i++;
            return new Assignment(id, l);
        }
    } else if (id->s == "print") {
        if (typeid(*tokens[i]) == typeid(Identifier)) {
            // Not really identifier but just trying to get something working for now
            Literal *l = new Literal(*static_cast<Identifier *>(tokens[i]));
            i++;
            return new Print(l);
        }
    } */

    return {};
}

static inline bool isSep(char c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '(' || c == ')' || c == ';'
           || c == '{' || c == '}' || c == '=';
}
