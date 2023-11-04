#ifndef TOKENS_H
#define TOKENS_H

#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <stddef.h>
#include <stdexcept>
#include <stdlib.h>
#include <string>
#include <typeinfo>
#include <vector>

struct Slice {
    char *start;
    size_t len;
    /**
     * @brief Construct a new Slice object based on a pointer to its start and its length
     *
     * @param start a pointer to the first character in the Slice
     * @param len the number of characters in the Slice
     */
    Slice(char *start, int len);
    /**
     * @brief Construct a new Slice object based on pointers to its start and end
     *
     * @param start a pointer to the first character in the Slice
     * @param end a pointer to the final character of the Slice (inclusive)
     */
    Slice(char *start, char *end);
    void show();
    /**
     * @brief Compares a Slice to a string
     *
     * @param rhs the std::string to compare to
     * @return whether the Slice and string contain the same characters and are the same
     * length
     */
    bool operator==(const std::string &rhs);

    operator std::string();
};

struct Token {
    static Token *createToken(Slice s);

    virtual void show() {
        fprintf(stderr, "Class: %s\n", typeid(*this).name());
    }
};

struct Keyword : public Token {
    enum class Type {
        VOID,
        RETURN
    };
    Type type;
    Keyword(Slice s);
    static bool isKeyword(Slice s);
};

struct Primitive : public Token {
    enum class Type {
        INT,
        BYTE,
        SHORT,
        LONG,
        FLOAT,
        DOUBLE,
        BOOL,
        CHAR,
    };
    Type type;
    Primitive(Slice s);
    virtual void show();
    void compile(FILE *outfile);

    static bool isPrimitive(Slice s);
};

struct Operator : public Token {
    enum class Type {
    };
    Type type;
    Operator(Slice s);

    static bool isOperator(Slice s);
};

struct Punctuation : public Token {
    enum class Type {
        OpenParen,
        CloseParen,
        Semicolon,
        OpenBrace,
        CloseBrace,
        Equals
    };
    Type type;
    Punctuation(Slice s);

    static bool isPunctuation(Slice s);
};

struct Identifier : public Token {
    Slice s;

    Identifier(Slice s) : s(s) {
    }

    virtual void show();
};

#endif
