#include "ast.h"

#include <functional>
#include <iostream>
#include <memory>
#include <vector>

Expression::Expression(const Slice &s) : s(s) {
}

int Expression::getTypeID() const {
	return typeID;
}

void Expression::setTypeID(int typeID) {
	this->typeID = typeID;
}

Slice &Expression::getSlice() {
	return s;
}

Statement::Statement(const Slice &s) : s(s) {
}

Slice &Statement::getSlice() {
	return s;
}

FunctionCallExpression::FunctionCallExpression(std::unique_ptr<Expression> function)
    : Expression(function->getSlice()), function(std::move(function)) {
}

Expression &FunctionCallExpression::getFunction() {
	return *function;
}

void FunctionCallExpression::accept(ASTVisitor &visitor) {
	visitor.visit(*this);
}

BinaryExpression::BinaryExpression(std::unique_ptr<Operator> op,
      std::unique_ptr<Expression> left, std::unique_ptr<Expression> right)
    : Expression(Slice::merge(left->getSlice(), right->getSlice())), op(std::move(op)),
      left(std::move(left)), right(std::move(right)) {
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
    : Expression(Slice::merge(op->s, operand->getSlice())), op(std::move(op)),
      operand(std::move(operand)) {
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
    : Expression(literal->s), literal(std::move(literal)) {
}

IntegerLiteral &LiteralExpression::getLiteral() {
	return *literal;
}

void LiteralExpression::accept(ASTVisitor &visitor) {
	visitor.visit(*this);
}

SymbolExpression::SymbolExpression(std::unique_ptr<Symbol> symbol)
    : Expression(symbol->s), symbol(std::move(symbol)) {
}

Symbol &SymbolExpression::getSymbol() {
	return *symbol;
}

void SymbolExpression::accept(ASTVisitor &visitor) {
	visitor.visit(*this);
}

BlockExpression::BlockExpression(const Punctuation &open,
      std::vector<std::unique_ptr<Statement>> statements,
      std::unique_ptr<Expression> finalExpression, const Punctuation &close)
    : Expression(Slice::merge(open.s, close.s)), statements(std::move(statements)),
      finalExpression(std::move(finalExpression)) {
}

void BlockExpression::forEachStatement(
      const std::function<void(Statement &)> &statementHandler) {
	for (auto &statement : statements) {
		statementHandler(*statement);
	}
}

Expression *BlockExpression::getFinalExpression() {
	return finalExpression.get();
}

int BlockExpression::getSymbolType(std::string_view symbol) {
	if (symbols.find(symbol) == symbols.end()) {
		return -1;
	}
	return symbols[symbol];
}

void BlockExpression::setSymbolType(std::string_view symbol, int typeID) {
	if (symbols.find(symbol) == symbols.end()) {
		symbols[symbol] = typeID;
	} else {
		std::cerr << "Symbol already exists";
		exit(EXIT_FAILURE);
	}
}

void BlockExpression::accept(ASTVisitor &visitor) {
	visitor.visit(*this);
}

ReturnExpression::ReturnExpression(const Keyword &returnKeyword,
      std::unique_ptr<Expression> expression)
    : Expression((expression == nullptr)
                       ? returnKeyword.s
                       : Slice::merge(returnKeyword.s, expression->getSlice())),
      expression(std::move(expression)) {
}

Expression *ReturnExpression::getExpression() {
	return expression.get();
}

void ReturnExpression::accept(ASTVisitor &visitor) {
	visitor.visit(*this);
}

ParenthesizedExpression::ParenthesizedExpression(const Punctuation &open,
      std::unique_ptr<Expression> expression, const Punctuation &close)
    : Expression(Slice::merge(open.s, close.s)), expression(std::move(expression)) {
}

Expression &ParenthesizedExpression::getExpression() {
	return *expression;
}

void ParenthesizedExpression::accept(ASTVisitor &visitor) {
	visitor.visit(*this);
}

ExpressionStatement::ExpressionStatement(std::unique_ptr<Expression> expression,
      const Punctuation &semicolon)
    : Statement(Slice::merge(expression->getSlice(), semicolon.s)),
      expression(std::move(expression)) {
}

ExpressionStatement::ExpressionStatement(std::unique_ptr<BlockExpression> expression)
    : Statement(expression->getSlice()), expression(std::move(expression)) {
}

Expression &ExpressionStatement::getExpression() {
	return *expression;
}

void ExpressionStatement::accept(ASTVisitor &visitor) {
	visitor.visit(*this);
}

LetStatement::LetStatement(const Keyword &let, std::unique_ptr<Symbol> symbol,
      std::unique_ptr<Symbol> typeAnnotation, std::unique_ptr<Operator> equalSign,
      std::unique_ptr<Expression> expression, Punctuation *semicolon)
    : Statement(Slice::merge(let.s,
            semicolon != nullptr ? semicolon->s : expression->getSlice())),
      symbol(std::move(symbol)), typeAnnotation(std::move(typeAnnotation)),
      equalSign(std::move(equalSign)), expression(std::move(expression)) {
}

Symbol &LetStatement::getSymbol() {
	return *symbol;
}

Expression &LetStatement::getExpression() {
	return *expression;
}

Symbol &LetStatement::getTypeAnnotation() {
	return *typeAnnotation;
}

Operator &LetStatement::getEqualSign() {
	return *equalSign;
}

void LetStatement::accept(ASTVisitor &visitor) {
	visitor.visit(*this);
}

Function::Function(std::unique_ptr<Symbol> returnTypeAnnotation,
      std::unique_ptr<BlockExpression> body)
    : returnTypeAnnotation(std::move(returnTypeAnnotation)), body(std::move(body)) {
}

Symbol *Function::getReturnTypeAnnotation() {
	return returnTypeAnnotation.get();
}

BlockExpression &Function::getBody() {
	return *body;
}

int Function::getTypeID() const {
	return typeID;
}

void Function::setTypeID(int typeID) {
	this->typeID = typeID;
}

void Function::accept(ASTVisitor &visitor) {
	visitor.visit(*this);
}

Module::Module() {
	insertType("()");
	insertType("!");
	insertType("i8");
	insertType("i16");
	insertType("i32");
	insertType("i64");
	insertType("u8");
	insertType("u16");
	insertType("u32");
	insertType("u64");
}

void Module::addFunction(std::unique_ptr<Symbol> name,
      std::unique_ptr<Function> function) {
	functions[name->s.contents] = std::move(function);
}

void Module::forEachFunction(
      const std::function<void(std::string_view, Function &)> &functionHandler) {
	for (auto &[name, function] : functions) {
		functionHandler(name, *function);
	}
}

int Module::getType(std::string_view type) {
	if (typeTable.find(type) == typeTable.end()) {
		return -1;
	}
	return typeTable[type];
}

void Module::insertType(std::string_view type) {
	if (typeTable.find(type) == typeTable.end()) {
		typeTable[type] = typeTable.size();
	} else {
		std::cerr << "Type already exists";
		exit(EXIT_FAILURE);
	}
}

bool Module::isTypeConvertible(int from, int to) {
	if (from == to) {
		return true;
	}
	if (from == getType("!")) {
		return true;
	}
	return false;
}

void Module::addUnaryOperator(Operator::Type op, int operandType, int resultType) {
	if (unaryOperators.find(op) == unaryOperators.end()) {
		unaryOperators[op] = {};
	}
	unaryOperators[op].push_back({operandType, resultType});
}

int Module::getUnaryOperator(Operator::Type op, int operandType) {
	if (unaryOperators.find(op) == unaryOperators.end()) {
		return -1;
	}
	for (auto &[operand, result] : unaryOperators[op]) {
		if (operand == operandType) {
			return result;
		}
	}
	return -1;
}

void Module::addBinaryOperator(Operator::Type op, int leftType, int rightType,
      int resultType) {
	if (binaryOperators.find(op) == binaryOperators.end()) {
		binaryOperators[op] = {};
	}
	binaryOperators[op].push_back({leftType, rightType, resultType});
}

int Module::getBinaryOperator(Operator::Type op, int leftType, int rightType) {
	if (binaryOperators.find(op) == binaryOperators.end()) {
		return -1;
	}
	for (auto &[left, right, result] : binaryOperators[op]) {
		if (left == leftType && right == rightType) {
			return result;
		}
	}
	return -1;
}

Function *Module::getFunction(std::string_view name) {
	if (functions.find(name) == functions.end()) {
		return nullptr;
	}
	return functions[name].get();
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
		std::cerr << std::string(tabLevel, '\t') << "()";
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
	node.getTypeAnnotation().print(std::cerr);
	std::cerr << " = ";
	node.getExpression().accept(*this);
	std::cerr << ';';
}

void ASTPrinter::visit([[maybe_unused]] Function &node) {
}

void ASTPrinter::visit(Module &node) {
	node.forEachFunction([this](std::string_view name, Function &function) {
		std::cerr << std::string(tabLevel, '\t') << "fun " << name
		          << "():" << function.getReturnTypeAnnotation()->s << ' ';
		function.getBody().accept(*this);
		std::cerr << '\n';
	});
}
