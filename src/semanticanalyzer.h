#ifndef SEMANTICANALYZER_H
#define SEMANTICANALYZER_H

#include "ast.h"
#include "errorhandler.h"
#include "tokens.h"

#include <memory>
#include <vector>

/**
 * @brief Validates whether an AST conforms to Canyon language semantics
 *
 */
class SemanticAnalyzer : public ASTVisitor {
	Module *module;
	ErrorHandler *errorHandler;
	std::vector<BlockExpression *> scopeStack;
	bool inUnreachableCode = false;
	Function *currentFunction = nullptr;
	std::istream &builtinApiJsonFile;
public:
	SemanticAnalyzer(Module *module, ErrorHandler *errorHandler,
	      std::istream &builtinApiJsonFile);
	void analyze();
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
	virtual ~SemanticAnalyzer() = default;
};

#endif
