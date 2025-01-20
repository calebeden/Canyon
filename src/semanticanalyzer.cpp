#include "semanticanalyzer.h"

#include "ast.h"
#include "errorhandler.h"
#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include <array>
#include <initializer_list>
#include <memory>
#include <span>
#include <string>
#include <utility>
#include <vector>

static void addDefaultOperators(Module *module);
static void addDefaultIntegerOperators(Module *module);
static void addSameSignIntegerArithmeticTypes(Module *module,
      std::span<const std::string_view> types,
      std::span<const Operator::Type> binaryOperators,
      std::span<const Operator::Type> unaryOperators);
static void addSameSignIntegerComparisonTypes(Module *module,
      std::span<const std::string_view> types,
      std::span<const Operator::Type> comparisonOperators);
static void addDefaultBoolOperators(Module *module);
static void addRuntimeFunctions(Module *module, std::istream &builtinApiJsonFile);

SemanticAnalyzer::SemanticAnalyzer(Module *module, ErrorHandler *errorHandler,
      std::istream &builtinApiJsonFile)
    : module(module), errorHandler(errorHandler), builtinApiJsonFile(builtinApiJsonFile) {
}

void SemanticAnalyzer::analyze() {
	visit(*module);
}

void SemanticAnalyzer::visit(FunctionCallExpression &node) {
	Expression &functionCall = node.getFunction();
	auto *symbol = dynamic_cast<SymbolExpression *>(&functionCall);
	auto *path = dynamic_cast<PathExpression *>(&functionCall);
	auto *fieldAccess = dynamic_cast<FieldAccessExpression *>(&functionCall);
	if (symbol == nullptr && path == nullptr && fieldAccess == nullptr) {
		errorHandler->error(functionCall.getSlice(),
		      "Function call target is not a symbol, path, or method");
		return;
	}
	Function *function = nullptr;
	if (symbol != nullptr) {
		function = module->getFunction(symbol->getSymbol().s.contents);
		if (function == nullptr) {
			errorHandler->error(symbol->getSymbol().s,
			      "Function " + std::string(symbol->getSymbol().s.contents)
			            + " not found");
			return;
		}
	} else if (path != nullptr) {
		Impl *impl = nullptr;
		bool pathLookupGood = true;
		size_t i = 0;
		path->forEachSymbol(
		      [this, &impl, &function, &pathLookupGood, &i](SymbolExpression &symbol) {
			      if (impl == nullptr && pathLookupGood) {
				      impl = module->getImpl(symbol.getSymbol().s.contents);
				      if (impl == nullptr) {
					      errorHandler->error(symbol.getSymbol().s,
					            "Impl " + std::string(symbol.getSymbol().s.contents)
					                  + " not found");
					      pathLookupGood = false;
					      return;
				      }
				      i++;
			      } else if (pathLookupGood) {
				      function = impl->getMethod(symbol.getSymbol().s.contents);
				      if (function == nullptr) {
					      errorHandler->error(symbol.getSymbol().s,
					            "Method " + std::string(symbol.getSymbol().s.contents)
					                  + " not found");
					      pathLookupGood = false;
					      return;
				      }
				      i++;
			      } else if (i >= 2) {
				      errorHandler->error(symbol.getSymbol().s, "Path too long");
				      pathLookupGood = false;
			      }
		      });
		if (!pathLookupGood) {
			return;
		}
	} else {
		FieldAccessExpression &fieldAccessExpr = *fieldAccess;
		fieldAccessExpr.getObject().accept(*this);
		if (inUnreachableCode) {
			errorHandler->error(fieldAccessExpr.getSlice(), "Unreachable code");
			return;
		}
		int objectTypeId = fieldAccessExpr.getObject().getTypeID();
		if (objectTypeId == -1) {
			return;
		}
		std::pair<std::string_view, Impl *> impl = module->getImpl(objectTypeId);
		if (impl.second == nullptr) {
			errorHandler->error(fieldAccessExpr.getSlice(),
			      "Field access on non-class type");
			return;
		}
		function
		      = impl.second->getMethod(fieldAccessExpr.getField().getSymbol().s.contents);
		if (function == nullptr) {
			errorHandler->error(fieldAccessExpr.getField().getSymbol().s,
			      "Method "
			            + std::string(fieldAccessExpr.getField().getSymbol().s.contents)
			            + " not found in impl for " + std::string(impl.first));
			return;
		}
	}
	std::vector<std::pair<Symbol &, int>> parameters;
	function->forEachParameter(
	      [&parameters, &function](Symbol &parameter, [[maybe_unused]]
	                                                  Symbol &type) {
		      int typeId = function->getBody().getSymbolType(parameter.s.contents);
		      parameters.emplace_back(parameter, typeId);
	      });

	size_t i = 0;
	bool unreachableArgument = false;
	bool unreachableHandled = false;
	FunctionVariant variant = function->getVariant();
	if (variant == FunctionVariant::CONSTRUCTOR || variant == FunctionVariant::METHOD) {
		// Implicit self argument
		i++;
	}
	node.forEachArgument([this, &parameters, &i, &unreachableArgument,
	                           &unreachableHandled](Expression &argument) {
		if (i == parameters.size()) {
			errorHandler->error(argument.getSlice(), "Incorrect number of arguments");
			i++;
			return;
		}
		if (i > parameters.size()) {
			return;
		}
		if (unreachableArgument) {
			i++;
			if (!unreachableHandled) {
				errorHandler->error(argument.getSlice(), "Unreachable argument");
				unreachableHandled = true;
			}
			return;
		}
		argument.accept(*this);
		if (inUnreachableCode) {
			unreachableArgument = true;
			i++;
			return;
		}
		if (!module->isTypeConvertible(argument.getTypeID(), parameters[i].second)) {
			errorHandler->error(argument.getSlice(),
			      "Argument type is not convertible to parameter type");
		}
		i++;
	});
	if (i < parameters.size()) {
		errorHandler->error(node.getSlice(), "Incorrect number of arguments");
	}
	if (unreachableArgument) {
		if (!unreachableHandled) {
			errorHandler->error(node.getSlice(), "Unreachable function call");
		}
		node.setTypeID(module->getType("!").id);
		return;
	}

	node.setVariant(function->getVariant());
	if (function->getVariant() == FunctionVariant::CONSTRUCTOR) {
		node.setTypeID(parameters[0].second);
	} else {
		node.setTypeID(function->getTypeID());
	}
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
	      && (dynamic_cast<SymbolExpression *>(&left) == nullptr)
	      && (dynamic_cast<FieldAccessExpression *>(&left) == nullptr)) {
		errorHandler->error(left.getSlice(),
		      "Left side of assignment must be a variable or field access");
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

void SemanticAnalyzer::visit(IntegerLiteralExpression &node) {
	IntegerLiteral &literal = node.getLiteral();
	node.setTypeID(module->getType(IntegerLiteral::typeToStringView(literal.type)).id);
}

void SemanticAnalyzer::visit(BoolLiteralExpression &node) {
	node.setTypeID(module->getType("bool").id);
}

void SemanticAnalyzer::visit(CharacterLiteralExpression &node) {
	node.setTypeID(module->getType("char").id);
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

void SemanticAnalyzer::visit([[maybe_unused]] PathExpression &node) {
	// TODO
}

void SemanticAnalyzer::visit(FieldAccessExpression &node) {
	node.getObject().accept(*this);
	if (inUnreachableCode) {
		errorHandler->error(node.getSlice(), "Unreachable code");
		return;
	}
	int objectTypeID = node.getObject().getTypeID();
	if (objectTypeID == -1) {
		return;
	}
	std::pair<std::string_view, Class *> cls = module->getClass(objectTypeID);
	if (cls.second == nullptr) {
		errorHandler->error(node.getSlice(), "Field access on non-class type");
		return;
	}
	cls.second->getScope().forEachSymbol([this, &node, &cls](std::string_view fieldName,
	                                           int /*unused*/, SymbolSource /*unused*/) {
		if (fieldName == node.getField().getSymbol().s.contents) {
			node.setTypeID(cls.second->getScope().getSymbolType(fieldName));
		}
	});
}

void SemanticAnalyzer::visit(IfElseExpression &node) {
	Expression &condition = node.getCondition();
	condition.accept(*this);
	if (inUnreachableCode) {
		errorHandler->error(node.getThenBlock().getSlice(), "Unreachable code");
		return;
	}
	if (condition.getTypeID() != module->getType("bool").id) {
		errorHandler->error(condition.getSlice(), "Condition is not of type bool");
	}
	BlockExpression &thenBlock = node.getThenBlock();
	thenBlock.accept(*this);
	inUnreachableCode = false;
	int thenTypeID = thenBlock.getTypeID();
	Expression *elseExpression = node.getElseExpression();
	int elseTypeID = -1;
	if (elseExpression == nullptr) {
		elseTypeID = module->getType("()").id;
	} else {
		elseExpression->accept(*this);
		inUnreachableCode = false;
		elseTypeID = elseExpression->getTypeID();
	}
	if (thenTypeID == -1 || elseTypeID == -1) {
		return;
	}
	int typeID = module->getCommonTypeAncestor(thenTypeID, elseTypeID).id;
	if (typeID == -1) {
		errorHandler->error(node.getSlice(),
		      "If and else block types are not convertible");
	}
	if (typeID == module->getType("!").id) {
		inUnreachableCode = true;
	}
	node.setTypeID(typeID);
}

void SemanticAnalyzer::visit(WhileExpression &node) {
	Expression &condition = node.getCondition();
	condition.accept(*this);
	if (inUnreachableCode) {
		errorHandler->error(node.getBody().getSlice(), "Unreachable code");
		return;
	}
	if (condition.getTypeID() != module->getType("bool").id) {
		errorHandler->error(condition.getSlice(), "Condition is not of type bool");
	}
	BlockExpression &block = node.getBody();
	block.accept(*this);
	node.setTypeID(block.getTypeID());
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
	if (!node.getIsFieldDeclaration()) {
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
		scopeStack.back()->pushSymbol(node.getSymbol().s.contents, typeID,
		      SymbolSource::LetStatement);
		if (typeID != value->getTypeID() && value->getTypeID() != -1) {
			errorHandler->error(node.getExpression()->getSlice(),
			      "Type mismatch in let statement");
		}
	} else {
		scopeStack.back()->pushSymbol(node.getSymbol().s.contents, typeID,
		      SymbolSource::FieldDeclaration);
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

void SemanticAnalyzer::visit(Class &node) {
	scopeStack.push_back(&node.getScope());
	node.forEachFieldDeclaration([this](LetStatement &declaration) {
		declaration.accept(*this);
	});
}

void SemanticAnalyzer::visit(Impl &node) {
	node.forEachMethod([this](std::string_view /*unused*/, Function &method) {
		method.accept(*this);
	});
}

void SemanticAnalyzer::visit(Module &node) {
	addDefaultOperators(&node);
	addRuntimeFunctions(&node, builtinApiJsonFile);
	node.forEachClass([this](std::string_view name, Class & /*unused*/, bool /*unused*/) {
		module->insertType(name, true);
	});
	node.forEachClass([this](std::string_view /*unused*/, Class &cls, bool isBuiltin) {
		if (isBuiltin) {
			return;
		}
		cls.accept(*this);
	});
	node.forEachFunction([this]([[maybe_unused]]
	                            std::string_view name,
	                           Function &function, bool /*unused*/) {
		function.forEachParameter([this, &function](Symbol &parameter, Symbol &type) {
			int typeID = module->getType(type.s.contents).id;
			if (typeID == -1) {
				errorHandler->error(type.s, "Unknown type");
				return;
			}
			function.getBody().pushSymbol(parameter.s.contents, typeID,
			      SymbolSource::FunctionParameter);
		});

		Symbol *type = function.getReturnTypeAnnotation();
		if (type == nullptr) {
			function.setTypeID(module->getType("()").id);
		} else {
			function.setTypeID(module->getType(type->s.contents).id);
		}
	});
	node.forEachImpl([this](std::string_view implName, Impl &impl, bool /*unused*/) {
		impl.forEachMethod([this, &implName](std::string_view /*unused*/,
		                         Function &method) {
			bool first = true;
			method.forEachParameter([this, &method, &first, &implName](Symbol &parameter,
			                              Symbol &type) {
				int typeID = module->getType(type.s.contents).id;
				if (typeID == -1) {
					errorHandler->error(type.s, "Unknown type");
					return;
				}
				if (first && method.getVariant() == FunctionVariant::CONSTRUCTOR
				      && parameter.s.contents != "self") {
					errorHandler->error(parameter.s,
					      "First parameter of constructor must be self");
				}
				if (first && method.getVariant() == FunctionVariant::CONSTRUCTOR
				      && type.s.contents != implName) {
					errorHandler->error(type.s, "Constructor self parameter type must be "
					                            "the same as impl name");
				}
				if (first && method.getVariant() == FunctionVariant::FUNCTION) {
					if (parameter.s.contents == "self") {
						method.setVariant(FunctionVariant::METHOD);
						if (type.s.contents != implName) {
							errorHandler->error(type.s, "Method self parameter type must "
							                            "be the same as impl name");
						}
					}
				}
				first = false;
				method.getBody().pushSymbol(parameter.s.contents, typeID,
				      SymbolSource::FunctionParameter);
			});

			Symbol *type = method.getReturnTypeAnnotation();
			if (type == nullptr) {
				method.setTypeID(module->getType("()").id);
			} else {
				method.setTypeID(module->getType(type->s.contents).id);
			}
		});
	});

	bool hasMain = false;
	node.forEachFunction(
	      [this, &hasMain](std::string_view name, Function &function, bool isBuiltin) {
		      if (isBuiltin) {
			      return;
		      }
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
	node.forEachImpl([this](std::string_view /*unused*/, Impl &impl, bool isBuiltin) {
		if (isBuiltin) {
			return;
		}
		impl.accept(*this);
	});
	if (!hasMain) {
		errorHandler->error(module->getSource(), "No main function");
	}
}

static void addDefaultOperators(Module *module) {
	addDefaultIntegerOperators(module);
	addDefaultBoolOperators(module);
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
	};
	const std::array comparisonOperators = {
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

	addSameSignIntegerArithmeticTypes(module, signedIntegerTypes, binaryIntegerOperators,
	      unaryIntegerOperators);
	addSameSignIntegerArithmeticTypes(module, unsignedIntegerTypes,
	      binaryIntegerOperators, unaryIntegerOperators);
	addSameSignIntegerComparisonTypes(module, signedIntegerTypes, comparisonOperators);
	addSameSignIntegerComparisonTypes(module, unsignedIntegerTypes, comparisonOperators);
}

static void addSameSignIntegerArithmeticTypes(Module *module,
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

static void addSameSignIntegerComparisonTypes(Module *module,
      const std::span<const std::string_view> types,
      const std::span<const Operator::Type> comparisonOperators) {
	static const int boolTypeID = module->getType("bool").id;
	for (auto it = types.begin(); it != types.end(); it++) {
		int typeID = module->getType(*it).id;
		for (const auto &op : comparisonOperators) {
			module->addBinaryOperator(op, typeID, typeID, boolTypeID);
		}
	}
}

static void addDefaultBoolOperators(Module *module) {
	static const int boolTypeID = module->getType("bool").id;
	module->addUnaryOperator(Operator::Type::LogicalNot, boolTypeID, boolTypeID);
	module->addBinaryOperator(Operator::Type::LogicalAnd, boolTypeID, boolTypeID,
	      boolTypeID);
	module->addBinaryOperator(Operator::Type::LogicalOr, boolTypeID, boolTypeID,
	      boolTypeID);
	module->addBinaryOperator(Operator::Type::Equality, boolTypeID, boolTypeID,
	      boolTypeID);
	module->addBinaryOperator(Operator::Type::Inequality, boolTypeID, boolTypeID,
	      boolTypeID);
	static const int unitTypeID = module->getType("()").id;
	module->addBinaryOperator(Operator::Type::Assignment, boolTypeID, boolTypeID,
	      unitTypeID);
}

static void addRuntimeFunctions(Module *module, std::istream &builtinApiJsonFile) {
	json data;
	builtinApiJsonFile >> data;

	for (const auto &[name, function] : data["functions"].items()) {
		module->ownedStrings.push_back(name);
		std::string_view functionName = module->ownedStrings.back();
		std::vector<std::pair<std::unique_ptr<Symbol>, std::unique_ptr<Symbol>>>
		      parameters;
		for (auto &parameter : function["parameters"]) {
			module->ownedStrings.push_back(parameter["name"].get<std::string>());
			std::string_view name = module->ownedStrings.back();
			module->ownedStrings.push_back(parameter["type"].get<std::string>());
			std::string_view type = module->ownedStrings.back();
			parameters.emplace_back(std::make_unique<Symbol>(Slice(name, "", 0, 0)),
			      std::make_unique<Symbol>(Slice(type, "", 0, 0)));
		}
		module->ownedStrings.emplace_back(function["returnType"].get<std::string>());
		std::string_view returnType = module->ownedStrings.back();
		std::unique_ptr<Symbol> returnTypeAnnotation
		      = std::make_unique<Symbol>(Slice(returnType, "", 0, 0));
		std::unique_ptr<Function> builtin = std::make_unique<Function>(
		      std::move(parameters), std::move(returnTypeAnnotation),
		      std::make_unique<BlockExpression>(), FunctionVariant::FUNCTION);
		module->addFunction(std::make_unique<Symbol>(Slice(functionName, "", 0, 0)),
		      std::move(builtin), true);
	}
}
