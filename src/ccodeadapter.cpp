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
	auto *oldPath = dynamic_cast<PathExpression *>(&oldFunction);
	auto *oldFieldAccess = dynamic_cast<FieldAccessExpression *>(&oldFunction);
	if (oldSymbol == nullptr && oldPath == nullptr && oldFieldAccess == nullptr) {
		std::cerr << "Function call target is not a symbol, path, or method" << std::endl;
		exit(EXIT_FAILURE);
	}
	std::unique_ptr<Expression> newWhichFunction = nullptr;
	if (oldSymbol != nullptr) {
		generatedStrings->push_back(
		      "CANYON_FUNCTION_" + std::string(oldSymbol->getSymbol().s.contents));
		std::string_view newName = generatedStrings->back();
		std::unique_ptr<Symbol> newSymbol
		      = std::make_unique<Symbol>(Slice(newName, inputModule->getSource(), 0, 0));
		newWhichFunction = std::make_unique<SymbolExpression>(std::move(newSymbol));
	} else if (oldPath != nullptr) {
		std::string functionName;
		bool first = true;
		oldPath->forEachSymbol([&functionName, &first](SymbolExpression &symbol) {
			functionName += symbol.getSymbol().s.contents;
			if (first) {
				functionName += "_METHOD_";
			}
			first = false;
		});
		generatedStrings->push_back("CANYON_IMPL_" + functionName);
		std::string_view newName = generatedStrings->back();
		std::unique_ptr<Symbol> newSymbol
		      = std::make_unique<Symbol>(Slice(newName, inputModule->getSource(), 0, 0));
		newWhichFunction = std::make_unique<SymbolExpression>(std::move(newSymbol));
	} else {
		oldFieldAccess->accept(*this);
		newWhichFunction = std::unique_ptr<FieldAccessExpression>(
		      dynamic_cast<FieldAccessExpression *>(returnValue.release()));
	}

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
	      = std::make_unique<FunctionCallExpression>(std::move(newWhichFunction),
	            std::move(newArguments));
	newFunctionCall->setTypeID(node.getTypeID());
	newFunctionCall->setVariant(node.getVariant());
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

void CCodeAdapter::visit(CharacterLiteralExpression &node) {
	CharacterLiteral &previousLiteral = node.getLiteral();
	std::unique_ptr<CharacterLiteral> newLiteral
	      = std::make_unique<CharacterLiteral>(previousLiteral, previousLiteral.value);
	std::unique_ptr<CharacterLiteralExpression> newLiteralExpression
	      = std::make_unique<CharacterLiteralExpression>(std::move(newLiteral));
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

void CCodeAdapter::visit([[maybe_unused]] PathExpression &node) {
	// TODO
}

void CCodeAdapter::visit(FieldAccessExpression &node) {
	node.getObject().accept(*this);
	std::unique_ptr<Expression> newObject = std::unique_ptr<Expression>(
	      dynamic_cast<Expression *>(returnValue.release()));
	// don't want to visit the field because otherwise it will get mangled; instead just
	// make a new one with the same name
	generatedStrings->push_back(std::string(node.getField().getSymbol().s.contents));
	std::unique_ptr<SymbolExpression> newField
	      = std::make_unique<SymbolExpression>(std::make_unique<Symbol>(
	            Slice(generatedStrings->back(), inputModule->getSource(), 0, 0)));
	std::unique_ptr<FieldAccessExpression> newFieldAccessExpression
	      = std::make_unique<FieldAccessExpression>(std::move(newObject),
	            std::move(newField));
	newFieldAccessExpression->setTypeID(node.getTypeID());
	returnValue = std::move(newFieldAccessExpression);
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
	size_t i = 0;
	int typeId = -1;
	node.forEachParameter(
	      [this, &newParameters, &i, &node, &typeId](Symbol &parameter, Symbol &type) {
		      generatedStrings->push_back(
		            "CANYON_PARAMETER_" + std::string(parameter.s.contents));
		      std::string_view newParameterName = generatedStrings->back();
		      std::unique_ptr<Symbol> newParameter = std::make_unique<Symbol>(
		            Slice(newParameterName, inputModule->getSource(), 0, 0));
		      std::unique_ptr<Symbol> newType = std::make_unique<Symbol>(type);
		      newParameters.emplace_back(std::move(newParameter), std::move(newType));
		      if (node.getVariant() == FunctionVariant::CONSTRUCTOR && i == 0) {
			      typeId = node.getBody().getSymbolType(parameter.s.contents);
		      }
		      i++;
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

	if (node.getVariant() == FunctionVariant::CONSTRUCTOR) {
		std::unique_ptr<Symbol> selfParameter = std::make_unique<Symbol>(
		      Slice("CANYON_PARAMETER_self", inputModule->getSource(), 0, 0));
		std::unique_ptr<SymbolExpression> selfSymbolExpression
		      = std::make_unique<SymbolExpression>(std::move(selfParameter));
		std::unique_ptr<ReturnExpression> returnExpression
		      = std::make_unique<ReturnExpression>(std::move(selfSymbolExpression));
		std::unique_ptr<ExpressionStatement> returnStatement
		      = std::make_unique<ExpressionStatement>(std::move(returnExpression));
		enclosingScope->pushStatement(std::move(returnStatement));
	}

	std::unique_ptr<Function> newFunction = std::make_unique<Function>(
	      std::move(newParameters), std::move(enclosingScope), node.getVariant());
	if (node.getVariant() == FunctionVariant::CONSTRUCTOR) {
		newFunction->setTypeID(typeId);
	}
	returnValue = std::move(newFunction);
}

void CCodeAdapter::visit(Class &node) {
	std::vector<std::unique_ptr<LetStatement>> newFieldDeclarations;
	node.forEachFieldDeclaration([this, &newFieldDeclarations](
	                                   LetStatement &declaration) {
		declaration.accept(*this);
		std::unique_ptr<LetStatement> newDeclaration = std::unique_ptr<LetStatement>(
		      dynamic_cast<LetStatement *>(returnValue.release()));
		generatedStrings->push_back(std::string(declaration.getSymbol().s.contents));
		std::string_view newName = generatedStrings->back();
		newDeclaration->getSymbol().s.contents = newName;
		newFieldDeclarations.push_back(std::move(newDeclaration));
	});
	returnValue = std::make_unique<Class>(std::move(newFieldDeclarations));
}

void CCodeAdapter::visit(Impl &node) {
	std::unordered_map<std::string_view, std::unique_ptr<Function>> newMethods;
	node.forEachMethod([this, &newMethods](std::string_view name, Function &method) {
		generatedStrings->push_back(std::string(name));
		std::string_view newName = generatedStrings->back();
		method.accept(*this);
		std::unique_ptr<Function> newMethod = std::unique_ptr<Function>(
		      dynamic_cast<Function *>(returnValue.release()));
		if (method.getVariant() != FunctionVariant::CONSTRUCTOR) {
			newMethod->setTypeID(method.getTypeID());
		}
		newMethods.emplace(newName, std::move(newMethod));
	});
	returnValue = std::make_unique<Impl>(std::move(newMethods));
}

void CCodeAdapter::visit(Module &node) {
	node.forEachFunction([this](std::string_view name, Function &oldFunction,
	                           bool isBuiltin) {
		generatedStrings->push_back("CANYON_FUNCTION_" + std::string(name));
		std::string_view newName = generatedStrings->back();
		oldFunction.accept(*this);
		std::unique_ptr<Function> newFunction = std::unique_ptr<Function>(
		      dynamic_cast<Function *>(returnValue.release()));
		if (oldFunction.getVariant() != FunctionVariant::CONSTRUCTOR) {
			newFunction->setTypeID(oldFunction.getTypeID());
		}
		outputModule->addFunction(
		      std::make_unique<Symbol>(Slice(newName, inputModule->getSource(), 0, 0)),
		      std::move(newFunction), isBuiltin);
	});
	node.forEachClass([this](std::string_view name, Class &oldClass, bool isBuiltin) {
		generatedStrings->push_back(std::string(name));
		std::string_view newName = generatedStrings->back();
		currentClassName = &generatedStrings->back();
		oldClass.accept(*this);
		std::unique_ptr<Class> newClass
		      = std::unique_ptr<Class>(dynamic_cast<Class *>(returnValue.release()));
		outputModule->addClass(
		      std::make_unique<Symbol>(Slice(newName, inputModule->getSource(), 0, 0)),
		      std::move(newClass), isBuiltin);
	});
	node.forEachImpl([this](std::string_view name, Impl &oldImpl, bool isBuiltin) {
		generatedStrings->push_back(std::string(name));
		std::string_view newName = generatedStrings->back();
		oldImpl.accept(*this);
		std::unique_ptr<Impl> newImpl
		      = std::unique_ptr<Impl>(dynamic_cast<Impl *>(returnValue.release()));
		outputModule->addImpl(
		      std::make_unique<Symbol>(Slice(newName, inputModule->getSource(), 0, 0)),
		      std::move(newImpl), isBuiltin);
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
