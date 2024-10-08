#include "ccodeadapter.h"

#include "ast.h"

#include <list>
#include <memory>
#include <string_view>
#include <utility>
#include <vector>

CCodeAdapter::CCodeAdapter(Module *module, std::list<std::string> *generatedStrings)
    : inputModule(module), outputModule(std::make_unique<Module>(*inputModule)),
      generatedStrings(generatedStrings) {
}

std::unique_ptr<Module> CCodeAdapter::transform() {
	inputModule->accept(*this);
	return std::move(outputModule);
}

void CCodeAdapter::visit(FunctionCallExpression &node) {
	Expression &oldFunction = node.getFunction();
	auto *oldSymbol = dynamic_cast<SymbolExpression *>(&oldFunction);
	if (oldSymbol == nullptr) {
		std::cerr << "Function call target is not a symbol" << std::endl;
		exit(EXIT_FAILURE);
	}
	generatedStrings->push_back(
	      "CANYON_FUNCTION_" + std::string(oldSymbol->getSymbol().s.contents));
	std::string_view newName = generatedStrings->back();
	std::unique_ptr<Symbol> newSymbol
	      = std::make_unique<Symbol>(Slice(newName, inputModule->getSource(), 0, 0));
	std::unique_ptr<SymbolExpression> newSymbolExpression
	      = std::make_unique<SymbolExpression>(std::move(newSymbol));
	std::unique_ptr<FunctionCallExpression> newFunctionCall
	      = std::make_unique<FunctionCallExpression>(std::move(newSymbolExpression));
	newFunctionCall->setTypeID(oldFunction.getTypeID());
	returnValue = std::move(newFunctionCall);
}

void CCodeAdapter::visit(BinaryExpression &node) {
	Expression &oldLeft = node.getLeft();
	Expression &oldRight = node.getRight();
	Operator &oldOperator = node.getOperator();
	visitExpression(oldLeft);
	std::unique_ptr<Expression> newLeft = std::unique_ptr<Expression>(
	      dynamic_cast<Expression *>(returnValue.release()));
	visitExpression(oldRight);
	std::unique_ptr<Expression> newRight = std::unique_ptr<Expression>(
	      dynamic_cast<Expression *>(returnValue.release()));
	std::unique_ptr<Operator> newOperator = std::make_unique<Operator>(oldOperator);
	std::unique_ptr<BinaryExpression> newBinaryExpression
	      = std::make_unique<BinaryExpression>(std::move(newOperator), std::move(newLeft),
	            std::move(newRight));
	newBinaryExpression->setTypeID(node.getTypeID());
	returnValue = std::move(newBinaryExpression);
}

void CCodeAdapter::visit(UnaryExpression &node) {
	Expression &oldExpression = node.getExpression();
	Operator &oldOperator = node.getOperator();
	visitExpression(oldExpression);
	std::unique_ptr<Expression> newExpression = std::unique_ptr<Expression>(
	      dynamic_cast<Expression *>(returnValue.release()));
	std::unique_ptr<Operator> newOperator = std::make_unique<Operator>(oldOperator);
	std::unique_ptr<UnaryExpression> newUnaryExpression
	      = std::make_unique<UnaryExpression>(std::move(newOperator),
	            std::move(newExpression));
	newUnaryExpression->setTypeID(node.getTypeID());
	returnValue = std::move(newUnaryExpression);
}

void CCodeAdapter::visit(LiteralExpression &node) {
	IntegerLiteral &previousLiteral = node.getLiteral();
	std::unique_ptr<IntegerLiteral> newLiteral = std::make_unique<IntegerLiteral>(
	      previousLiteral, previousLiteral.type, previousLiteral.value);
	std::unique_ptr<LiteralExpression> newLiteralExpression
	      = std::make_unique<LiteralExpression>(std::move(newLiteral));
	newLiteralExpression->setTypeID(node.getTypeID());
	returnValue = std::move(newLiteralExpression);
}

void CCodeAdapter::visit(SymbolExpression &node) {
	Symbol &oldSymbol = node.getSymbol();
	generatedStrings->push_back("CANYON_LOCAL_" + std::string(oldSymbol.s.contents));
	std::string_view newName = generatedStrings->back();
	std::unique_ptr<Symbol> newSymbol
	      = std::make_unique<Symbol>(Slice(newName, inputModule->getSource(), 0, 0));
	std::unique_ptr<SymbolExpression> newSymbolExpression
	      = std::make_unique<SymbolExpression>(std::move(newSymbol));
	newSymbolExpression->setTypeID(node.getTypeID());
	returnValue = std::move(newSymbolExpression);
}

void CCodeAdapter::visit(BlockExpression &node) {
	BlockExpression &oldBlock = node;
	Expression *oldFinalExpression = oldBlock.getFinalExpression();
	std::unique_ptr<BlockExpression> newBlockExpression
	      = std::make_unique<BlockExpression>();
	scopeStack.push_back(newBlockExpression.get());

	oldBlock.forEachStatement([this, &newBlockExpression](Statement &statement) {
		statement.accept(*this);
		std::unique_ptr<Statement> newStatement = std::unique_ptr<Statement>(
		      dynamic_cast<Statement *>(returnValue.release()));
		newBlockExpression->pushStatement(std::move(newStatement));
	});

	if (oldFinalExpression != nullptr) {
		visitExpression(*oldFinalExpression);
		std::unique_ptr<Expression> newFinalExpression = std::unique_ptr<Expression>(
		      dynamic_cast<Expression *>(returnValue.release()));
		if (node.getTypeID() != inputModule->getType("()").id
		      && node.getTypeID() != inputModule->getType("!").id) {
			std::string_view tempVariableName = blockTemporaryVariables.top();
			Punctuation equalSign = Punctuation(
			      Slice("=", inputModule->getSource(), 0, 0), Punctuation::Type::Equals);
			std::unique_ptr<Operator> assignmentOperator
			      = std::make_unique<Operator>(equalSign, Operator::Type::Assignment);
			std::unique_ptr<BinaryExpression> assignment
			      = std::make_unique<BinaryExpression>(std::move(assignmentOperator),
			            std::make_unique<SymbolExpression>(std::make_unique<Symbol>(
			                  Slice(tempVariableName, inputModule->getSource(), 0, 0))),
			            std::move(newFinalExpression));
			std::unique_ptr<ExpressionStatement> newAssignment
			      = std::make_unique<ExpressionStatement>(std::move(assignment));
			newBlockExpression->pushStatement(std::move(newAssignment));
			blockTemporaryVariables.pop();
		} else {
			newBlockExpression->pushStatement(
			      std::make_unique<ExpressionStatement>(std::move(newFinalExpression)));
		}
	}
	newBlockExpression->setTypeID(oldBlock.getTypeID());
	scopeStack.pop_back();
	returnValue = std::move(newBlockExpression);
}

void CCodeAdapter::visit(ReturnExpression &node) {
	Expression *oldExpression = node.getExpression();
	std::unique_ptr<ReturnExpression> newReturnExpression = nullptr;
	if (oldExpression != nullptr) {
		visitExpression(*oldExpression);
		std::unique_ptr<Expression> newExpression = std::unique_ptr<Expression>(
		      dynamic_cast<Expression *>(returnValue.release()));
		newReturnExpression
		      = std::make_unique<ReturnExpression>(std::move(newExpression));
	} else {
		newReturnExpression = std::make_unique<ReturnExpression>(nullptr);
	}
	newReturnExpression->setTypeID(node.getTypeID());
	returnValue = std::move(newReturnExpression);
}

void CCodeAdapter::visit(ParenthesizedExpression &node) {
	Expression &oldExpression = node.getExpression();
	visitExpression(oldExpression);
	std::unique_ptr<Expression> newExpression = std::unique_ptr<Expression>(
	      dynamic_cast<Expression *>(returnValue.release()));
	std::unique_ptr<ParenthesizedExpression> newParenthesizedExpression
	      = std::make_unique<ParenthesizedExpression>(std::move(newExpression));
	newParenthesizedExpression->setTypeID(node.getTypeID());
	returnValue = std::move(newParenthesizedExpression);
}

void CCodeAdapter::visit(ExpressionStatement &node) {
	Expression &oldExpression = node.getExpression();
	visitExpression(oldExpression);
	std::unique_ptr<Expression> newExpression = std::unique_ptr<Expression>(
	      dynamic_cast<Expression *>(returnValue.release()));
	returnValue = std::make_unique<ExpressionStatement>(std::move(newExpression));
}

void CCodeAdapter::visit(LetStatement &node) {
	Symbol &oldSymbol = node.getSymbol();
	Expression *oldExpression = node.getExpression();
	generatedStrings->push_back("CANYON_LOCAL_" + std::string(oldSymbol.s.contents));
	std::string_view newName = generatedStrings->back();
	std::unique_ptr<Symbol> newSymbol
	      = std::make_unique<Symbol>(Slice(newName, inputModule->getSource(), 0, 0));
	visitExpression(*oldExpression);
	std::unique_ptr<Expression> newExpression = std::unique_ptr<Expression>(
	      dynamic_cast<Expression *>(returnValue.release()));
	std::unique_ptr<LetStatement> newLetStatement = std::make_unique<LetStatement>(
	      std::move(newSymbol), std::move(newExpression));
	newLetStatement->setSymbolTypeID(node.getSymbolTypeID());
	returnValue = std::move(newLetStatement);
}

void CCodeAdapter::visit(Function &node) {
	Expression &oldBody = node.getBody();
	std::unique_ptr<BlockExpression> enclosingScope = std::make_unique<BlockExpression>();
	scopeStack.push_back(enclosingScope.get());
	visitExpression(oldBody);
	std::unique_ptr<Expression> newBody = std::unique_ptr<Expression>(
	      dynamic_cast<Expression *>(returnValue.release()));
	scopeStack.pop_back();
	std::unique_ptr<ExpressionStatement> bodyStatement
	      = std::make_unique<ExpressionStatement>(std::move(newBody));
	enclosingScope->pushStatement(std::move(bodyStatement));
	returnValue = std::make_unique<Function>(std::move(enclosingScope));
}

void CCodeAdapter::visit(Module &node) {
	node.forEachFunction([this](std::string_view name, Function &oldFunction) {
		oldFunction.accept(*this);
		std::unique_ptr<Function> newFunction = std::unique_ptr<Function>(
		      dynamic_cast<Function *>(returnValue.release()));
		generatedStrings->push_back("CANYON_FUNCTION_" + std::string(name));
		std::string_view newName = generatedStrings->back();
		newFunction->setTypeID(oldFunction.getTypeID());
		outputModule->addFunction(
		      std::make_unique<Symbol>(Slice(newName, inputModule->getSource(), 0, 0)),
		      std::move(newFunction));
	});
}

void CCodeAdapter::visitExpression(Expression &node) {
	auto *blockExpression = dynamic_cast<BlockExpression *>(&node);
	if (blockExpression != nullptr
	      && blockExpression->getTypeID() != inputModule->getType("()").id
	      && blockExpression->getTypeID() != inputModule->getType("!").id) {
		generatedStrings->push_back("CANYON_BLOCK_" + std::to_string(blockCount++));
		std::string_view tempVariableName = generatedStrings->back();
		std::unique_ptr<LetStatement> declaration = std::make_unique<LetStatement>(
		      std::make_unique<Symbol>(
		            Slice(tempVariableName, inputModule->getSource(), 0, 0)),
		      nullptr);
		scopeStack.back()->setSymbolType(tempVariableName, node.getTypeID());
		declaration->setSymbolTypeID(node.getTypeID());
		scopeStack.back()->pushStatement(std::move(declaration));
		blockTemporaryVariables.push(tempVariableName);
		node.accept(*this);
		std::unique_ptr<BlockExpression> newBlock = std::unique_ptr<BlockExpression>(
		      dynamic_cast<BlockExpression *>(returnValue.release()));
		std::unique_ptr<ExpressionStatement> blockExpressionStatement
		      = std::make_unique<ExpressionStatement>(std::move(newBlock));
		scopeStack.back()->pushStatement(std::move(blockExpressionStatement));
		returnValue = std::make_unique<SymbolExpression>(std::make_unique<Symbol>(
		      Slice(tempVariableName, inputModule->getSource(), 0, 0)));
	} else {
		node.accept(*this);
	}
}
