#ifndef AST_H
#define AST_H

#include "errorhandler.h"
#include "tokens.h"
#include <string_view>
#include <unordered_map>

#include <cstdint>
#include <iostream>
#include <vector>

namespace AST {

class CodeBlock;

struct rvalue {
	const char *source;
	size_t row;
	size_t col;
	virtual void print(std::ostream &os) const = 0;
	virtual void compile(std::ostream &outfile) const = 0;
	virtual Type typeCheck(const CodeBlock &context, ErrorHandler &errors) const = 0;
protected:
	rvalue(const char *source, size_t row, size_t col);
};

struct Literal : public rvalue {
	int32_t value;
	explicit Literal(const Identifier *value);
	virtual void print(std::ostream &os) const;
	virtual void compile(std::ostream &outfile) const;
	virtual Type typeCheck(const CodeBlock &context, ErrorHandler &errors) const;
};

struct Statement {
	virtual void print(std::ostream &os) const = 0;
	virtual void compile(std::ostream &outfile) const = 0;
	virtual Type typeCheck(const CodeBlock &context, Type returnType,
	      ErrorHandler &errors) const
	      = 0;
};

struct Variable : public rvalue {
	Identifier *variable;
	explicit Variable(Identifier *variable);
	Type type = Type::UNKNOWN;
	virtual void print(std::ostream &os) const;
	virtual void compile(std::ostream &outfile) const;
	virtual Type typeCheck(const CodeBlock &context, ErrorHandler &errors) const;
};

struct Assignment : public rvalue {
	Identifier *variable;
	rvalue *expression;
	Assignment(Identifier *variable, rvalue *expression);
	virtual void print(std::ostream &os) const;
	virtual void compile(std::ostream &outfile) const;
	virtual Type typeCheck(const CodeBlock &context, ErrorHandler &errors) const;
};

struct Addition : public rvalue {
	rvalue *operand1;
	rvalue *operand2;
	Addition(rvalue *operand1, rvalue *operand2);
	virtual void print(std::ostream &os) const;
	virtual void compile(std::ostream &outfile) const;
	virtual Type typeCheck(const CodeBlock &context, ErrorHandler &errors) const;
};

struct Subtraction : public rvalue {
	rvalue *operand1;
	rvalue *operand2;
	Subtraction(rvalue *operand1, rvalue *operand2);
	virtual void print(std::ostream &os) const;
	virtual void compile(std::ostream &outfile) const;
	virtual Type typeCheck(const CodeBlock &context, ErrorHandler &errors) const;
};

struct Multiplication : public rvalue {
	rvalue *operand1;
	rvalue *operand2;
	Multiplication(rvalue *operand1, rvalue *operand2);
	virtual void print(std::ostream &os) const;
	virtual void compile(std::ostream &outfile) const;
	virtual Type typeCheck(const CodeBlock &context, ErrorHandler &errors) const;
};

struct Division : public rvalue {
	rvalue *operand1;
	rvalue *operand2;
	Division(rvalue *operand1, rvalue *operand2);
	virtual void print(std::ostream &os) const;
	virtual void compile(std::ostream &outfile) const;
	virtual Type typeCheck(const CodeBlock &context, ErrorHandler &errors) const;
};

struct Modulo : public rvalue {
	rvalue *operand1;
	rvalue *operand2;
	Modulo(rvalue *operand1, rvalue *operand2);
	virtual void print(std::ostream &os) const;
	virtual void compile(std::ostream &outfile) const;
	virtual Type typeCheck(const CodeBlock &context, ErrorHandler &errors) const;
};

struct Expression : public Statement {
	rvalue *rval;
	explicit Expression(rvalue *rval);
	virtual void print(std::ostream &os) const;
	virtual void compile(std::ostream &outfile) const;
	virtual Type typeCheck(const CodeBlock &context, Type returnType,
	      ErrorHandler &errors) const;
};

struct FunctionCall : public rvalue {
	Variable *name;
	std::vector<rvalue *> arguments;
	explicit FunctionCall(Variable *name);
	virtual void print(std::ostream &os) const;
	virtual void compile(std::ostream &outfile) const;
	virtual Type typeCheck(const CodeBlock &context, ErrorHandler &errors) const;
};

struct Return : public Statement {
	rvalue *rval;
	Token *token;
	Return(rvalue *rval, Token *token);
	virtual void print(std::ostream &os) const;
	virtual void compile(std::ostream &outfile) const;
	virtual Type typeCheck(const CodeBlock &context, Type returnType,
	      ErrorHandler &errors) const;
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
	std::unordered_map<Identifier *, std::tuple<Type, bool>, Hasher, Comparator> locals;
	std::vector<Variable *> deferred;
	CodeBlock *parent = nullptr;
	struct AST *global;
	explicit CodeBlock(AST *global);
	void compile(std::ostream &outfile) const;
	/**
	 * @brief Defers an identifier to be resolved within the global scope at the end of
	 * the module
	 *
	 * @param rval the identifier to defer
	 */
	void defer(Variable *rval);
	IdentifierStatus find(const Variable *id) const;
	// TODO global vars
	/**
	 * @brief Resolves deferred identifiers by looking at functions found in the module's
	 * global scope. Also performs type checking on rvalues
	 *
	 */
	virtual void resolve(ErrorHandler &errors);
	Type getType(Identifier *var) const;
	virtual void typeCheck(Type returnType, ErrorHandler &errors) const;
};

struct Function {
	CodeBlock *body;
	Type type = Type::UNKNOWN;
	std::vector<std::pair<Identifier *, Type>> parameters;
	explicit Function(AST *ast);
	virtual void compile(std::ostream &outfile, std::string_view name) const;
	virtual void forward(std::ostream &outfile, std::string_view name) const;
	virtual void resolve(ErrorHandler &errors);
	void typeCheck(ErrorHandler &errors) const;
};

struct AST {
	AST();
	std::unordered_map<std::string_view, Function *> functions;
	std::vector<FunctionCall *> functionCalls;
	void compile(std::ostream &outfile) const;
	// TODO global vars
	/**
	 * @brief Resolves all deferred identifiers from all CodeBlocks in the AST by looking
	 * at functions found in the module's global scope. Includes resolving functionCalls
	 * to check whether they call a function that exists with the correct arguments. Also
	 * performs type checking on rvalues
	 *
	 */
	void resolve(ErrorHandler &errors);
};

} // namespace AST

#endif
