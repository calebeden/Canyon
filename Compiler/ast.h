#ifndef AST_H
#define AST_H

#include "tokens.h"
#include <stdint-gcc.h>

#include <vector>

struct Expression {
    virtual void show() = 0;
    virtual void compile(FILE *outfile) = 0;
};

struct Literal : public Expression {
    uint64_t value;
    Literal(Identifier &value);
    virtual void show();
    virtual void compile(FILE *outfile);
};

struct Variable : public Expression {
    Slice s;
    Variable(Identifier &value);
    virtual void show();
    virtual void compile(FILE *outfile);
};

struct Statement {
    virtual void show() = 0;
    virtual void compile(FILE *outfile) = 0;
};

struct Declaration : public Statement {
    Primitive *type;
    Variable *variable;
    Declaration(Primitive *type, Variable *variable);
    virtual void show();
    virtual void compile(FILE *outfile);
};

struct Assignment : public Statement {
    Identifier *variable;
    Expression *expression;
    Assignment(Identifier *variable, Expression *expression);
    virtual void show();
    virtual void compile(FILE *outfile);
};

struct Print : public Statement {
    Expression *expression;
    Print(Expression *expression);
    virtual void show();
    virtual void compile(FILE *outfile);
};

struct Return : public Statement {
    virtual void show();
    virtual void compile(FILE *outfile);
};

// TODO: eventually have arbitrary code blocks for precise scoping but for now just
// functions, loops, etc
struct CodeBlock {
    std::vector<Statement *> statements;
    void compile(FILE *outfile);
};

struct Function {
    CodeBlock body;
};

#endif
