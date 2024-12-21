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

	std::vector<std::unique_ptr<Expression>> newArguments;
	node.forEachArgument([this, &newArguments](Expression &argument) {
		generatedStrings->push_back("CANYON_ARGUMENT_" + std::to_string(blockCount++));
		std::string_view tempVariableName = generatedStrings->back();
		std::unique_ptr<Symbol> tempSymbol = std::make_unique<Symbol>(
		      Slice(tempVariableName, inputModule->getSource(), 0, 0));
		visitExpression(argument);
		std::unique_ptr<Expression> newArgument = std::unique_ptr<Expression>(
		      dynamic_cast<Expression *>(returnValue.release()));
		std::unique_ptr<LetStatement> newLetStatement = std::make_unique<LetStatement>(
		      std::make_unique<Symbol>(*tempSymbol), std::move(newArgument));
		scopeStack.back()->pushSymbol(tempVariableName, argument.getTypeID(),
		      SymbolSource::GENERATED_Argument);
		newLetStatement->setSymbolTypeID(argument.getTypeID());
		scopeStack.back()->pushStatement(std::move(newLetStatement));
		newArguments.push_back(std::make_unique<SymbolExpression>(std::move(tempSymbol)));
	});

	std::unique_ptr<FunctionCallExpression> newFunctionCall
	      = std::make_unique<FunctionCallExpression>(std::move(newSymbolExpression),
	            std::move(newArguments));
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

void CCodeAdapter::visit(IntegerLiteralExpression &node) {
	IntegerLiteral &previousLiteral = node.getLiteral();
	std::unique_ptr<IntegerLiteral> newLiteral = std::make_unique<IntegerLiteral>(
	      previousLiteral, previousLiteral.type, previousLiteral.value);
	std::unique_ptr<IntegerLiteralExpression> newLiteralExpression
	      = std::make_unique<IntegerLiteralExpression>(std::move(newLiteral));
	newLiteralExpression->setTypeID(node.getTypeID());
	returnValue = std::move(newLiteralExpression);
}

void CCodeAdapter::visit(BoolLiteralExpression &node) {
	BoolLiteral &previousLiteral = node.getLiteral();
	std::unique_ptr<BoolLiteral> newLiteral
	      = std::make_unique<BoolLiteral>(previousLiteral, previousLiteral.value);
	std::unique_ptr<BoolLiteralExpression> newLiteralExpression
	      = std::make_unique<BoolLiteralExpression>(std::move(newLiteral));
	newLiteralExpression->setTypeID(node.getTypeID());
	returnValue = std::move(newLiteralExpression);
}

void CCodeAdapter::visit(SymbolExpression &node) {
	Symbol &oldSymbol = node.getSymbol();
	SymbolSource source = SymbolSource::Unknown;
	for (auto it = scopeStack.rbegin(); it != scopeStack.rend(); it++) {
		source = (*it)->getSymbolSource(oldSymbol.s.contents);
		if (source != SymbolSource::Unknown) {
			break;
		}
	}
	if (source == SymbolSource::FunctionParameter) {
		generatedStrings->push_back(
		      "CANYON_PARAMETER_" + std::string(oldSymbol.s.contents));
	} else {
		generatedStrings->push_back("CANYON_LOCAL_" + std::string(oldSymbol.s.contents));
	}
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

	oldBlock.forEachSymbol([&newBlockExpression](std::string_view symbol, int typeID,
	                             SymbolSource source) {
		newBlockExpression->pushSymbol(symbol, typeID, source);
	});

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
	newBlockExpression->getSlice().source = oldBlock.getSlice().source;
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

void CCodeAdapter::visit(IfElseExpression &node) {
	if (node.getTypeID() != inputModule->getType("()").id
	      && node.getTypeID() != inputModule->getType("!").id) {
		generatedStrings->push_back("CANYON_IFELSE_" + std::to_string(blockCount++));
		std::string_view tempVariableName = generatedStrings->back();
		std::unique_ptr<LetStatement> declaration = std::make_unique<LetStatement>(
		      std::make_unique<Symbol>(
		            Slice(tempVariableName, inputModule->getSource(), 0, 0)),
		      nullptr);
		scopeStack.back()->pushSymbol(tempVariableName, node.getTypeID(),
		      SymbolSource::GENERATED_IfElse);
		declaration->setSymbolTypeID(node.getTypeID());
		scopeStack.back()->pushStatement(std::move(declaration));
		blockTemporaryVariables.push(tempVariableName);

		Expression &oldCondition = node.getCondition();
		oldCondition.accept(*this);
		std::unique_ptr<Expression> newCondition = std::unique_ptr<Expression>(
		      dynamic_cast<Expression *>(returnValue.release()));

		Expression &oldThenExpression = node.getThenBlock();
		std::unique_ptr<BlockExpression> newThenBlock
		      = std::make_unique<BlockExpression>();
		scopeStack.push_back(newThenBlock.get());
		visitExpression(oldThenExpression);
		scopeStack.pop_back();
		std::unique_ptr<Expression> newFinalExpression = std::unique_ptr<Expression>(
		      dynamic_cast<Expression *>(returnValue.release()));
		Punctuation equalSign = Punctuation(Slice("=", inputModule->getSource(), 0, 0),
		      Punctuation::Type::Equals);
		std::unique_ptr<Operator> assignmentOperator
		      = std::make_unique<Operator>(equalSign, Operator::Type::Assignment);
		std::unique_ptr<BinaryExpression> assignment
		      = std::make_unique<BinaryExpression>(std::move(assignmentOperator),
		            std::make_unique<SymbolExpression>(std::make_unique<Symbol>(
		                  Slice(tempVariableName, inputModule->getSource(), 0, 0))),
		            std::move(newFinalExpression));
		std::unique_ptr<ExpressionStatement> newAssignment
		      = std::make_unique<ExpressionStatement>(std::move(assignment));
		newThenBlock->pushStatement(std::move(newAssignment));

		std::unique_ptr<IfElseExpression> newIfElseExpression;
		Expression *oldElseExpression = node.getElseExpression();
		if (oldElseExpression != nullptr) {
			std::unique_ptr<BlockExpression> newElseBlock
			      = std::make_unique<BlockExpression>();
			scopeStack.push_back(newElseBlock.get());
			visitExpression(*oldElseExpression);
			scopeStack.pop_back();
			std::unique_ptr<Expression> newFinalExpression = std::unique_ptr<Expression>(
			      dynamic_cast<Expression *>(returnValue.release()));
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
			newElseBlock->pushStatement(std::move(newAssignment));
			newIfElseExpression
			      = std::make_unique<IfElseExpression>(std::move(newCondition),
			            std::move(newThenBlock), std::move(newElseBlock));
		} else {
			newIfElseExpression = std::make_unique<IfElseExpression>(
			      std::move(newCondition), std::move(newThenBlock), nullptr);
		}
		blockTemporaryVariables.pop();
		newIfElseExpression->setTypeID(node.getTypeID());

		std::unique_ptr<ExpressionStatement> ifElseExpressionStatement
		      = std::make_unique<ExpressionStatement>(std::move(newIfElseExpression));
		scopeStack.back()->pushStatement(std::move(ifElseExpressionStatement));
		returnValue = std::make_unique<SymbolExpression>(std::make_unique<Symbol>(
		      Slice(tempVariableName, inputModule->getSource(), 0, 0)));
	} else {
		Expression &oldCondition = node.getCondition();
		Expression &oldThenBlock = node.getThenBlock();
		Expression *oldElseExpression = node.getElseExpression();
		visitExpression(oldCondition);
		std::unique_ptr<Expression> newCondition = std::unique_ptr<Expression>(
		      dynamic_cast<Expression *>(returnValue.release()));
		visitExpression(oldThenBlock);
		std::unique_ptr<BlockExpression> newThenBlock = std::unique_ptr<BlockExpression>(
		      dynamic_cast<BlockExpression *>(returnValue.release()));
		std::unique_ptr<IfElseExpression> newIfElseExpression;
		if (oldElseExpression != nullptr) {
			visitExpression(*oldElseExpression);
			std::unique_ptr<Expression> newElseExpression = std::unique_ptr<Expression>(
			      dynamic_cast<Expression *>(returnValue.release()));
			newIfElseExpression
			      = std::make_unique<IfElseExpression>(std::move(newCondition),
			            std::move(newThenBlock), std::move(newElseExpression));
		} else {
			newIfElseExpression = std::make_unique<IfElseExpression>(
			      std::move(newCondition), std::move(newThenBlock), nullptr);
		}
		newIfElseExpression->setTypeID(node.getTypeID());
		returnValue = std::move(newIfElseExpression);
	}
}

void CCodeAdapter::visit(WhileExpression &node) {
	if (node.getTypeID() != inputModule->getType("()").id
	      && node.getTypeID() != inputModule->getType("!").id) {
		generatedStrings->push_back("CANYON_WHILE_" + std::to_string(blockCount++));
		std::string_view tempVariableName = generatedStrings->back();
		std::unique_ptr<LetStatement> declaration = std::make_unique<LetStatement>(
		      std::make_unique<Symbol>(
		            Slice(tempVariableName, inputModule->getSource(), 0, 0)),
		      nullptr);
		scopeStack.back()->pushSymbol(tempVariableName, node.getTypeID(),
		      SymbolSource::GENERATED_While);
		declaration->setSymbolTypeID(node.getTypeID());
		scopeStack.back()->pushStatement(std::move(declaration));
		blockTemporaryVariables.push(tempVariableName);

		Expression &oldCondition = node.getCondition();
		oldCondition.accept(*this);
		std::unique_ptr<Expression> newCondition = std::unique_ptr<Expression>(
		      dynamic_cast<Expression *>(returnValue.release()));

		Expression &oldBlock = node.getBody();
		std::unique_ptr<BlockExpression> newBlock = std::make_unique<BlockExpression>();
		scopeStack.push_back(newBlock.get());
		visitExpression(oldBlock);
		scopeStack.pop_back();
		std::unique_ptr<Expression> newFinalExpression = std::unique_ptr<Expression>(
		      dynamic_cast<Expression *>(returnValue.release()));
		Punctuation equalSign = Punctuation(Slice("=", inputModule->getSource(), 0, 0),
		      Punctuation::Type::Equals);
		std::unique_ptr<Operator> assignmentOperator
		      = std::make_unique<Operator>(equalSign, Operator::Type::Assignment);
		std::unique_ptr<BinaryExpression> assignment
		      = std::make_unique<BinaryExpression>(std::move(assignmentOperator),
		            std::make_unique<SymbolExpression>(std::make_unique<Symbol>(
		                  Slice(tempVariableName, inputModule->getSource(), 0, 0))),
		            std::move(newFinalExpression));
		std::unique_ptr<ExpressionStatement> newAssignment
		      = std::make_unique<ExpressionStatement>(std::move(assignment));
		newBlock->pushStatement(std::move(newAssignment));
		blockTemporaryVariables.pop();
		std::unique_ptr<WhileExpression> newWhileExpression
		      = std::make_unique<WhileExpression>(std::move(newCondition),
		            std::move(newBlock));
		newWhileExpression->setTypeID(node.getTypeID());

		std::unique_ptr<ExpressionStatement> whileExpressionStatement
		      = std::make_unique<ExpressionStatement>(std::move(newWhileExpression));
		scopeStack.back()->pushStatement(std::move(whileExpressionStatement));
		returnValue = std::make_unique<SymbolExpression>(std::make_unique<Symbol>(
		      Slice(tempVariableName, inputModule->getSource(), 0, 0)));
	} else {
		Expression &oldCondition = node.getCondition();
		Expression &oldBlock = node.getBody();
		visitExpression(oldCondition);
		std::unique_ptr<Expression> newCondition = std::unique_ptr<Expression>(
		      dynamic_cast<Expression *>(returnValue.release()));
		visitExpression(oldBlock);
		std::unique_ptr<BlockExpression> newBlock = std::unique_ptr<BlockExpression>(
		      dynamic_cast<BlockExpression *>(returnValue.release()));
		std::unique_ptr<WhileExpression> newWhileExpression
		      = std::make_unique<WhileExpression>(std::move(newCondition),
		            std::move(newBlock));
		newWhileExpression->setTypeID(node.getTypeID());
		returnValue = std::move(newWhileExpression);
	}
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
	std::vector<std::pair<std::unique_ptr<Symbol>, std::unique_ptr<Symbol>>>
	      newParameters;
	node.forEachParameter([this, &newParameters](Symbol &parameter, Symbol &type) {
		generatedStrings->push_back(
		      "CANYON_PARAMETER_" + std::string(parameter.s.contents));
		std::string_view newParameterName = generatedStrings->back();
		std::unique_ptr<Symbol> newParameter = std::make_unique<Symbol>(
		      Slice(newParameterName, inputModule->getSource(), 0, 0));
		std::unique_ptr<Symbol> newType = std::make_unique<Symbol>(type);
		newParameters.emplace_back(std::move(newParameter), std::move(newType));
	});

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
	returnValue = std::make_unique<Function>(std::move(newParameters),
	      std::move(enclosingScope));
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
	// TODO(#11) move this logic into visit BlockExpression
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
		scopeStack.back()->pushSymbol(tempVariableName, node.getTypeID(),
		      SymbolSource::GENERATED_Block);
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
