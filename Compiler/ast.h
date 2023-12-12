#ifndef AST_H
#define AST_H

#include "tokens.h"

#include <cstdint>
#include <vector>
#include <unordered_map>

struct rvalue {
    virtual void show() = 0;
    virtual void compile(FILE *outfile) = 0;
};

struct Literal : public rvalue {
    uint64_t value;
    Literal(Identifier &value);
    virtual void show();
    virtual void compile(FILE *outfile);
};

struct Statement {
    virtual void show() = 0;
    virtual void compile(FILE *outfile) = 0;
};

struct Assignment : public rvalue {
    Identifier *variable;
    rvalue *expression;
    Assignment(Identifier *variable, rvalue *expression);
    virtual void show();
    virtual void compile(FILE *outfile);
};

struct Expression : public Statement {
    rvalue *rval;
    Expression(rvalue *rval);
    virtual void show();
    virtual void compile(FILE *outfile);
};

struct Print : public rvalue {
    rvalue *expression;
    Print(rvalue *expression);
    virtual void show();
    virtual void compile(FILE *outfile);
};

struct PrintVar : public rvalue {
    Identifier *variable;
    PrintVar(Identifier *variable);
    virtual void show();
    virtual void compile(FILE *outfile);
};

struct Return : public rvalue {
    virtual void show();
    virtual void compile(FILE *outfile);
};

// TODO: eventually have arbitrary code blocks for precise scoping but for now just
// functions, loops, etc
struct CodeBlock {
    std::vector<Statement *> statements;
    std::unordered_map<Identifier *, Primitive *> *locals;
    CodeBlock();
    void compile(FILE *outfile);
};

struct Function {
    CodeBlock *body;
    void compile(FILE *outfile);
};

struct AST {
    std::vector<Function *> functions;
    void compile(FILE *outfile);
};

#endif
