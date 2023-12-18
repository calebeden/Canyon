#ifndef AST_H
#define AST_H

#include "tokens.h"
#include <unordered_map>

#include <cstdint>
#include <vector>

namespace AST {

struct rvalue {
    char *source;
    size_t row;
    size_t col;
    virtual void show() = 0;
    virtual void compile(FILE *outfile) = 0;
    void error(const char *const error, ...);
protected:
    rvalue(char *source, size_t row, size_t col);
};

struct Literal : public rvalue {
    uint64_t value;
    Literal(Identifier *value);
    virtual void show();
    virtual void compile(FILE *outfile);
};

struct Statement {
    virtual void show() = 0;
    virtual void compile(FILE *outfile) = 0;
};

struct Variable : public rvalue {
    Identifier *variable;
    Variable(Identifier *variable);
    virtual void show();
    virtual void compile(FILE *outfile);
};

struct Assignment : public rvalue {
    Identifier *variable;
    rvalue *expression;
    Assignment(Identifier *variable, rvalue *expression);
    virtual void show();
    virtual void compile(FILE *outfile);
};

struct Addition : public rvalue {
    rvalue *operand1;
    rvalue *operand2;
    Addition(rvalue *operand1, rvalue *operand2);
    virtual void show();
    virtual void compile(FILE *outfile);
};

struct Subtraction : public rvalue {
    rvalue *operand1;
    rvalue *operand2;
    Subtraction(rvalue *operand1, rvalue *operand2);
    virtual void show();
    virtual void compile(FILE *outfile);
};

struct Multiplication : public rvalue {
    rvalue *operand1;
    rvalue *operand2;
    Multiplication(rvalue *operand1, rvalue *operand2);
    virtual void show();
    virtual void compile(FILE *outfile);
};

struct Division : public rvalue {
    rvalue *operand1;
    rvalue *operand2;
    Division(rvalue *operand1, rvalue *operand2);
    virtual void show();
    virtual void compile(FILE *outfile);
};

struct Modulo : public rvalue {
    rvalue *operand1;
    rvalue *operand2;
    Modulo(rvalue *operand1, rvalue *operand2);
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

struct FunctionCall : public rvalue {
    Variable *name;
    std::vector<rvalue *> arguments;
    FunctionCall(Variable *name);
    virtual void show();
    virtual void compile(FILE *outfile);
};

struct Return : public Statement {
    rvalue *rval;
    Return(rvalue *rval);
    virtual void show();
    virtual void compile(FILE *outfile);
};

// TODO: eventually have arbitrary code blocks for precise scoping but for now just
// functions, loops, etc
struct CodeBlock {
    enum IdentifierStatus {
        VARIABLE,
        FUNCTION,
        UNKNOWN,
    };

    std::vector<Statement *> statements;
    // For each local, the info tuple says the type and whether it is a parameter
    std::unordered_map<Identifier *, std::tuple<Primitive *, bool>, Hasher, Comparator>
          *locals;
    std::vector<Variable *> deferred;
    CodeBlock *parent = nullptr;
    struct AST *global;
    CodeBlock(AST *global);
    void compile(FILE *outfile);
    /**
     * @brief Defers an identifier to be resolved within the global scope at the end of
     * the module
     *
     * @param rval the identifier to defer
     */
    void defer(Variable *rval);
    IdentifierStatus find(Variable *id);
    // TODO global vars
    /**
     * @brief Resolves deferred identifiers by looking at functions found in the module's
     * global scope
     *
     */
    void resolve();
};

struct Function {
    CodeBlock *body;
    Primitive::Type type;
    std::vector<std::pair<Identifier *, Primitive *>> parameters;
    Function(AST *ast);
    void compile(FILE *outfile, std::string name);
    void forward(FILE *outfile, std::string name);
};

struct AST {
    std::unordered_map<std::string, Function *> functions;
    std::vector<FunctionCall *> functionCalls;
    void compile(FILE *outfile);
    // TODO global vars
    /**
     * @brief Resolves all deferred identifiers from all CodeBlocks in the AST by looking
     * at functions found in the module's global scope. Also resolves functionCalls to
     * check whether they call a function that exists with the correct arguments
     *
     */
    void resolve();
};

} // namespace AST

#endif
