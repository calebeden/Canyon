#ifndef AST_H
#define AST_H

#include "tokens.h"

#include <functional>
#include <list>
#include <memory>
#include <tuple>
#include <unordered_map>
#include <vector>

/// Classes representing the parsed Abstract Syntax Tree (AST) of Canyon source code

class ASTVisitor;

class ASTComponent {
public:
	virtual void accept(ASTVisitor &visitor) = 0;
	virtual ~ASTComponent() = default;
};

class Expression : public ASTComponent {
protected:
	Slice s;
	int typeID = -1;
	Expression(const Slice &s);
public:
	int getTypeID() const;
	void setTypeID(int typeID);
	Slice &getSlice();
	virtual void accept(ASTVisitor &visitor) = 0;
	virtual ~Expression() = default;
};

class Statement : public ASTComponent {
protected:
	Slice s;
	Statement(const Slice &s);
public:
	Slice &getSlice();
	virtual void accept(ASTVisitor &visitor) = 0;
	virtual ~Statement() = default;
};

enum class FunctionVariant {
	FUNCTION,
	METHOD,
	CONSTRUCTOR,
};

class FunctionCallExpression : public Expression {
	std::unique_ptr<Expression> function;
	std::vector<std::unique_ptr<Expression>> arguments;
	FunctionVariant variant;
public:
	FunctionCallExpression(std::unique_ptr<Expression> function, const Punctuation &open,
	      std::vector<std::unique_ptr<Expression>> arguments, const Punctuation &close);
	FunctionCallExpression(std::unique_ptr<Expression> function,
	      std::vector<std::unique_ptr<Expression>> arguments);
	Expression &getFunction();
	void addFirstArgument(std::unique_ptr<Expression> argument);
	void forEachArgument(const std::function<void(Expression &)> &argumentHandler);
	void setVariant(FunctionVariant variant);
	FunctionVariant getVariant() const;
	void accept(ASTVisitor &visitor) override;
	virtual ~FunctionCallExpression() = default;
};

class BinaryExpression : public Expression {
private:
	std::unique_ptr<Operator> op;
	std::unique_ptr<Expression> left;
	std::unique_ptr<Expression> right;
public:
	BinaryExpression(std::unique_ptr<Operator> op, std::unique_ptr<Expression> left,
	      std::unique_ptr<Expression> right);
	Expression &getLeft();
	Expression &getRight();
	Operator &getOperator();
	void accept(ASTVisitor &visitor) override;
	virtual ~BinaryExpression() = default;
};

class UnaryExpression : public Expression {
private:
	std::unique_ptr<Operator> op;
	std::unique_ptr<Expression> operand;
public:
	UnaryExpression(std::unique_ptr<Operator> op, std::unique_ptr<Expression> operand);
	void accept(ASTVisitor &visitor) override;
	Expression &getExpression();
	Operator &getOperator();
	virtual ~UnaryExpression() = default;
};

class IntegerLiteralExpression : public Expression {
private:
	std::unique_ptr<IntegerLiteral> literal;
public:
	IntegerLiteralExpression(std::unique_ptr<IntegerLiteral> literal);
	IntegerLiteral &getLiteral();
	void accept(ASTVisitor &visitor) override;
	virtual ~IntegerLiteralExpression() = default;
};

class BoolLiteralExpression : public Expression {
private:
	std::unique_ptr<BoolLiteral> literal;
public:
	BoolLiteralExpression(std::unique_ptr<BoolLiteral> literal);
	BoolLiteral &getLiteral();
	void accept(ASTVisitor &visitor) override;
	virtual ~BoolLiteralExpression() = default;
};

class CharacterLiteralExpression : public Expression {
private:
	std::unique_ptr<CharacterLiteral> literal;
public:
	CharacterLiteralExpression(std::unique_ptr<CharacterLiteral> literal);
	CharacterLiteral &getLiteral();
	void accept(ASTVisitor &visitor) override;
	virtual ~CharacterLiteralExpression() = default;
};

class SymbolExpression : public Expression {
private:
	std::unique_ptr<Symbol> symbol;
public:
	SymbolExpression(std::unique_ptr<Symbol> symbol);
	Symbol &getSymbol();
	void accept(ASTVisitor &visitor) override;
	virtual ~SymbolExpression() = default;
};

enum class SymbolSource {
	Unknown,
	LetStatement,
	FunctionParameter,
	FieldDeclaration,
	GENERATED_Block,
	GENERATED_IfElse,
	GENERATED_While,
	GENERATED_Argument,
};

class BlockExpression : public Expression {
private:
	std::vector<std::unique_ptr<Statement>> statements;
	std::unique_ptr<Expression> finalExpression;
	std::unordered_map<std::string_view, std::tuple<int, SymbolSource>> symbols;
public:
	BlockExpression(const Punctuation &open,
	      std::vector<std::unique_ptr<Statement>> statements,
	      std::unique_ptr<Expression> finalExpression, const Punctuation &close);
	BlockExpression();
	void forEachStatement(const std::function<void(Statement &)> &statementHandler);
	Expression *getFinalExpression();
	int getSymbolType(std::string_view symbol);
	void setSymbolType(std::string_view symbol, int typeID);
	SymbolSource getSymbolSource(std::string_view symbol);
	void pushSymbol(std::string_view symbol, int typeID, SymbolSource source);
	void forEachSymbol(
	      const std::function<void(std::string_view, int, SymbolSource)> &symbolHandler);
	void pushStatement(std::unique_ptr<Statement> statement);
	void accept(ASTVisitor &visitor) override;
	virtual ~BlockExpression() = default;
};

class ReturnExpression : public Expression {
	std::unique_ptr<Expression> expression;
public:
	ReturnExpression(const Keyword &returnKeyword,
	      std::unique_ptr<Expression> expression);
	ReturnExpression(std::unique_ptr<Expression> expression);
	Expression *getExpression();
	void accept(ASTVisitor &visitor) override;
	virtual ~ReturnExpression() = default;
};

class ParenthesizedExpression : public Expression {
private:
	std::unique_ptr<Expression> expression;
public:
	ParenthesizedExpression(const Punctuation &open,
	      std::unique_ptr<Expression> expression, const Punctuation &close);
	explicit ParenthesizedExpression(std::unique_ptr<Expression> expression);
	Expression &getExpression();
	void accept(ASTVisitor &visitor) override;
	virtual ~ParenthesizedExpression() = default;
};

class PathExpression : public Expression {
private:
	std::vector<std::unique_ptr<SymbolExpression>> path;
public:
	PathExpression(std::vector<std::unique_ptr<SymbolExpression>> path);
	void forEachSymbol(const std::function<void(SymbolExpression &)> &symbolHandler);
	void accept(ASTVisitor &visitor) override;
	virtual ~PathExpression() = default;
};

class FieldAccessExpression : public Expression {
private:
	std::unique_ptr<Expression> object;
	std::unique_ptr<SymbolExpression> field;
public:
	FieldAccessExpression(std::unique_ptr<Expression> object,
	      std::unique_ptr<SymbolExpression> field);
	Expression &getObject();
	SymbolExpression &getField();
	void accept(ASTVisitor &visitor) override;
	virtual ~FieldAccessExpression() = default;
};

class IfElseExpression : public Expression {
private:
	std::unique_ptr<Expression> condition;
	std::unique_ptr<BlockExpression> thenBlock;
	std::unique_ptr<Expression> elseExpression;
public:
	IfElseExpression(const Keyword &ifKeyword, std::unique_ptr<Expression> condition,
	      std::unique_ptr<BlockExpression> thenBlock, const Keyword &elseKeyword,
	      std::unique_ptr<Expression> elseExpression);
	IfElseExpression(const Keyword &ifKeyword, std::unique_ptr<Expression> condition,
	      std::unique_ptr<BlockExpression> thenBlock);
	IfElseExpression(std::unique_ptr<Expression> condition,
	      std::unique_ptr<BlockExpression> thenBlock,
	      std::unique_ptr<Expression> elseExpression);
	Expression &getCondition();
	BlockExpression &getThenBlock();
	Expression *getElseExpression();
	void accept(ASTVisitor &visitor) override;
	virtual ~IfElseExpression() = default;
};

class WhileExpression : public Expression {
private:
	std::unique_ptr<Expression> condition;
	std::unique_ptr<BlockExpression> body;
public:
	WhileExpression(const Keyword &whileKeyword, std::unique_ptr<Expression> condition,
	      std::unique_ptr<BlockExpression> body);
	WhileExpression(std::unique_ptr<Expression> condition,
	      std::unique_ptr<BlockExpression> body);
	Expression &getCondition();
	BlockExpression &getBody();
	void accept(ASTVisitor &visitor) override;
	virtual ~WhileExpression() = default;
};

class ExpressionStatement : public Statement {
private:
	std::unique_ptr<Expression> expression;
public:
	ExpressionStatement(std::unique_ptr<Expression> expression,
	      const Punctuation &semicolon);
	ExpressionStatement(std::unique_ptr<Expression> expression);
	Expression &getExpression();
	void accept(ASTVisitor &visitor) override;
	virtual ~ExpressionStatement() = default;
};

class LetStatement : public Statement {
private:
	std::unique_ptr<Symbol> symbol;
	std::unique_ptr<Symbol> typeAnnotation;
	std::unique_ptr<Operator> equalSign;
	std::unique_ptr<Expression> expression;
	bool isFieldDeclaration = false;
	int symbolTypeID = -1;
public:
	LetStatement(const Keyword &let, std::unique_ptr<Symbol> symbol,
	      std::unique_ptr<Symbol> typeAnnotation, std::unique_ptr<Operator> equalSign,
	      std::unique_ptr<Expression> expression, Punctuation *semicolon);
	LetStatement(std::unique_ptr<Symbol> symbol, std::unique_ptr<Symbol> typeAnnotation, Punctuation *semicolon);
	LetStatement(std::unique_ptr<Symbol> symbol, std::unique_ptr<Expression> expression);
	Symbol &getSymbol();
	Expression *getExpression();
	Symbol *getTypeAnnotation();
	Operator &getEqualSign();
	void setSymbolTypeID(int typeID);
	int getSymbolTypeID() const;
	bool getIsFieldDeclaration() const;
	void accept(ASTVisitor &visitor) override;
	virtual ~LetStatement() = default;
};

class Function : public ASTComponent {
private:
	std::vector<std::pair<std::unique_ptr<Symbol>, std::unique_ptr<Symbol>>> parameters;
	std::unique_ptr<Symbol> returnTypeAnnotation;
	std::unique_ptr<BlockExpression> body;
	FunctionVariant variant;
	int typeID = -1;
public:
	Function(std::vector<std::pair<std::unique_ptr<Symbol>, std::unique_ptr<Symbol>>>
	               parameters,
	      std::unique_ptr<Symbol> returnTypeAnnotation,
	      std::unique_ptr<BlockExpression> body, FunctionVariant variant);
	Function(std::vector<std::pair<std::unique_ptr<Symbol>, std::unique_ptr<Symbol>>>
	               parameters,
	      std::unique_ptr<BlockExpression> body, FunctionVariant variant);
	void forEachParameter(
	      const std::function<void(Symbol &, Symbol &)> &parameterHandler);
	Symbol *getReturnTypeAnnotation();
	BlockExpression &getBody();
	int getTypeID() const;
	void setTypeID(int typeID);
	void setVariant(FunctionVariant variant);
	FunctionVariant getVariant() const;
	void accept(ASTVisitor &visitor);
	~Function() = default;
};

class Class : public ASTComponent {
private:
	std::vector<std::unique_ptr<LetStatement>> fieldDeclarations;
	std::unique_ptr<BlockExpression> scope = std::make_unique<BlockExpression>();
public:
	Class(std::vector<std::unique_ptr<LetStatement>> fieldDeclarations);
	Class(std::unique_ptr<BlockExpression> scope);
	void forEachFieldDeclaration(const std::function<void(LetStatement &)> &fieldHandler);
	BlockExpression &getScope();
	void accept(ASTVisitor &visitor);
	~Class() = default;
};

class Impl : public ASTComponent {
private:
	std::unordered_map<std::string_view, std::unique_ptr<Function>> methods;
public:
	Impl(std::unordered_map<std::string_view, std::unique_ptr<Function>> methods);
	void forEachMethod(
	      const std::function<void(std::string_view, Function &)> &methodHandler);
	Function *getMethod(std::string_view name);
	void accept(ASTVisitor &visitor);
	~Impl() = default;
};

struct Type {
	int id;
	int parentID;
	bool isClass;
	std::string_view name;
	Type(int id, int parentID, std::string_view name, bool isClass);
	Type(const Type &) = default;
};

class Module : public ASTComponent {
private:
	std::unordered_map<std::string_view, std::tuple<std::unique_ptr<Function>, bool>>
	      functions;
	std::unordered_map<std::string_view, Type> typeTableByName;
	std::unordered_map<int, Type> typeTableByID;
	std::unordered_map<Operator::Type, std::vector<std::tuple<int, int>>> unaryOperators;
	std::unordered_map<Operator::Type, std::vector<std::tuple<int, int, int>>>
	      binaryOperators;
	std::filesystem::path source;
	std::unordered_map<std::string_view, std::tuple<std::unique_ptr<Class>, bool>>
	      classes;
	std::unordered_map<std::string_view, std::tuple<std::unique_ptr<Impl>, bool>> impls;
public:
	std::list<std::string> ownedStrings;
	explicit Module(std::filesystem::path source);
	explicit Module(const Module &module);
	void addFunction(std::unique_ptr<Symbol> name, std::unique_ptr<Function> function,
	      bool isBuiltin = false);
	void forEachFunction(
	      const std::function<void(std::string_view, Function &, bool)> &functionHandler);
	void addClass(std::unique_ptr<Symbol> name, std::unique_ptr<Class> cls,
	      bool isBuiltin = false);
	void addImpl(std::unique_ptr<Symbol> className, std::unique_ptr<Impl> impl,
	      bool isBuiltin = false);
	void forEachClass(
	      const std::function<void(std::string_view, Class &, bool)> &classHandler);
	void forEachImpl(
	      const std::function<void(std::string_view, Impl &, bool)> &implHandler);
	Type getType(std::string_view typeName);
	Type getType(int id);
	void insertType(std::string_view typeName, bool isClass);
	bool isTypeConvertible(int from, int to);
	Type getCommonTypeAncestor(int type1, int type2);
	void addUnaryOperator(Operator::Type op, int operandType, int resultType);
	int getUnaryOperator(Operator::Type op, int operandType);
	void addBinaryOperator(Operator::Type op, int leftType, int rightType,
	      int resultType);
	int getBinaryOperator(Operator::Type op, int leftType, int rightType);
	Function *getFunction(std::string_view name);
	Impl *getImpl(std::string_view name);
	std::pair<std::string_view, Impl *> getImpl(int typeID);
	std::pair<std::string_view, Class *> getClass(int typeID);
	std::filesystem::path getSource();
	void accept(ASTVisitor &visitor);
	~Module() = default;
};

class ASTVisitor {
public:
	virtual void visit(FunctionCallExpression &node) = 0;
	virtual void visit(BinaryExpression &node) = 0;
	virtual void visit(UnaryExpression &node) = 0;
	virtual void visit(IntegerLiteralExpression &node) = 0;
	virtual void visit(BoolLiteralExpression &node) = 0;
	virtual void visit(CharacterLiteralExpression &node) = 0;
	virtual void visit(SymbolExpression &node) = 0;
	virtual void visit(BlockExpression &node) = 0;
	virtual void visit(ReturnExpression &node) = 0;
	virtual void visit(ParenthesizedExpression &node) = 0;
	virtual void visit(PathExpression &node) = 0;
	virtual void visit(FieldAccessExpression &node) = 0;
	virtual void visit(IfElseExpression &node) = 0;
	virtual void visit(WhileExpression &node) = 0;
	virtual void visit(ExpressionStatement &node) = 0;
	virtual void visit(LetStatement &node) = 0;
	virtual void visit(Function &node) = 0;
	virtual void visit(Class &node) = 0;
	virtual void visit(Impl &node) = 0;
	virtual void visit(Module &node) = 0;
	virtual ~ASTVisitor() = default;
};

class ASTPrinter : public ASTVisitor {
	int tabLevel = 0;
public:
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
	void visit(PathExpression &node) override;
	void visit(FieldAccessExpression &node) override;
	void visit(IfElseExpression &node) override;
	void visit(WhileExpression &node) override;
	void visit(ExpressionStatement &node) override;
	void visit(LetStatement &node) override;
	void visit(Function &node) override;
	void visit(Class &node) override;
	void visit(Impl &node) override;
	void visit(Module &node) override;
	virtual ~ASTPrinter() = default;
};

#endif
