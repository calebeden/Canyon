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

#ifdef DEBUG_TEST_MODE
#  define mockable virtual
#else
#  define mockable
#endif

enum class Type {
    INT,
    BYTE,
    SHORT,
    LONG,
    FLOAT,
    DOUBLE,
    BOOL,
    CHAR,
    VOID,
    UNKNOWN,
};

const char *const typeStr(Type type);

struct Slice {
    const char *start;
    size_t len;
    size_t row;
    size_t col;
    const char *source;
    /**
     * @brief Construct a new Slice object based on pointers to its start and end
     *
     * @param start a pointer to the first character in the Slice
     * @param end a pointer to the final character of the Slice (inclusive)
     */
    Slice(const char *const start, const char *const end, const char *const source,
          size_t row, size_t col);
    Slice(const char *const start, size_t len, const char *const source, size_t row,
          size_t col);
    void show() const;
    bool operator==(const std::string &rhs) const;
    bool operator==(const Slice &other) const;
    bool operator==(const char *const other) const;
    operator std::string() const;
};

struct Token {
    const char *source;
    size_t row;
    size_t col;

    virtual void show() const;
    mockable void error(const char *const error, ...) const __attribute__((noreturn));
protected:
    Token(const char *const source, size_t row, size_t col);
};

struct Keyword : public Token {
    enum class Type {
        VOID,
        RETURN
    };
    Type type;
    Keyword(Slice s, Type type);
};

struct Primitive : public Token {
    Type type;
    Primitive(Slice s, Type type);
    virtual void show();
    void compile(FILE *outfile);

    static void show(Type t);
    static void compile(FILE *outfile, Type t);
};

struct Punctuation : public Token {
    enum class Type {
        OpenParen,
        CloseParen,
        Semicolon,
        OpenBrace,
        CloseBrace,
        Comma,
        Equals,
        Plus,
        Minus,
        Times,
        Divide,
        Mod
    };
    Type type;
    Punctuation(Slice s, Type type);
};

struct Identifier : public Token {
    Slice s;

    Identifier(Slice s);

    virtual void show();
    void compile(FILE *outfile);
};

struct Hasher {
    std::size_t operator()(Identifier *const id) const;
};

struct Comparator {
    bool operator()(Identifier *a, Identifier *b) const;
};

#endif
