#ifndef SEMANTICANALYZER_H
#define SEMANTICANALYZER_H

#include "ast.h"
#include "errorhandler.h"
#include "tokens.h"

#include <memory>
#include <vector>

class SemanticAnalyzer : public ASTVisitor {
	std::unique_ptr<Module> module;
	ErrorHandler *errorHandler;
	std::vector<BlockExpression *> scopeStack;
	bool inUnreachableCode = false;
	Function *currentFunction = nullptr;
public:
	SemanticAnalyzer(std::unique_ptr<Module> module, ErrorHandler *errorHandler);
	std::unique_ptr<Module> analyze();
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
	virtual ~SemanticAnalyzer() = default;
};

#endif