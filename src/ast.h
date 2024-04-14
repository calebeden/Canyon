#ifndef AST_H
#define AST_H

#include "errorhandler.h"
#include "tokens.h"
#include <string_view>
#include <unordered_map>

#include <cstdint>
#include <iostream>
#include <memory>
#include <vector>

namespace AST {

class CodeBlock;

struct rvalue {
	std::filesystem::path source;
	size_t row;
	size_t col;
	virtual void print(std::ostream &os) const = 0;
	virtual void compile(std::ostream &outfile) const = 0;
	virtual Type typeCheck(const CodeBlock &context, ErrorHandler &errors) const = 0;
	virtual ~rvalue() = default;
protected:
	rvalue(std::filesystem::path source, size_t row, size_t col);
};

struct Literal : public rvalue {
	int32_t value;
	explicit Literal(const Identifier *value);
	virtual void print(std::ostream &os) const;
	virtual void compile(std::ostream &outfile) const;
	virtual Type typeCheck(const CodeBlock &context, ErrorHandler &errors) const;
	virtual ~Literal() = default;
};

struct Statement {
	virtual void print(std::ostream &os) const = 0;
	virtual void compile(std::ostream &outfile) const = 0;
	virtual Type typeCheck(const CodeBlock &context, Type returnType,
	      ErrorHandler &errors) const
	      = 0;
	virtual ~Statement() = default;
};

struct Variable : public rvalue {
	Identifier *variable;
	explicit Variable(Identifier *variable);
	Type type = Type::UNKNOWN;
	virtual void print(std::ostream &os) const;
	virtual void compile(std::ostream &outfile) const;
	virtual Type typeCheck(const CodeBlock &context, ErrorHandler &errors) const;
	virtual ~Variable() = default; // TODO
};

struct Assignment : public rvalue {
	Identifier *variable;
	std::unique_ptr<rvalue> expression;
	Assignment(Identifier *variable, std::unique_ptr<rvalue> expression);
	virtual void print(std::ostream &os) const;
	virtual void compile(std::ostream &outfile) const;
	virtual Type typeCheck(const CodeBlock &context, ErrorHandler &errors) const;
	virtual ~Assignment() = default; // TODO
};

struct Addition : public rvalue {
	std::unique_ptr<rvalue> operand1;
	std::unique_ptr<rvalue> operand2;
	Addition(std::unique_ptr<rvalue> operand1, std::unique_ptr<rvalue> operand2);
	virtual void print(std::ostream &os) const;
	virtual void compile(std::ostream &outfile) const;
	virtual Type typeCheck(const CodeBlock &context, ErrorHandler &errors) const;
	virtual ~Addition() = default;
};

struct Subtraction : public rvalue {
	std::unique_ptr<rvalue> operand1;
	std::unique_ptr<rvalue> operand2;
	Subtraction(std::unique_ptr<rvalue> operand1, std::unique_ptr<rvalue> operand2);
	virtual void print(std::ostream &os) const;
	virtual void compile(std::ostream &outfile) const;
	virtual Type typeCheck(const CodeBlock &context, ErrorHandler &errors) const;
	virtual ~Subtraction() = default;
};

struct Multiplication : public rvalue {
	std::unique_ptr<rvalue> operand1;
	std::unique_ptr<rvalue> operand2;
	Multiplication(std::unique_ptr<rvalue> operand1, std::unique_ptr<rvalue> operand2);
	virtual void print(std::ostream &os) const;
	virtual void compile(std::ostream &outfile) const;
	virtual Type typeCheck(const CodeBlock &context, ErrorHandler &errors) const;
	virtual ~Multiplication() = default;
};

struct Division : public rvalue {
	std::unique_ptr<rvalue> operand1;
	std::unique_ptr<rvalue> operand2;
	Division(std::unique_ptr<rvalue> operand1, std::unique_ptr<rvalue> operand2);
	virtual void print(std::ostream &os) const;
	virtual void compile(std::ostream &outfile) const;
	virtual Type typeCheck(const CodeBlock &context, ErrorHandler &errors) const;
	virtual ~Division() = default;
};

struct Modulo : public rvalue {
	std::unique_ptr<rvalue> operand1;
	std::unique_ptr<rvalue> operand2;
	Modulo(std::unique_ptr<rvalue> operand1, std::unique_ptr<rvalue> operand2);
	virtual void print(std::ostream &os) const;
	virtual void compile(std::ostream &outfile) const;
	virtual Type typeCheck(const CodeBlock &context, ErrorHandler &errors) const;
	virtual ~Modulo() = default;
};

struct Expression : public Statement {
	std::unique_ptr<rvalue> rval;
	explicit Expression(std::unique_ptr<rvalue> rval);
	virtual void print(std::ostream &os) const;
	virtual void compile(std::ostream &outfile) const;
	virtual Type typeCheck(const CodeBlock &context, Type returnType,
	      ErrorHandler &errors) const;
	virtual ~Expression() = default;
};

struct FunctionCall : public rvalue {
	std::unique_ptr<Variable> name;
	std::vector<std::unique_ptr<rvalue>> arguments;
	explicit FunctionCall(std::unique_ptr<Variable> name);
	virtual void print(std::ostream &os) const;
	virtual void compile(std::ostream &outfile) const;
	virtual Type typeCheck(const CodeBlock &context, ErrorHandler &errors) const;
	virtual ~FunctionCall() = default; // TODO
};

struct Return : public Statement {
	std::unique_ptr<rvalue> rval;
	std::filesystem::path source;
	size_t row;
	size_t col;
	Return(std::unique_ptr<rvalue> rval, Token *token);
	virtual void print(std::ostream &os) const;
	virtual void compile(std::ostream &outfile) const;
	virtual Type typeCheck(const CodeBlock &context, Type returnType,
	      ErrorHandler &errors) const;
	virtual ~Return() = default;
};

// TODO: eventually have arbitrary code blocks for precise scoping but for now just
// functions, loops, etc
struct CodeBlock {
	enum IdentifierStatus {
		VARIABLE,
		FUNCTION,
		UNKNOWN,
	};

	std::vector<std::unique_ptr<Statement>> statements;
	// For each local, the info tuple says the type and whether it is a parameter
	std::unordered_map<Identifier *, std::tuple<Type, bool>, Hasher, Comparator> locals;
	std::vector<Variable *> deferred;
	CodeBlock *parent = nullptr;
	struct Module *global;
	explicit CodeBlock(Module *global);
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
	~CodeBlock() = default; // TODO
};

struct Function {
	CodeBlock body;
	Type type = Type::UNKNOWN;
	std::vector<std::pair<Identifier *, Type>> parameters;
	explicit Function(Module *module);
	virtual void compile(std::ostream &outfile, std::string_view name) const;
	virtual void forward(std::ostream &outfile, std::string_view name) const;
	virtual void resolve(ErrorHandler &errors);
	void typeCheck(ErrorHandler &errors) const;
	virtual ~Function() = default; // TODO
};

struct Module {
	Module();
	std::unordered_map<std::string_view, Function *> functions;
	std::vector<FunctionCall *> functionCalls;
	void compile(std::ostream &outfile) const;
	// TODO global vars
	/**
	 * @brief Resolves all deferred identifiers from all CodeBlocks in the Module by
	 * looking at functions found in the module's global scope. Includes resolving
	 * functionCalls to check whether they call a function that exists with the correct
	 * arguments. Also performs type checking on rvalues
	 *
	 */
	void resolve(ErrorHandler &errors);
	~Module() = default;
};

} // namespace AST

#endif
