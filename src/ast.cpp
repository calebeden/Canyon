#include "ast.h"

#include <functional>
#include <iostream>
#include <memory>
#include <vector>

FunctionCallExpression::FunctionCallExpression(std::unique_ptr<Expression> function)
    : function(std::move(function)) {
}

Expression &FunctionCallExpression::getFunction() {
	return *function;
}

void FunctionCallExpression::accept(ASTVisitor &visitor) {
	visitor.visit(*this);
}

BinaryExpression::BinaryExpression(std::unique_ptr<Operator> op,
      std::unique_ptr<Expression> left, std::unique_ptr<Expression> right)
    : op(std::move(op)), left(std::move(left)), right(std::move(right)) {
}

Expression &BinaryExpression::getLeft() {
	return *left;
}

Expression &BinaryExpression::getRight() {
	return *right;
}

Operator &BinaryExpression::getOperator() {
	return *op;
}

void BinaryExpression::accept(ASTVisitor &visitor) {
	visitor.visit(*this);
}

UnaryExpression::UnaryExpression(std::unique_ptr<Operator> op,
      std::unique_ptr<Expression> operand)
    : op(std::move(op)), operand(std::move(operand)) {
}

Expression &UnaryExpression::getExpression() {
	return *operand;
}

Operator &UnaryExpression::getOperator() {
	return *op;
}

void UnaryExpression::accept(ASTVisitor &visitor) {
	visitor.visit(*this);
}

LiteralExpression::LiteralExpression(std::unique_ptr<IntegerLiteral> literal)
    : literal(std::move(literal)) {
}

IntegerLiteral &LiteralExpression::getLiteral() {
	return *literal;
}

void LiteralExpression::accept(ASTVisitor &visitor) {
	visitor.visit(*this);
}

SymbolExpression::SymbolExpression(std::unique_ptr<Symbol> symbol)
    : symbol(std::move(symbol)) {
}

Symbol &SymbolExpression::getSymbol() {
	return *symbol;
}

void SymbolExpression::accept(ASTVisitor &visitor) {
	visitor.visit(*this);
}

BlockExpression::BlockExpression() {
}

void BlockExpression::pushStatement(std::unique_ptr<Statement> statement) {
	statements.push_back(std::move(statement));
}

void BlockExpression::setFinalExpression(std::unique_ptr<Expression> expr) {
	finalExpression = std::move(expr);
}

void BlockExpression::forEachStatement(
      std::function<void(Statement &)> statementHandler) {
	for (auto &statement : statements) {
		statementHandler(*statement);
	}
}

Expression *BlockExpression::getFinalExpression() {
	return finalExpression.get();
}

void BlockExpression::accept(ASTVisitor &visitor) {
	visitor.visit(*this);
}

ReturnExpression::ReturnExpression(std::unique_ptr<Expression> expression)
    : expression(std::move(expression)) {
}

Expression *ReturnExpression::getExpression() {
	return expression.get();
}

void ReturnExpression::accept(ASTVisitor &visitor) {
	visitor.visit(*this);
}

ParenthesizedExpression::ParenthesizedExpression(std::unique_ptr<Expression> expression)
    : expression(std::move(expression)) {
}

Expression &ParenthesizedExpression::getExpression() {
	return *expression;
}

void ParenthesizedExpression::accept(ASTVisitor &visitor) {
	visitor.visit(*this);
}

ExpressionStatement::ExpressionStatement(std::unique_ptr<Expression> expression)
    : expression(std::move(expression)) {
}

Expression &ExpressionStatement::getExpression() {
	return *expression;
}

void ExpressionStatement::accept(ASTVisitor &visitor) {
	visitor.visit(*this);
}

LetStatement::LetStatement(std::unique_ptr<Symbol> symbol, std::unique_ptr<Symbol> type,
      std::unique_ptr<Expression> expression)
    : symbol(std::move(symbol)), type(std::move(type)),
      expression(std::move(expression)) {
}

Symbol &LetStatement::getSymbol() {
	return *symbol;
}

Expression &LetStatement::getExpression() {
	return *expression;
}

Symbol &LetStatement::getType() {
	return *type;
}

void LetStatement::accept(ASTVisitor &visitor) {
	visitor.visit(*this);
}

Function::Function(std::unique_ptr<Symbol> name, std::unique_ptr<Symbol> returnType,
      std::unique_ptr<BlockExpression> body)
    : name(std::move(name)), returnType(std::move(returnType)), body(std::move(body)) {
}

Symbol &Function::getName() {
	return *name;
}

Symbol &Function::getReturnType() {
	return *returnType;
}

BlockExpression &Function::getBody() {
	return *body;
}

void Function::accept(ASTVisitor &visitor) {
	visitor.visit(*this);
}

Module::Module() {
}

void Module::pushFunction(std::unique_ptr<Function> function) {
	functions.push_back(std::move(function));
}

void Module::forEachFunction(std::function<void(Function &)> funcHandler) {
	for (auto &function : functions) {
		funcHandler(*function);
	}
}

void ASTPrinter::visit(FunctionCallExpression &node) {
	std::cerr << '(';
	node.getFunction().accept(*this);
	std::cerr << ')';
}

void ASTPrinter::visit(BinaryExpression &node) {
	std::cerr << '(';
	node.getOperator().print(std::cerr);
	std::cerr << ' ';
	node.getLeft().accept(*this);
	std::cerr << ' ';
	node.getRight().accept(*this);
	std::cerr << ')';
}

void ASTPrinter::visit(UnaryExpression &node) {
	std::cerr << '(';
	node.getOperator().print(std::cerr);
	std::cerr << ' ';
	node.getExpression().accept(*this);
	std::cerr << ')';
}

void ASTPrinter::visit(LiteralExpression &node) {
	node.getLiteral().print(std::cerr);
}

void ASTPrinter::visit(SymbolExpression &node) {
	node.getSymbol().print(std::cerr);
}

void ASTPrinter::visit(BlockExpression &node) {
	std::cerr << "{\n";
	tabLevel++;
	node.forEachStatement([this](Statement &statement) {
		std::cerr << std::string(tabLevel, '\t');
		statement.accept(*this);
		std::cerr << '\n';
	});
	Expression *finalExpression = node.getFinalExpression();
	if (finalExpression != nullptr) {
		std::cerr << std::string(tabLevel, '\t');
		finalExpression->accept(*this);
	} else {
		std::cerr << std::string(tabLevel, '\t');
		std::cerr << Type::UNIT;
	}
	tabLevel--;
	std::cerr << '\n' << std::string(tabLevel, '\t') << '}';
}

void ASTPrinter::visit(ReturnExpression &node) {
	std::cerr << "return";
	Expression *expr = node.getExpression();
	if (expr != nullptr) {
		std::cerr << ' ';
		expr->accept(*this);
	}
}

void ASTPrinter::visit(ParenthesizedExpression &node) {
	node.getExpression().accept(*this);
}

void ASTPrinter::visit(ExpressionStatement &node) {
	node.getExpression().accept(*this);
	std::cerr << ';';
}

void ASTPrinter::visit(LetStatement &node) {
	std::cerr << "let ";
	node.getSymbol().print(std::cerr);
	std::cerr << ": ";
	node.getType().print(std::cerr);
	std::cerr << " = ";
	node.getExpression().accept(*this);
	std::cerr << ';';
}

void ASTPrinter::visit(Function &node) {
	std::cerr << std::string(tabLevel, '\t') << "fun ";
	node.getName().print(std::cerr);
	std::cerr << "(): ";
	node.getReturnType().print(std::cerr);
	std::cerr << ' ';
	node.getBody().accept(*this);
}

void ASTPrinter::visit(Module &node) {
	node.forEachFunction([this](Function &function) {
		function.accept(*this);
		std::cerr << '\n';
	});
}
