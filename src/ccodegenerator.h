#ifndef CCODEGENERATOR_H
#define CCODEGENERATOR_H

#include "ast.h"

#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>

/**
 * @brief Generates C code from a Canyon AST
 *
 */
class CCodeGenerator : public ASTVisitor {
private:
	Module *module;
	std::ostream *os;
	std::unordered_map<int, std::string> cTypes;
	int tabLevel = 0;
	std::list<std::string> generatedStrings;
public:
	CCodeGenerator(Module *module, std::ostream *os);
	void generate();
	void visit(FunctionCallExpression &node) override;
	void visit(BinaryExpression &node) override;
	void visit(UnaryExpression &node) override;
	void visit(IntegerLiteralExpression &node) override;
	void visit(BoolLiteralExpression &node) override;
	void visit(CharacterLiteralExpression &node) override;
	void visit(SymbolExpression &node) override;
	void visit(BlockExpression &node) override;
	void visit(ReturnExpression &node) override;
	void visit(ParenthesizedExpression &node) override;
	void visit(IfElseExpression &node) override;
	void visit(WhileExpression &node) override;
	void visit(ExpressionStatement &node) override;
	void visit(LetStatement &node) override;
	void visit(Function &node) override;
	void visit(Module &node) override;
	~CCodeGenerator() = default;
private:
	void generateIncludes();
	void generateMain();
};

#endif
