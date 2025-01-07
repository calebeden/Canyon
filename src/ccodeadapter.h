#ifndef CCODEADAPTER_H
#define CCODEADAPTER_H

#include "ast.h"

#include <list>
#include <memory>
#include <stack>
#include <string_view>
#include <vector>

/**
 * @brief Transforms an AST into one that is more suitable for generating C code
 *
 */
class CCodeAdapter : ASTVisitor {
private:
	Module *inputModule;
	std::unique_ptr<Module> outputModule;
	std::unique_ptr<ASTComponent> returnValue = nullptr;
	int blockCount = 0;
	std::stack<std::string_view> blockTemporaryVariables;
	std::vector<BlockExpression *> scopeStack;
	std::list<std::string> *generatedStrings;
	std::string *currentClassName = nullptr;
public:
	CCodeAdapter(Module *module, std::list<std::string> *generatedStrings);
	std::unique_ptr<Module> transform();
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
	void visit(Class &node) override;
	void visit(Module &node) override;
	virtual ~CCodeAdapter() = default;
private:
	void visitExpression(Expression &node);
};

#endif
