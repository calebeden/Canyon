#ifndef CCODEGENERATOR_H
#define CCODEGENERATOR_H

#include "ast.h"

#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>

class CCodeGenerator : public ASTVisitor {
private:
	std::unique_ptr<Module> module;
	std::ostream *os;
	std::unordered_map<int, std::string> cTypes;
	int tabLevel = 0;
public:
	CCodeGenerator(std::unique_ptr<Module> module, std::ostream *os);
	void generate();
	void visit(FunctionCallExpression &node) override;
	void visit(BinaryExpression &node) override;
	void visit(UnaryExpression &node) override;
	void visit(LiteralExpression &node) override;
	void visit(SymbolExpression &node) override;
	void visit(BlockExpression &node) override;
	void visit(ReturnExpression &node) override;
	void visit(ParenthesizedExpression &node) override;
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
