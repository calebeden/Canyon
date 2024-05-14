#ifndef AST_H
#define AST_H

#include "tokens.h"

#include <functional>
#include <memory>
#include <ranges>
#include <vector>

class ASTVisitor;

class Expression {
public:
	virtual void accept(ASTVisitor &visitor) = 0;
	virtual ~Expression() = default;
};

class Statement {
public:
	virtual void accept(ASTVisitor &visitor) = 0;
	virtual ~Statement() = default;
};

class FunctionCallExpression : public Expression {
	std::unique_ptr<Expression> function;
public:
	FunctionCallExpression(std::unique_ptr<Expression> function);
	Expression &getFunction();
	void accept(ASTVisitor &visitor) override;
	virtual ~FunctionCallExpression() = default;
};

class BinaryExpression : public Expression {
private:
	std::unique_ptr<Operator> op;
	std::unique_ptr<Expression> left;
	std::unique_ptr<Expression> right;
public:
	BinaryExpression(std::unique_ptr<Operator> op, std::unique_ptr<Expression> left,
	      std::unique_ptr<Expression> right);
	Expression &getLeft();
	Expression &getRight();
	Operator &getOperator();
	void accept(ASTVisitor &visitor) override;
	virtual ~BinaryExpression() = default;
	friend class ASTVisitor;
};

class UnaryExpression : public Expression {
private:
	std::unique_ptr<Operator> op;
	std::unique_ptr<Expression> operand;
public:
	UnaryExpression(std::unique_ptr<Operator> op, std::unique_ptr<Expression> operand);
	void accept(ASTVisitor &visitor) override;
	Expression &getExpression();
	Operator &getOperator();
	virtual ~UnaryExpression() = default;
};

class LiteralExpression : public Expression {
private:
	std::unique_ptr<IntegerLiteral> literal;
public:
	LiteralExpression(std::unique_ptr<IntegerLiteral> literal);
	IntegerLiteral &getLiteral();
	void accept(ASTVisitor &visitor) override;
	virtual ~LiteralExpression() = default;
};

class SymbolExpression : public Expression {
private:
	std::unique_ptr<Symbol> symbol;
public:
	SymbolExpression(std::unique_ptr<Symbol> symbol);
	Symbol &getSymbol();
	void accept(ASTVisitor &visitor) override;
	virtual ~SymbolExpression() = default;
};

class BlockExpression : public Expression {
private:
	std::vector<std::unique_ptr<Statement>> statements;
	std::unique_ptr<Expression> finalExpression;
public:
	BlockExpression() = default;
	void pushStatement(std::unique_ptr<Statement> statement);
	void setFinalExpression(std::unique_ptr<Expression> finalExpression);
	void forEachStatement(const std::function<void(Statement &)> &statementHandler);
	Expression *getFinalExpression();
	void accept(ASTVisitor &visitor) override;
	virtual ~BlockExpression() = default;
};

class ReturnExpression : public Expression {
	std::unique_ptr<Expression> expression;
public:
	ReturnExpression(std::unique_ptr<Expression> expression);
	Expression *getExpression();
	void accept(ASTVisitor &visitor) override;
	virtual ~ReturnExpression() = default;
};

class ParenthesizedExpression : public Expression {
private:
	std::unique_ptr<Expression> expression;
public:
	ParenthesizedExpression(std::unique_ptr<Expression> expression);
	Expression &getExpression();
	void accept(ASTVisitor &visitor) override;
	virtual ~ParenthesizedExpression() = default;
};

class ExpressionStatement : public Statement {
private:
	std::unique_ptr<Expression> expression;
public:
	ExpressionStatement(std::unique_ptr<Expression> expression);
	Expression &getExpression();
	void accept(ASTVisitor &visitor) override;
	virtual ~ExpressionStatement() = default;
};

class LetStatement : public Statement {
private:
	std::unique_ptr<Symbol> symbol;
	std::unique_ptr<Symbol> type;
	std::unique_ptr<Expression> expression;
public:
	LetStatement(std::unique_ptr<Symbol> symbol, std::unique_ptr<Symbol> type,
	      std::unique_ptr<Expression> expression);
	Symbol &getSymbol();
	Expression &getExpression();
	Symbol &getType();
	void accept(ASTVisitor &visitor) override;
	virtual ~LetStatement() = default;
};

class Function {
private:
	std::unique_ptr<Symbol> name;
	std::unique_ptr<Symbol> returnType;
	std::unique_ptr<BlockExpression> body;
public:
	Function(std::unique_ptr<Symbol> name, std::unique_ptr<Symbol> returnType,
	      std::unique_ptr<BlockExpression> body);
	Symbol &getName();
	Symbol &getReturnType();
	BlockExpression &getBody();
	void accept(ASTVisitor &visitor);
	~Function() = default;
};

class Module {
private:
	std::vector<std::unique_ptr<Function>> functions;
public:
	Module() = default;
	void pushFunction(std::unique_ptr<Function> function);
	void forEachFunction(const std::function<void(Function &)> &functionHandler);
	~Module() = default;
};

class ASTVisitor {
public:
	virtual void visit(FunctionCallExpression &node) = 0;
	virtual void visit(BinaryExpression &node) = 0;
	virtual void visit(UnaryExpression &node) = 0;
	virtual void visit(LiteralExpression &node) = 0;
	virtual void visit(SymbolExpression &node) = 0;
	virtual void visit(BlockExpression &node) = 0;
	virtual void visit(ReturnExpression &node) = 0;
	virtual void visit(ParenthesizedExpression &node) = 0;
	virtual void visit(ExpressionStatement &node) = 0;
	virtual void visit(LetStatement &node) = 0;
	virtual void visit(Function &node) = 0;
	virtual void visit(Module &node) = 0;
	virtual ~ASTVisitor() = default;
};

class ASTPrinter : public ASTVisitor {
	int tabLevel = 0;
public:
	void visit(FunctionCallExpression &node) override;
	void visit(BinaryExpression &node) override;
	void visit(UnaryExpression &node) override;
	void visit(LiteralExpression &node) override;
	void visit(SymbolExpression &node) override;
	void visit(BlockExpression &node) override;
	void visit(ReturnExpression &node) override;
	void visit(ParenthesizedExpression &node) override;
	void visit(ExpressionStatement &node) override;
	void visit(LetStatement &node) override;
	void visit(Function &node) override;
	void visit(Module &node) override;
	virtual ~ASTPrinter() = default;
};

#endif
