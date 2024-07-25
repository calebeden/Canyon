#include "semanticanalyzer.h"

#include "ast.h"
#include "errorhandler.h"

#include <array>
#include <initializer_list>
#include <memory>
#include <span>
#include <string>
#include <utility>
#include <vector>

static void addDefaultOperators(Module *module);
static void addDefaultIntegerOperators(Module *module);
static void addSameSignIntegerTypes(Module *module,
      std::span<const std::string_view> types,
      std::span<const Operator::Type> binaryOperators,
      std::span<const Operator::Type> unaryOperators);

SemanticAnalyzer::SemanticAnalyzer(std::unique_ptr<Module> module,
      ErrorHandler *errorHandler)
    : module(std::move(module)), errorHandler(errorHandler) {
}

std::unique_ptr<Module> SemanticAnalyzer::analyze() {
	visit(*module);
	return std::move(module);
}

void SemanticAnalyzer::visit(FunctionCallExpression &node) {
	Expression &functionCall = node.getFunction();
	auto *symbol = dynamic_cast<SymbolExpression *>(&functionCall);
	if (symbol == nullptr) {
		errorHandler->error(functionCall.getSlice(),
		      "Function call target is not a symbol");
		return;
	}
	Function *function = module->getFunction(symbol->getSymbol().s.contents);
	if (function == nullptr) {
		errorHandler->error(symbol->getSymbol().s,
		      "Function " + std::string(symbol->getSymbol().s.contents) + " not found");
		return;
	}
	node.setTypeID(function->getTypeID());
}

void SemanticAnalyzer::visit(BinaryExpression &node) {
	Expression &left = node.getLeft();
	Expression &right = node.getRight();
	left.accept(*this);
	if (inUnreachableCode) {
		errorHandler->error(node.getOperator().s, "Unreachable code");
		return;
	}
	right.accept(*this);
	if (inUnreachableCode) {
		errorHandler->error(node.getOperator().s, "Unreachable code");
		return;
	}
	if (node.getOperator().type == Operator::Type::Assignment
	      && (dynamic_cast<SymbolExpression *>(&left) == nullptr)) {
		errorHandler->error(left.getSlice(),
		      "Left side of assignment must be a variable");
		return;
	}
	if (left.getTypeID() == -1 || right.getTypeID() == -1) {
		return;
	}
	int typeID = module->getBinaryOperator(node.getOperator().type, left.getTypeID(),
	      right.getTypeID());
	if (typeID == -1) {
		errorHandler->error(node.getOperator().s,
		      "Binary operator not defined for types");
	}
	node.setTypeID(typeID);
}

void SemanticAnalyzer::visit(UnaryExpression &node) {
	Expression &operand = node.getExpression();
	operand.accept(*this);
	if (inUnreachableCode) {
		errorHandler->error(node.getOperator().s, "Unreachable code");
		return;
	}
	int typeID = module->getUnaryOperator(node.getOperator().type, operand.getTypeID());
	if (typeID == -1) {
		errorHandler->error(node.getOperator().s, "Unary operator not defined for type");
	}
	node.setTypeID(typeID);
}

void SemanticAnalyzer::visit(LiteralExpression &node) {
	IntegerLiteral &literal = node.getLiteral();
	node.setTypeID(module->getType(IntegerLiteral::typeToStringView(literal.type)).id);
}

void SemanticAnalyzer::visit(SymbolExpression &node) {
	for (auto it = scopeStack.rbegin(); it != scopeStack.rend(); it++) {
		int typeID = (*it)->getSymbolType(node.getSymbol().s.contents);
		if (typeID != -1) {
			node.setTypeID(typeID);
			return;
		}
	}
	errorHandler->error(node.getSymbol(),
	      "Symbol " + std::string(node.getSymbol().s.contents) + " not found");
}

void SemanticAnalyzer::visit(BlockExpression &node) {
	scopeStack.push_back(&node);
	node.forEachStatement([this](Statement &statement) {
		if (inUnreachableCode) {
			errorHandler->error(statement.getSlice(), "Unreachable code");
			return;
		}
		statement.accept(*this);
	});
	Expression *finalExpression = node.getFinalExpression();
	if (inUnreachableCode) {
		if (finalExpression != nullptr) {
			errorHandler->error(finalExpression->getSlice(), "Unreachable code");
		}
		node.setTypeID(module->getType("!").id);
	} else {
		if (finalExpression != nullptr) {
			finalExpression->accept(*this);
			node.setTypeID(finalExpression->getTypeID());
		} else {
			node.setTypeID(module->getType("()").id);
		}
	}
	scopeStack.pop_back();
}

void SemanticAnalyzer::visit(ReturnExpression &node) {
	Expression *expr = node.getExpression();
	if (expr != nullptr) {
		expr->accept(*this);
		if (inUnreachableCode) {
			errorHandler->error(node.getSlice(), "Unreachable code");
			return;
		}
		if (currentFunction->getTypeID() != expr->getTypeID()) {
			errorHandler->error(expr->getSlice(),
			      "Return type does not match function return type");
		}
	} else if (currentFunction->getTypeID() != module->getType("()").id) {
		errorHandler->error(node.getSlice(),
		      "Return type does not match function return type");
	}
	node.setTypeID(module->getType("!").id);
	inUnreachableCode = true;
}

void SemanticAnalyzer::visit(ParenthesizedExpression &node) {
	Expression &expr = node.getExpression();
	expr.accept(*this);
	node.setTypeID(expr.getTypeID());
}

void SemanticAnalyzer::visit(ExpressionStatement &node) {
	if (inUnreachableCode) {
		errorHandler->error(node.getSlice(), "Unreachable code");
		return;
	}
	node.getExpression().accept(*this);
}

void SemanticAnalyzer::visit(LetStatement &node) {
	if (inUnreachableCode) {
		errorHandler->error(node.getSlice(), "Unreachable code");
		return;
	}
	if (scopeStack.back()->getSymbolType(node.getSymbol().s.contents) != -1) {
		errorHandler->error(node.getSymbol(),
		      "Redefinition of variable " + std::string(node.getSymbol().s.contents));
		return;
	}
	Symbol *typeAnnotation = node.getTypeAnnotation();
	if (!typeAnnotation) {
		errorHandler->error(node.getEqualSign(), "Expected type annotation");
		return;
	}
	int typeID = module->getType(typeAnnotation->s.contents).id;
	Expression *value = node.getExpression();
	if (!value) {
		errorHandler->error(node.getSlice(), "Expression required for let statement");
		return;
	}
	value->accept(*this);
	if (inUnreachableCode) {
		errorHandler->error(node.getEqualSign().s, "Unreachable code");
		return;
	}
	scopeStack.back()->setSymbolType(node.getSymbol().s.contents, typeID);
	if (typeID != value->getTypeID() && value->getTypeID() != -1) {
		errorHandler->error(node.getExpression()->getSlice(),
		      "Type mismatch in let statement");
	}
	node.setSymbolTypeID(typeID);
}

void SemanticAnalyzer::visit(Function &node) {
	node.getBody().accept(*this);
	int blockTypeID = node.getBody().getTypeID();
	if (blockTypeID == -1) {
		// We have already reported a problem, no need to report another
		inUnreachableCode = false;
		return;
	}
	Symbol *typeAnnotation = node.getReturnTypeAnnotation();
	int annotationTypeID = -1;
	if (typeAnnotation != nullptr) {
		annotationTypeID = module->getType(typeAnnotation->s.contents).id;
	} else {
		annotationTypeID = module->getType("()").id;
	}
	if (!module->isTypeConvertible(blockTypeID, annotationTypeID)) {
		Expression *finalExpression = node.getBody().getFinalExpression();
		if (finalExpression != nullptr) {
			errorHandler->error(finalExpression->getSlice(),
			      "Function body block expression type does not match function "
			      "return type");
		} else if (typeAnnotation != nullptr) {
			errorHandler->error(typeAnnotation->s, "Function body block expression type "
			                                       "does not match function return type");
		} else {
			errorHandler->error(node.getBody().getSlice(),
			      "Function body block expression type does not match function "
			      "return type");
		}
	}
	inUnreachableCode = false;
}

void SemanticAnalyzer::visit(Module &node) {
	addDefaultOperators(&node);
	node.forEachFunction([this]([[maybe_unused]]
	                            std::string_view name,
	                           Function &function) {
		Symbol *type = function.getReturnTypeAnnotation();
		if (type == nullptr) {
			function.setTypeID(module->getType("()").id);
		} else {
			function.setTypeID(module->getType(type->s.contents).id);
		}
	});
	bool hasMain = false;
	node.forEachFunction([this, &hasMain](std::string_view name, Function &function) {
		currentFunction = &function;
		function.accept(*this);
		if (name == "main") {
			hasMain = true;
			if (function.getTypeID() != module->getType("()").id) {
				errorHandler->error(*function.getReturnTypeAnnotation(),
				      "main must return unit type");
			}
		}
	});
	if (!hasMain) {
		errorHandler->error(module->getSource(), "No main function");
	}
}

static void addDefaultOperators(Module *module) {
	addDefaultIntegerOperators(module);
}

static void addDefaultIntegerOperators(Module *module) {
	const std::array unaryIntegerOperators = {
	      Operator::Type::Addition,
	      Operator::Type::Subtraction,
	      Operator::Type::BitwiseNot,
	};
	const std::array binaryIntegerOperators = {
	      Operator::Type::Addition,
	      Operator::Type::Subtraction,
	      Operator::Type::Multiplication,
	      Operator::Type::Division,
	      Operator::Type::Modulus,
	      Operator::Type::BitwiseAnd,
	      Operator::Type::BitwiseOr,
	      Operator::Type::BitwiseXor,
	      Operator::Type::BitwiseShiftLeft,
	      Operator::Type::BitwiseShiftRight,
	      Operator::Type::Equality,
	      Operator::Type::Inequality,
	      Operator::Type::LessThan,
	      Operator::Type::LessThanOrEqual,
	      Operator::Type::GreaterThan,
	      Operator::Type::GreaterThanOrEqual,
	};
	const std::array<std::string_view, 4> signedIntegerTypes = {
	      "i8",
	      "i16",
	      "i32",
	      "i64",
	};
	const std::array<std::string_view, 4> unsignedIntegerTypes = {
	      "u8",
	      "u16",
	      "u32",
	      "u64",
	};

	addSameSignIntegerTypes(module, signedIntegerTypes, binaryIntegerOperators,
	      unaryIntegerOperators);
	addSameSignIntegerTypes(module, unsignedIntegerTypes, binaryIntegerOperators,
	      unaryIntegerOperators);
}

static void addSameSignIntegerTypes(Module *module,
      const std::span<const std::string_view> types,
      const std::span<const Operator::Type> binaryOperators,
      const std::span<const Operator::Type> unaryOperators) {
	static const int unitTypeID = module->getType("()").id;
	for (auto it = types.begin(); it != types.end(); it++) {
		int typeID = module->getType(*it).id;
		for (const auto &op : unaryOperators) {
			module->addUnaryOperator(op, typeID, typeID);
		}
		for (const auto &op : binaryOperators) {
			module->addBinaryOperator(op, typeID, typeID, typeID);
		}
		module->addBinaryOperator(Operator::Type::Assignment, typeID, typeID, unitTypeID);
	}
}
