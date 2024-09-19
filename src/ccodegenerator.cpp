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
}

void CCodeGenerator::generate() {
	std::list<std::string> generatedStrings;
	generateIncludes();
	CCodeAdapter adapter = CCodeAdapter(module, &generatedStrings);
	std::unique_ptr<Module> adapted = adapter.transform();
	visit(*adapted);
}

void CCodeGenerator::generateIncludes() {
	*os << "#include <stdint.h>\n"
	       "\n";
}

void CCodeGenerator::visit(FunctionCallExpression &node) {
	node.getFunction().accept(*this);
	*os << "()";
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

void CCodeGenerator::visit(LiteralExpression &node) {
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

void CCodeGenerator::visit(Module &node) {
	// Forward declarations
	node.forEachFunction([this](std::string_view name, Function &function) {
		Type functionType = module->getType(function.getTypeID());
		const std::string &cType = cTypes[functionType.id];
		*os << cType << ' ' << name << "();\n";
	});
	*os << '\n';
	generateMain();

	// Function definitions
	node.forEachFunction([this](std::string_view name, Function &function) {
		Type functionType = module->getType(function.getTypeID());
		const std::string &cType = cTypes[functionType.id];
		*os << cType << ' ' << name << "() ";
		function.getBody().accept(*this);
		*os << "\n";
	});
}

void CCodeGenerator::generateMain() {
	*os << "int main() {\n"
	       "    CANYON_FUNCTION_main();\n"
	       "    return 0;\n"
	       "}\n"
	       "\n";
}
