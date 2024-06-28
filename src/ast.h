#ifndef AST_H
#define AST_H

#include "tokens.h"

#include <functional>
#include <memory>
#include <tuple>
#include <unordered_map>
#include <vector>

class ASTVisitor;

class Expression {
protected:
	Slice s;
	int typeID = -1;
	Expression(const Slice &s);
public:
	int getTypeID();
	void setTypeID(int typeID);
	Slice &getSlice();
	virtual void accept(ASTVisitor &visitor) = 0;
	virtual ~Expression() = default;
};

class Statement {
protected:
	Slice s;
	Statement(const Slice &s);
public:
	Slice &getSlice();
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
	std::unordered_map<std::string_view, int> symbols;
public:
	BlockExpression(const Punctuation &open,
	      std::vector<std::unique_ptr<Statement>> statements,
	      std::unique_ptr<Expression> finalExpression, const Punctuation &close);
	void forEachStatement(const std::function<void(Statement &)> &statementHandler);
	Expression *getFinalExpression();
	int getSymbolType(std::string_view symbol);
	void setSymbolType(std::string_view symbol, int typeID);
	void accept(ASTVisitor &visitor) override;
	virtual ~BlockExpression() = default;
};

class ReturnExpression : public Expression {
	std::unique_ptr<Expression> expression;
public:
	ReturnExpression(const Keyword &returnKeyword,
	      std::unique_ptr<Expression> expression);
	Expression *getExpression();
	void accept(ASTVisitor &visitor) override;
	virtual ~ReturnExpression() = default;
};

class ParenthesizedExpression : public Expression {
private:
	std::unique_ptr<Expression> expression;
public:
	ParenthesizedExpression(const Punctuation &open,
	      std::unique_ptr<Expression> expression, const Punctuation &close);
	Expression &getExpression();
	void accept(ASTVisitor &visitor) override;
	virtual ~ParenthesizedExpression() = default;
};

class ExpressionStatement : public Statement {
private:
	std::unique_ptr<Expression> expression;
public:
	ExpressionStatement(std::unique_ptr<Expression> expression,
	      const Punctuation &semicolon);
	ExpressionStatement(std::unique_ptr<BlockExpression> expression);
	Expression &getExpression();
	void accept(ASTVisitor &visitor) override;
	virtual ~ExpressionStatement() = default;
};

class LetStatement : public Statement {
private:
	std::unique_ptr<Symbol> symbol;
	std::unique_ptr<Symbol> typeAnnotation;
	std::unique_ptr<Operator> equalSign;
	std::unique_ptr<Expression> expression;
public:
	LetStatement(const Keyword &let, std::unique_ptr<Symbol> symbol,
	      std::unique_ptr<Symbol> typeAnnotation, std::unique_ptr<Operator> equalSign,
	      std::unique_ptr<Expression> expression, const Punctuation &semicolon);
	Symbol &getSymbol();
	Expression &getExpression();
	Symbol &getTypeAnnotation();
	Operator &getEqualSign();
	void accept(ASTVisitor &visitor) override;
	virtual ~LetStatement() = default;
};

class Function {
private:
	std::unique_ptr<Symbol> returnTypeAnnotation;
	std::unique_ptr<BlockExpression> body;
	int typeID = -1;
public:
	Function(std::unique_ptr<Symbol> returnTypeAnnotation,
	      std::unique_ptr<BlockExpression> body);
	Symbol *getReturnTypeAnnotation();
	BlockExpression &getBody();
	int getTypeID();
	void setTypeID(int typeID);
	void accept(ASTVisitor &visitor);
	~Function() = default;
};

class Module {
private:
	std::unordered_map<std::string_view, std::unique_ptr<Function>> functions;
	std::unordered_map<std::string_view, int> typeTable;
	std::unordered_map<Operator::Type, std::vector<std::tuple<int, int>>> unaryOperators;
	std::unordered_map<Operator::Type, std::vector<std::tuple<int, int, int>>>
	      binaryOperators;
public:
	Module();
	void addFunction(std::unique_ptr<Symbol> name, std::unique_ptr<Function> function);
	void forEachFunction(
	      const std::function<void(std::string_view, Function &)> &functionHandler);
	int getType(std::string_view type);
	void insertType(std::string_view type);
	bool isTypeConvertible(int from, int to);
	void addUnaryOperator(Operator::Type op, int operandType, int resultType);
	int getUnaryOperator(Operator::Type op, int operandType);
	void addBinaryOperator(Operator::Type op, int leftType, int rightType,
	      int resultType);
	int getBinaryOperator(Operator::Type op, int leftType, int rightType);
	Function *getFunction(std::string_view name);
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
