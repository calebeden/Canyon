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
    size_t row;
    size_t col;
    char *source;
    /**
     * @brief Construct a new Slice object based on pointers to its start and end
     *
     * @param start a pointer to the first character in the Slice
     * @param end a pointer to the final character of the Slice (inclusive)
     */
    Slice(char *start, char *end, char *source, size_t row, size_t col);
    void show();
    /**
     * @brief Compares a Slice to a string
     *
     * @param rhs the std::string to compare to
     * @return whether the Slice and string contain the same characters and are the same
     * length
     */
    bool operator==(const std::string &rhs);

    bool operator==(const Slice &other);

    operator std::string();
};

struct Token {
    char *source;
    size_t row;
    size_t col;
    static Token *createToken(Slice s);

    virtual void show();
    virtual void parse_error(const char *const error, ...);
protected:
    Token(char *source, size_t row, size_t col);
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

struct Punctuation : public Token {
    enum class Type {
        OpenParen,
        CloseParen,
        Semicolon,
        OpenBrace,
        CloseBrace,
        Equals,
        Plus,
        Minus,
        Times,
        Divide,
        Mod
    };
    Type type;
    Punctuation(Slice s);

    static bool isPunctuation(Slice s);
};

struct Identifier : public Token {
    Slice s;

    Identifier(Slice s);

    virtual void show();
    void compile(FILE *outfile);
    bool operator==(const Identifier &other);
};

struct Hasher {
    std::size_t operator()(Identifier *const id) const {
        // djb2
        size_t hash = 5381;
        for (size_t i = 0; i < id->s.len; i++) {
            hash = hash * 33 + id->s.start[i];
        }
        return hash;
    }
};

struct Comparator {
    bool operator()(Identifier *a, Identifier *b) const {
        return a->s == b->s;
    }
};

#endif
