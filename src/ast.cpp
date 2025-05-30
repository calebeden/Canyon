#include "ast.h"

#include <functional>
#include <iostream>
#include <memory>
#include <utility>
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

FunctionCallExpression::FunctionCallExpression(std::unique_ptr<Expression> function,
      [[maybe_unused]] const Punctuation &open,
      std::vector<std::unique_ptr<Expression>> arguments, const Punctuation &close)
    : Expression(Slice::merge(function->getSlice(), close.s)),
      function(std::move(function)), arguments(std::move(arguments)) {
}

FunctionCallExpression::FunctionCallExpression(std::unique_ptr<Expression> function,
      std::vector<std::unique_ptr<Expression>> arguments)
    : Expression(function->getSlice()), function(std::move(function)),
      arguments(std::move(arguments)) {
}

Expression &FunctionCallExpression::getFunction() {
	return *function;
}

void FunctionCallExpression::forEachArgument(
      const std::function<void(Expression &)> &argumentHandler) {
	for (auto &argument : arguments) {
		argumentHandler(*argument);
	}
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

IntegerLiteralExpression::IntegerLiteralExpression(
      std::unique_ptr<IntegerLiteral> literal)
    : Expression(literal->s), literal(std::move(literal)) {
}

IntegerLiteral &IntegerLiteralExpression::getLiteral() {
	return *literal;
}

void IntegerLiteralExpression::accept(ASTVisitor &visitor) {
	visitor.visit(*this);
}

BoolLiteralExpression::BoolLiteralExpression(std::unique_ptr<BoolLiteral> literal)
    : Expression(literal->s), literal(std::move(literal)) {
}

BoolLiteral &BoolLiteralExpression::getLiteral() {
	return *literal;
}

void BoolLiteralExpression::accept(ASTVisitor &visitor) {
	visitor.visit(*this);
}

CharacterLiteralExpression::CharacterLiteralExpression(
      std::unique_ptr<CharacterLiteral> literal)
    : Expression(literal->s), literal(std::move(literal)) {
}

CharacterLiteral &CharacterLiteralExpression::getLiteral() {
	return *literal;
}

void CharacterLiteralExpression::accept(ASTVisitor &visitor) {
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

BlockExpression::BlockExpression() : Expression(Slice("", "", 0, 0)) {
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
	return std::get<0>(symbols[symbol]);
}

SymbolSource BlockExpression::getSymbolSource(std::string_view symbol) {
	if (symbols.find(symbol) == symbols.end()) {
		return SymbolSource::Unknown;
	}
	return std::get<1>(symbols[symbol]);
}

void BlockExpression::pushSymbol(std::string_view symbol, int typeID,
      SymbolSource source) {
	if (symbols.find(symbol) == symbols.end()) {
		symbols[symbol] = {typeID, source};
	} else {
		std::cerr << "Symbol already exists";
		exit(EXIT_FAILURE);
	}
}

void BlockExpression::forEachSymbol(
      const std::function<void(std::string_view, int, SymbolSource)> &symbolHandler) {
	for (auto &[symbol, info] : symbols) {
		symbolHandler(symbol, std::get<0>(info), std::get<1>(info));
	}
}

void BlockExpression::pushStatement(std::unique_ptr<Statement> statement) {
	statements.push_back(std::move(statement));
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

ReturnExpression::ReturnExpression(std::unique_ptr<Expression> expression)
    : Expression(Slice("", "", 0, 0)), expression(std::move(expression)) {
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

ParenthesizedExpression::ParenthesizedExpression(std::unique_ptr<Expression> expression)
    : Expression(expression->getSlice()), expression(std::move(expression)) {
}

Expression &ParenthesizedExpression::getExpression() {
	return *expression;
}

void ParenthesizedExpression::accept(ASTVisitor &visitor) {
	visitor.visit(*this);
}

IfElseExpression::IfElseExpression(const Keyword &ifKeyword,
      std::unique_ptr<Expression> condition, std::unique_ptr<BlockExpression> thenBlock,
      [[maybe_unused]] const Keyword &elseKeyword,
      std::unique_ptr<Expression> elseExpression)
    : Expression(Slice::merge(ifKeyword.s, elseExpression->getSlice())),
      condition(std::move(condition)), thenBlock(std::move(thenBlock)),
      elseExpression(std::move(elseExpression)) {
}

IfElseExpression::IfElseExpression(const Keyword &ifKeyword,
      std::unique_ptr<Expression> condition, std::unique_ptr<BlockExpression> thenBlock)
    : Expression(Slice::merge(ifKeyword.s, thenBlock->getSlice())),
      condition(std::move(condition)), thenBlock(std::move(thenBlock)),
      elseExpression(nullptr) {
}

IfElseExpression::IfElseExpression(std::unique_ptr<Expression> condition,
      std::unique_ptr<BlockExpression> thenBlock,
      std::unique_ptr<Expression> elseExpression)
    : Expression(Slice("", "", 0, 0)), condition(std::move(condition)),
      thenBlock(std::move(thenBlock)), elseExpression(std::move(elseExpression)) {
}

WhileExpression::WhileExpression(const Keyword &whileKeyword,
      std::unique_ptr<Expression> condition, std::unique_ptr<BlockExpression> body)
    : Expression(Slice::merge(whileKeyword.s, body->getSlice())),
      condition(std::move(condition)), body(std::move(body)) {
}

WhileExpression::WhileExpression(std::unique_ptr<Expression> condition,
      std::unique_ptr<BlockExpression> body)
    : Expression(Slice("", condition->getSlice().source, 0, 0)),
      condition(std::move(condition)), body(std::move(body)) {
}

Expression &WhileExpression::getCondition() {
	return *condition;
}

BlockExpression &WhileExpression::getBody() {
	return *body;
}

void WhileExpression::accept(ASTVisitor &visitor) {
	visitor.visit(*this);
}

Expression &IfElseExpression::getCondition() {
	return *condition;
}

BlockExpression &IfElseExpression::getThenBlock() {
	return *thenBlock;
}

Expression *IfElseExpression::getElseExpression() {
	return elseExpression.get();
}

void IfElseExpression::accept(ASTVisitor &visitor) {
	visitor.visit(*this);
}

ExpressionStatement::ExpressionStatement(std::unique_ptr<Expression> expression,
      const Punctuation &semicolon)
    : Statement(Slice::merge(expression->getSlice(), semicolon.s)),
      expression(std::move(expression)) {
}

ExpressionStatement::ExpressionStatement(std::unique_ptr<Expression> expression)
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

LetStatement::LetStatement(std::unique_ptr<Symbol> symbol,
      std::unique_ptr<Expression> expression)
    : Statement(Slice("", "", 0, 0)), symbol(std::move(symbol)), typeAnnotation(nullptr),
      equalSign(nullptr), expression(std::move(expression)) {
}

Symbol &LetStatement::getSymbol() {
	return *symbol;
}

Expression *LetStatement::getExpression() {
	return expression.get();
}

Symbol *LetStatement::getTypeAnnotation() {
	return typeAnnotation.get();
}

Operator &LetStatement::getEqualSign() {
	return *equalSign;
}

void LetStatement::setSymbolTypeID(int typeID) {
	symbolTypeID = typeID;
}

int LetStatement::getSymbolTypeID() const {
	return symbolTypeID;
}

void LetStatement::accept(ASTVisitor &visitor) {
	visitor.visit(*this);
}

Function::Function(
      std::vector<std::pair<std::unique_ptr<Symbol>, std::unique_ptr<Symbol>>> parameters,
      std::unique_ptr<Symbol> returnTypeAnnotation, std::unique_ptr<BlockExpression> body)
    : parameters(std::move(parameters)),
      returnTypeAnnotation(std::move(returnTypeAnnotation)), body(std::move(body)) {
}

Function::Function(
      std::vector<std::pair<std::unique_ptr<Symbol>, std::unique_ptr<Symbol>>> parameters,
      std::unique_ptr<BlockExpression> body)
    : parameters(std::move(parameters)), returnTypeAnnotation(nullptr),
      body(std::move(body)) {
}

void Function::forEachParameter(
      const std::function<void(Symbol &, Symbol &)> &parameterHandler) {
	for (auto &[symbol, type] : parameters) {
		parameterHandler(*symbol, *type);
	}
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

Type::Type(int id, int parentID, std::string_view name)
    : id(id), parentID(parentID), name(name) {
}

Module::Module(std::filesystem::path source) : source(std::move(source)) {
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
	insertType("bool");
	insertType("char");
}

Module::Module(const Module &module)
    : typeTableByName(module.typeTableByName), typeTableByID(module.typeTableByID),
      unaryOperators(module.unaryOperators), binaryOperators(module.binaryOperators),
      source(module.source) {
}

void Module::addFunction(std::unique_ptr<Symbol> name, std::unique_ptr<Function> function,
      bool isBuiltin) {
	functions[name->s.contents] = {std::move(function), isBuiltin};
}

void Module::forEachFunction(
      const std::function<void(std::string_view, Function &, bool)> &functionHandler) {
	for (auto &[name, function] : functions) {
		functionHandler(name, *std::get<0>(function), std::get<1>(function));
	}
}

Type Module::getType(std::string_view typeName) {
	if (typeTableByName.find(typeName) == typeTableByName.end()) {
		return Type(-1, -1, "");
	}
	return typeTableByName.at(typeName);
}

Type Module::getType(int id) {
	if (typeTableByID.find(id) == typeTableByID.end()) {
		return Type(-1, -1, "");
	}
	return typeTableByID.at(id);
}

void Module::insertType(std::string_view typeName) {
	if (typeTableByName.find(typeName) == typeTableByName.end()) {
		typeTableByName.insert({typeName, Type(typeTableByName.size(), -1, typeName)});
		typeTableByID.insert(
		      {typeTableByID.size(), Type(typeTableByID.size(), -1, typeName)});
	} else {
		std::cerr << "Type already exists";
		exit(EXIT_FAILURE);
	}
}

bool Module::isTypeConvertible(int from, int to) {
	if (from == to) {
		return true;
	}
	if (from == getType("!").id) {
		return true;
	}
	return false;
}

Type Module::getCommonTypeAncestor(int type1, int type2) {
	if (type1 == type2) {
		return getType(type1);
	}
	if (type1 == getType("!").id) {
		return getType(type2);
	}
	if (type2 == getType("!").id) {
		return getType(type1);
	}
	return Type(-1, -1, "");
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
	return std::get<0>(functions[name]).get();
}

std::filesystem::path Module::getSource() {
	return source;
}

void Module::accept(ASTVisitor &visitor) {
	visitor.visit(*this);
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

void ASTPrinter::visit(IntegerLiteralExpression &node) {
	node.getLiteral().print(std::cerr);
}

void ASTPrinter::visit(BoolLiteralExpression &node) {
	node.getLiteral().print(std::cerr);
}

void ASTPrinter::visit(CharacterLiteralExpression &node) {
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

void ASTPrinter::visit(IfElseExpression &node) {
	std::cerr << "if ";
	node.getCondition().accept(*this);
	std::cerr << ' ';
	node.getThenBlock().accept(*this);
	Expression *elseExpression = node.getElseExpression();
	if (elseExpression != nullptr) {
		std::cerr << " else ";
		elseExpression->accept(*this);
	}
}

void ASTPrinter::visit(WhileExpression &node) {
	std::cerr << "while ";
	node.getCondition().accept(*this);
	std::cerr << ' ';
	node.getBody().accept(*this);
}

void ASTPrinter::visit(ExpressionStatement &node) {
	node.getExpression().accept(*this);
	std::cerr << ';';
}

void ASTPrinter::visit(LetStatement &node) {
	Expression *expression = node.getExpression();
	Symbol *typeAnnotation = node.getTypeAnnotation();
	if (expression != nullptr) {
		std::cerr << "let ";
		node.getSymbol().print(std::cerr);
		if (typeAnnotation != nullptr) {
			std::cerr << ": ";
			typeAnnotation->print(std::cerr);
		}
		std::cerr << " = ";
		expression->accept(*this);
		std::cerr << ';';
	} else {
		std::cerr << "let ";
		node.getSymbol().print(std::cerr);
		if (typeAnnotation != nullptr) {
			std::cerr << ": ";
			typeAnnotation->print(std::cerr);
		}
		std::cerr << ';';
	}
}

void ASTPrinter::visit([[maybe_unused]] Function &node) {
}

void ASTPrinter::visit(Module &node) {
	node.forEachFunction(
	      [this](std::string_view name, Function &function, bool /*unused*/) {
		      std::cerr << std::string(tabLevel, '\t') << "fun " << name
		                << "():" << function.getReturnTypeAnnotation()->s << ' ';
		      function.getBody().accept(*this);
		      std::cerr << '\n';
	      });
}
