#include "ccodegenerator.h"

#include "ccodeadapter.h"

#include <iostream>
#include <list>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>

CCodeGenerator::CCodeGenerator(Module *module, std::ostream *os)
    : module(module), os(os) {
	cTypes[-1] = "UNKNOWN_TYPE";
	cTypes[this->module->getType("()").id] = "void";
	cTypes[this->module->getType("i8").id] = "int8_t";
	cTypes[this->module->getType("i16").id] = "int16_t";
	cTypes[this->module->getType("i32").id] = "int32_t";
	cTypes[this->module->getType("i64").id] = "int64_t";
	cTypes[this->module->getType("u8").id] = "uint8_t";
	cTypes[this->module->getType("u16").id] = "uint16_t";
	cTypes[this->module->getType("u32").id] = "uint32_t";
	cTypes[this->module->getType("u64").id] = "uint64_t";
	cTypes[this->module->getType("bool").id] = "bool";
	cTypes[this->module->getType("char").id] = "char";
}

void CCodeGenerator::generate() {
	generateIncludes();
	CCodeAdapter adapter = CCodeAdapter(module, &generatedStrings);
	std::unique_ptr<Module> adapted = adapter.transform();
	visit(*adapted);
}

void CCodeGenerator::generateIncludes() {
	*os << "#include <stdint.h>\n"
	       "#include <stdbool.h>\n"
	       "#include <stdio.h>\n"
	       "#include <inttypes.h>\n"
	       "\n";
}

void CCodeGenerator::visit(FunctionCallExpression &node) {
	node.getFunction().accept(*this);
	*os << '(';
	bool first = true;
	node.forEachArgument([this, &first](Expression &argument) {
		if (!first) {
			*os << ", ";
		}
		first = false;
		argument.accept(*this);
	});
	*os << ')';
}

void CCodeGenerator::visit(BinaryExpression &node) {
	*os << '(';
	node.getLeft().accept(*this);
	*os << ") " << node.getOperator().s << " (";
	node.getRight().accept(*this);
	*os << ')';
}

void CCodeGenerator::visit(UnaryExpression &node) {
	*os << node.getOperator().s;
	*os << '(';
	node.getExpression().accept(*this);
	*os << ')';
}

void CCodeGenerator::visit(IntegerLiteralExpression &node) {
	IntegerLiteral &literal = node.getLiteral();
	switch (literal.type) {
		case IntegerLiteral::Type::I8:
		case IntegerLiteral::Type::I16:
		case IntegerLiteral::Type::I32:
			*os << literal.value;
			break;
		case IntegerLiteral::Type::I64:
			*os << literal.value << "LL";
			break;
		case IntegerLiteral::Type::U8:
		case IntegerLiteral::Type::U16:
		case IntegerLiteral::Type::U32:
			*os << literal.value << "U";
			break;
		case IntegerLiteral::Type::U64:
			*os << literal.value << "ULL";
			break;
	}
}

void CCodeGenerator::visit(BoolLiteralExpression &node) {
	*os << (node.getLiteral().value ? "true" : "false");
}

void CCodeGenerator::visit(CharacterLiteralExpression &node) {
	*os << '\'';
	char c = node.getLiteral().value;
	switch (c) {
		case '\a':
			*os << "\\a";
			break;
		case '\b':
			*os << "\\b";
			break;
		case '\f':
			*os << "\\f";
			break;
		case '\n':
			*os << "\\n";
			break;
		case '\r':
			*os << "\\r";
			break;
		case '\t':
			*os << "\\t";
			break;
		case '\v':
			*os << "\\v";
			break;
		case '\\':
			*os << "\\\\";
			break;
		case '\'':
			*os << "\\'";
			break;
		case '\"':
			*os << "\\\"";
			break;
		default:
			*os << c;
			break;
	}
	*os << '\'';
}

void CCodeGenerator::visit(SymbolExpression &node) {
	*os << node.getSymbol().s;
}

void CCodeGenerator::visit(BlockExpression &node) {
	*os << "{\n";
	tabLevel++;
	node.forEachStatement([this](Statement &statement) {
		*os << std::string(tabLevel, '\t');
		statement.accept(*this);
	});
	tabLevel--;
	*os << std::string(tabLevel, '\t');
	*os << "}\n";
}

void CCodeGenerator::visit(ReturnExpression &node) {
	// Invariant: the return expression will only ever be in an ExpressionStatement (not
	// nested inside another expression) Otherwise it would trigger unreachable code error
	// before reaching the codegen phase
	*os << "return";
	Expression *expression = node.getExpression();
	if (expression != nullptr) {
		*os << ' ';
		expression->accept(*this);
	}
}

void CCodeGenerator::visit(ParenthesizedExpression &node) {
	node.getExpression().accept(*this);
}

void CCodeGenerator::visit(IfElseExpression &node) {
	*os << "if (";
	node.getCondition().accept(*this);
	*os << ") ";
	node.getThenBlock().accept(*this);
	Expression *elseExpression = node.getElseExpression();
	if (elseExpression != nullptr) {
		*os << std::string(tabLevel, '\t');
		*os << "else ";
		elseExpression->accept(*this);
	}
}

void CCodeGenerator::visit(WhileExpression &node) {
	*os << "while (";
	node.getCondition().accept(*this);
	*os << ") ";
	node.getBody().accept(*this);
}

void CCodeGenerator::visit(ExpressionStatement &node) {
	node.getExpression().accept(*this);
	if (!dynamic_cast<BlockExpression *>(&node.getExpression())) {
		*os << ";\n";
	}
}

void CCodeGenerator::visit(LetStatement &node) {
	const std::string &cType = cTypes[node.getSymbolTypeID()];
	Expression *expression = node.getExpression();
	if (expression != nullptr) {
		*os << cType << ' ' << node.getSymbol().s << " = ";
		node.getExpression()->accept(*this);
		*os << ";\n";
	} else {
		*os << cType << ' ' << node.getSymbol().s << ";\n";
	}
}

void CCodeGenerator::visit([[maybe_unused]] Function &node) {
	// Currently being handled in the Module visitor
}

void CCodeGenerator::visit([[maybe_unused]] Class &node) {
	// Currently being handled in the Module visitor
}

void CCodeGenerator::visit([[maybe_unused]] Impl &node) {
	// TODO
}

void CCodeGenerator::visit(Module &node) {
	// Function prototypes
	node.forEachFunction(
	      [this](std::string_view name, Function &function, bool /*unused*/) {
		      Type functionType = module->getType(function.getTypeID());
		      const std::string &cType = cTypes[functionType.id];
		      *os << cType << ' ' << name << '(';
		      bool first = true;
		      function.forEachParameter([this, &first](Symbol &parameter, Symbol &type) {
			      const std::string &cType = cTypes[module->getType(type.s.contents).id];
			      if (!first) {
				      *os << ", ";
			      }
			      first = false;
			      *os << cType << ' ' << parameter.s;
		      });
		      *os << ");\n";
	      });
	*os << '\n';

	// Method prototypes
	node.forEachImpl([this](std::string_view name, Impl &impl, bool /*unused*/) {
		impl.forEachMethod([this, &name](std::string_view methodName, Function &method) {
			Type functionType = module->getType(method.getTypeID());
			const std::string &cType = cTypes[functionType.id];
			*os << cType << ' ' << name << "_METHOD_" << methodName << '(';
			bool first = true;
			method.forEachParameter([this, &first](Symbol &parameter, Symbol &type) {
				const std::string &cType = cTypes[module->getType(type.s.contents).id];
				if (!first) {
					*os << ", ";
				}
				first = false;
				*os << cType << ' ' << parameter.s;
			});
			*os << ");\n";
		});
	});
	*os << '\n';

	// Virtual tables
	node.forEachImpl([this](std::string_view name, Impl &impl, bool /*unused*/) {
		*os << "struct " << name << "_VT {\n";
		tabLevel++;
		impl.forEachMethod([this](std::string_view methodName, Function &method) {
			Type methodType = module->getType(method.getTypeID());
			const std::string &cType = cTypes[methodType.id];
			*os << std::string(tabLevel, '\t') << cType << " (*" << methodName << ")(";
			bool first = true;
			method.forEachParameter([this, &first](Symbol & /*unused*/, Symbol &type) {
				const std::string &cType = cTypes[module->getType(type.s.contents).id];
				if (!first) {
					*os << ", ";
				}
				first = false;
				*os << cType;
			});
			*os << ");\n";
		});
		tabLevel--;
		*os << "} " << name << "_VT_INSTANCE = {\n";
		tabLevel++;
		impl.forEachMethod(
		      [this, &name](std::string_view methodName, Function & /*unused*/) {
			      std::string newName
			            = std::string(name) + "_METHOD_" + std::string(methodName);
			      *os << std::string(tabLevel, '\t') << newName << ",\n";
		      });
		tabLevel--;
		*os << "};\n";
	});
	*os << '\n';

	// Class structs
	node.forEachClass([this](std::string_view name, Class &cls, bool /*unused*/) {
		*os << "typedef struct {\n";
		tabLevel++;
		cls.forEachFieldDeclaration([this](LetStatement &declaration) {
			*os << std::string(tabLevel, '\t');
			const std::string &cType = cTypes[declaration.getSymbolTypeID()];
			*os << cType << ' ' << declaration.getSymbol().s << ";\n";
		});
		tabLevel--;
		*os << "} *" << name << ";\n";
	});
	*os << '\n';

	// Function definitions
	generateBuiltinFunctions();
	node.forEachFunction(
	      [this](std::string_view name, Function &function, bool isBuiltin) {
		      if (isBuiltin) {
			      return;
		      }
		      Type functionType = module->getType(function.getTypeID());
		      const std::string &cType = cTypes[functionType.id];
		      *os << cType << ' ' << name << '(';
		      bool first = true;
		      function.forEachParameter([this, &first](Symbol &parameter, Symbol &type) {
			      const std::string &cType = cTypes[module->getType(type.s.contents).id];
			      if (!first) {
				      *os << ", ";
			      }
			      first = false;
			      *os << cType << ' ' << parameter.s;
		      });
		      *os << ") ";
		      function.getBody().accept(*this);
		      *os << "\n";
	      });
	*os << '\n';

	// Method definitions
	node.forEachImpl([this](std::string_view name, Impl &impl, bool isBuiltin) {
		if (isBuiltin) {
			return;
		}
		impl.forEachMethod([this, &name](std::string_view methodName, Function &method) {
			Type functionType = module->getType(method.getTypeID());
			const std::string &cType = cTypes[functionType.id];
			*os << cType << ' ' << name << "_METHOD_" << methodName << '(';
			bool first = true;
			method.forEachParameter([this, &first](Symbol &parameter, Symbol &type) {
				const std::string &cType = cTypes[module->getType(type.s.contents).id];
				if (!first) {
					*os << ", ";
				}
				first = false;
				*os << cType << ' ' << parameter.s;
			});
			*os << ") ";
			method.getBody().accept(*this);
			*os << "\n";
		});
	});
}

void CCodeGenerator::generateBuiltinFunctions() {
	*os << "int main() {\n"
	       "\tCANYON_FUNCTION_main();\n"
	       "\treturn 0;\n"
	       "}\n"
	       "void CANYON_FUNCTION_printI8(int8_t CANYON_PARAMETER_value) {\n"
	       "\tprintf(\"%\" PRId8, CANYON_PARAMETER_value);\n"
	       "}\n"
	       "void CANYON_FUNCTION_printI16(int16_t CANYON_PARAMETER_value) {\n"
	       "\tprintf(\"%\" PRId16, CANYON_PARAMETER_value);\n"
	       "}\n"
	       "void CANYON_FUNCTION_printI32(int32_t CANYON_PARAMETER_value) {\n"
	       "\tprintf(\"%\" PRId32, CANYON_PARAMETER_value);\n"
	       "}\n"
	       "void CANYON_FUNCTION_printI64(int64_t CANYON_PARAMETER_value) {\n"
	       "\tprintf(\"%\" PRId64, CANYON_PARAMETER_value);\n"
	       "}\n"
	       "void CANYON_FUNCTION_printU8(uint8_t CANYON_PARAMETER_value) {\n"
	       "\tprintf(\"%\" PRIu8, CANYON_PARAMETER_value);\n"
	       "}\n"
	       "void CANYON_FUNCTION_printU16(uint16_t CANYON_PARAMETER_value) {\n"
	       "\tprintf(\"%\" PRIu16, CANYON_PARAMETER_value);\n"
	       "}\n"
	       "void CANYON_FUNCTION_printU32(uint32_t CANYON_PARAMETER_value) {\n"
	       "\tprintf(\"%\" PRIu32, CANYON_PARAMETER_value);\n"
	       "}\n"
	       "void CANYON_FUNCTION_printU64(uint64_t CANYON_PARAMETER_value) {\n"
	       "\tprintf(\"%\" PRIu64, CANYON_PARAMETER_value);\n"
	       "}\n"
	       "void CANYON_FUNCTION_printBool(bool CANYON_PARAMETER_value) {\n"
	       "\tprintf(CANYON_PARAMETER_value ? \"true\" : \"false\");\n"
	       "}\n"
	       "void CANYON_FUNCTION_printChar(char CANYON_PARAMETER_value) {\n"
	       "\tprintf(\"%c\", CANYON_PARAMETER_value);\n"
	       "}\n";
}
