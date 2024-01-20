#ifndef DEBUG_TEST_MODE
#	error "DEBUG_TEST_MODE not defined"
#endif

#include "ast.h"
#include "parser.h"
#include "tokens.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include <cstring>
#include <stdio.h>
#include <stdlib.h>
#include <vector>

using namespace AST;

struct EmptyRvalue : public rvalue {
	EmptyRvalue() : rvalue("", 0, 0) {
	}

	virtual void show() {
	}

	virtual void compile([[maybe_unused]] FILE *outfile) {
	}

	virtual Type typeCheck([[maybe_unused]] CodeBlock *context) {
		return Type::UNKNOWN;
	}
};

struct EmptyStatement : public Statement {
	EmptyStatement() {
	}

	virtual void show() {
	}

	virtual void compile([[maybe_unused]] FILE *outfile) {
	}

	virtual Type typeCheck([[maybe_unused]] CodeBlock *context,
	      [[maybe_unused]] Type returnType) {
		return Type::UNKNOWN;
	}
};

struct EmptyFunction : public Function {
	EmptyFunction(AST::AST *ast) : Function(ast) {
	}
};

TEST(test_parser, test_parseModule) {
	class MockParser : public Parser {
	public:
		MOCK_METHOD(void, parseFunctions, (std::vector<Token *> * tokens, AST::AST *ast));
	};

	std::vector<Token *> tokens;

	MockParser p;
	AST::Function *function;
	EXPECT_CALL(p, parseFunctions)
	      .WillOnce(::testing::Invoke(
	            [&function]([[maybe_unused]] std::vector<Token *> *tokens,
	                  [[maybe_unused]]
	                  AST::AST *ast) {
		            function = new EmptyFunction(ast);
		            ast->functions["canyonMain"] = function;
	            }));

	AST::AST *ast = p.parseModule(&tokens);
	// main + print
	EXPECT_EQ(ast->functions.size(), 2);
	AST::Function *main = ast->functions["canyonMain"];
	EXPECT_EQ(main, function);
}

TEST(test_parser, test_parseModule_error) {
	std::vector<Token *> tokens;
	Parser p;

	// Test 1: no main function defined
	EXPECT_EXIT(p.parseModule(&tokens), ::testing::ExitedWithCode(EXIT_FAILURE),
	      "Parse error: no main function");
}

TEST(test_parser, test_parseFunctions) {
	class MockParser : public Parser {
	public:
		MOCK_METHOD(void, parseFunction,
		      (std::vector<Token *>::iterator & it, AST::AST *ast));
	};

	std::vector<Token *> tokens;
	AST::AST *ast = new AST::AST;

	MockParser p;
	tokens = {};
	tokens.push_back(new Identifier(Slice("x", 1, "", 0, 0)));
	EXPECT_CALL(p, parseFunction)
	      .WillOnce(::testing::DoDefault())
	      .WillOnce(::testing::DoDefault())
	      .WillOnce(::testing::DoDefault())
	      .WillOnce(::testing::DoDefault())
	      .WillOnce(::testing::DoDefault())
	      .WillOnce(::testing::DoDefault())
	      .WillOnce(
	            ::testing::Invoke([](std::vector<Token *>::iterator &it, [[maybe_unused]]
	                                                                     AST::AST *ast) {
		            it++;
	            }));

	p.parseFunctions(&tokens, ast);
}

TEST(test_parser, test_parseFunction) {
	class MockParser : public Parser {
	public:
		MOCK_METHOD(void, parseParameters,
		      (std::vector<Token *>::iterator & it, AST::Function *function));
		MOCK_METHOD(void, parseBlock,
		      (std::vector<Token *>::iterator & it, AST::CodeBlock *context));
	};

	std::vector<Token *> tokens;
	std::vector<Token *>::iterator it;
	AST::AST *ast = new AST::AST;
	Identifier *param;
	Statement *statement;
	Function *function;

	// Test 1: Normal function parsing
	// 1a: Primitive return type
	MockParser p1a;
	tokens = {};
	ast = new AST::AST;
	tokens.push_back(new Primitive(Slice("int", 3, "", 0, 0), Type::INT));
	tokens.push_back(new Identifier(Slice("foo", 3, "", 0, 0)));
	param = new Identifier(Slice("x", 1, "", 0, 0));
	EXPECT_CALL(p1a, parseParameters)
	      .WillOnce(::testing::Invoke([param]([[maybe_unused]]
	                                          std::vector<Token *>::iterator &it,
	                                        AST::Function *function) {
		      function->parameters.emplace_back(param, Type::INT);
	      }));
	statement = new EmptyStatement;
	EXPECT_CALL(p1a, parseBlock)
	      .WillOnce(::testing::Invoke([statement]([[maybe_unused]]
	                                              std::vector<Token *>::iterator &it,
	                                        AST::CodeBlock *context) {
		      context->statements.push_back(statement);
	      }));
	it = tokens.begin();

	p1a.parseFunction(it, ast);

	EXPECT_EQ(it, tokens.end());
	// foo + print
	EXPECT_EQ(ast->functions.size(), 2);
	if (ast->functions.size() >= 2) {
		function = ast->functions["foo"];
		EXPECT_EQ(function->type, Type::INT);
		EXPECT_EQ(function->parameters.size(), 1);
		if (function->parameters.size() >= 1) {
			EXPECT_EQ(function->parameters[0].first->s, "x");
			EXPECT_EQ(function->parameters[0].second, Type::INT);
		}
		EXPECT_EQ(function->body->statements.size(), 1);
		if (function->body->statements.size() >= 1) {
			EXPECT_EQ(function->body->statements[0], statement);
		}
	}

	// 1b: void return type
	MockParser p1b;
	tokens = {};
	ast = new AST::AST;
	tokens.push_back(new Keyword(Slice("void", 4, "", 0, 0), Keyword::Type::VOID));
	tokens.push_back(new Identifier(Slice("foo", 3, "", 0, 0)));
	param = new Identifier(Slice("x", 1, "", 0, 0));
	EXPECT_CALL(p1b, parseParameters)
	      .WillOnce(::testing::Invoke([param]([[maybe_unused]]
	                                          std::vector<Token *>::iterator &it,
	                                        AST::Function *function) {
		      function->parameters.emplace_back(param, Type::INT);
	      }));
	statement = new EmptyStatement;
	EXPECT_CALL(p1b, parseBlock)
	      .WillOnce(::testing::Invoke([statement]([[maybe_unused]]
	                                              std::vector<Token *>::iterator &it,
	                                        AST::CodeBlock *context) {
		      context->statements.push_back(statement);
	      }));
	it = tokens.begin();

	p1b.parseFunction(it, ast);

	EXPECT_EQ(it, tokens.end());
	// foo + print
	EXPECT_EQ(ast->functions.size(), 2);
	if (ast->functions.size() >= 2) {
		function = ast->functions["foo"];
		EXPECT_EQ(function->type, Type::VOID);
		EXPECT_EQ(function->parameters.size(), 1);
		if (function->parameters.size() >= 1) {
			EXPECT_EQ(function->parameters[0].first->s, "x");
			EXPECT_EQ(function->parameters[0].second, Type::INT);
		}
		EXPECT_EQ(function->body->statements.size(), 1);
		if (function->body->statements.size() >= 1) {
			EXPECT_EQ(function->body->statements[0], statement);
		}
	}

	// Test 2: main gets mangled to canyonMain
	MockParser p2;
	tokens = {};
	ast = new AST::AST;
	tokens.push_back(new Keyword(Slice("void", 4, "", 0, 0), Keyword::Type::VOID));
	tokens.push_back(new Identifier(Slice("main", 4, "", 0, 0)));
	EXPECT_CALL(p2, parseParameters).Times(1);
	EXPECT_CALL(p2, parseBlock).Times(1);
	it = tokens.begin();

	p2.parseFunction(it, ast);

	EXPECT_EQ(it, tokens.end());
	// main + print
	EXPECT_EQ(ast->functions.size(), 2);
	if (ast->functions.size() >= 2) {
		function = ast->functions["canyonMain"];
		EXPECT_EQ(function->type, Type::VOID);
		EXPECT_EQ(function->parameters.size(), 0);
		EXPECT_EQ(function->body->statements.size(), 0);
	}
}

TEST(test_parser, test_parseFunction_error) {
	std::vector<Token *> tokens;
	std::vector<Token *>::iterator it;
	AST::AST *ast = new AST::AST;
	Parser p;

	// Test 1: Missing function type
	tokens = {};
	tokens.push_back(new Identifier(Slice("x", 1, "", 0, 0)));
	it = tokens.begin();

	EXPECT_EXIT(p.parseFunction(it, ast), ::testing::ExitedWithCode(EXIT_FAILURE),
	      "Expected function type");

	// Test 2: Missing function name
	// 2a: primitive type
	tokens = {};
	tokens.push_back(new Primitive(Slice("int", 3, "", 0, 0), Type::INT));
	tokens.push_back(
	      new Punctuation(Slice("(", 1, "", 0, 0), Punctuation::Type::OpenParen));
	it = tokens.begin();

	EXPECT_EXIT(p.parseFunction(it, ast), ::testing::ExitedWithCode(EXIT_FAILURE),
	      "Expected identifier");

	// 2a: void type
	tokens = {};
	tokens.push_back(new Keyword(Slice("void", 4, "", 0, 0), Keyword::Type::VOID));
	tokens.push_back(
	      new Punctuation(Slice("(", 1, "", 0, 0), Punctuation::Type::OpenParen));
	it = tokens.begin();

	EXPECT_EXIT(p.parseFunction(it, ast), ::testing::ExitedWithCode(EXIT_FAILURE),
	      "Expected identifier");
}

TEST(test_parser, test_parseParameters) {
	std::vector<Token *> tokens;
	std::vector<Token *>::iterator it;
	AST::Function *function = new AST::Function(new AST::AST);
	Parser p;

	tokens = {};
	tokens.push_back(
	      new Punctuation(Slice("(", 1, "", 0, 0), Punctuation::Type::OpenParen));
	tokens.push_back(new Primitive(Slice("int", 3, "", 0, 0), Type::INT));
	tokens.push_back(new Identifier(Slice("x", 1, "", 0, 0)));
	tokens.push_back(new Punctuation(Slice(",", 1, "", 0, 0), Punctuation::Type::Comma));
	tokens.push_back(new Primitive(Slice("float", 5, "", 0, 0), Type::FLOAT));
	tokens.push_back(new Identifier(Slice("y", 1, "", 0, 0)));
	tokens.push_back(new Punctuation(Slice(",", 1, "", 0, 0), Punctuation::Type::Comma));
	tokens.push_back(new Primitive(Slice("char", 4, "", 0, 0), Type::CHAR));
	tokens.push_back(new Identifier(Slice("z", 1, "", 0, 0)));
	tokens.push_back(new Punctuation(Slice(",", 1, "", 0, 0), Punctuation::Type::Comma));
	tokens.push_back(new Primitive(Slice("double", 6, "", 0, 0), Type::DOUBLE));
	tokens.push_back(new Identifier(Slice("a", 1, "", 0, 0)));
	tokens.push_back(new Punctuation(Slice(",", 1, "", 0, 0), Punctuation::Type::Comma));
	tokens.push_back(new Primitive(Slice("long", 4, "", 0, 0), Type::LONG));
	tokens.push_back(new Identifier(Slice("b", 1, "", 0, 0)));
	tokens.push_back(new Punctuation(Slice(",", 1, "", 0, 0), Punctuation::Type::Comma));
	tokens.push_back(new Primitive(Slice("bool", 4, "", 0, 0), Type::BOOL));
	tokens.push_back(new Identifier(Slice("c", 1, "", 0, 0)));
	tokens.push_back(
	      new Punctuation(Slice(")", 1, "", 0, 0), Punctuation::Type::CloseParen));
	it = tokens.begin();

	p.parseParameters(it, function);
	EXPECT_EQ(it, tokens.end());
	EXPECT_EQ(function->parameters.size(), 6);
	if (function->parameters.size() >= 6) {
		EXPECT_EQ(function->parameters[0].first->s, "x");
		EXPECT_EQ(function->parameters[0].second, Type::INT);
		EXPECT_EQ(function->parameters[1].first->s, "y");
		EXPECT_EQ(function->parameters[1].second, Type::FLOAT);
		EXPECT_EQ(function->parameters[2].first->s, "z");
		EXPECT_EQ(function->parameters[2].second, Type::CHAR);
		EXPECT_EQ(function->parameters[3].first->s, "a");
		EXPECT_EQ(function->parameters[3].second, Type::DOUBLE);
		EXPECT_EQ(function->parameters[4].first->s, "b");
		EXPECT_EQ(function->parameters[4].second, Type::LONG);
		EXPECT_EQ(function->parameters[5].first->s, "c");
		EXPECT_EQ(function->parameters[5].second, Type::BOOL);
	}
}

TEST(test_parser, test_parseParameters_error) {
	std::vector<Token *> tokens;
	std::vector<Token *>::iterator it;
	AST::Function *function = new AST::Function(new AST::AST);
	Parser p;

	// Test 1: Missing open parenthesis
	tokens = {};
	tokens.push_back(new Identifier(Slice("x", 1, "", 0, 0)));
	it = tokens.begin();

	EXPECT_EXIT(p.parseParameters(it, function), ::testing::ExitedWithCode(EXIT_FAILURE),
	      "Expected '\\('");

	// Test 2: Redeclaration of parameter of same type
	tokens = {};
	function = new AST::Function(new AST::AST);
	tokens.push_back(
	      new Punctuation(Slice("(", 1, "", 0, 0), Punctuation::Type::OpenParen));
	tokens.push_back(new Primitive(Slice("int", 3, "", 0, 0), Type::INT));
	tokens.push_back(new Identifier(Slice("x", 1, "", 0, 0)));
	tokens.push_back(new Punctuation(Slice(",", 1, "", 0, 0), Punctuation::Type::Comma));
	tokens.push_back(new Primitive(Slice("int", 3, "", 0, 0), Type::INT));
	tokens.push_back(new Identifier(Slice("x", 1, "", 0, 0)));
	it = tokens.begin();

	EXPECT_EXIT(p.parseParameters(it, function), ::testing::ExitedWithCode(EXIT_FAILURE),
	      "Re-declaration of parameter x");

	// Test 3: Redeclaration of parameter of different type
	tokens = {};
	function = new AST::Function(new AST::AST);
	tokens.push_back(
	      new Punctuation(Slice("(", 1, "", 0, 0), Punctuation::Type::OpenParen));
	tokens.push_back(new Primitive(Slice("int", 3, "", 0, 0), Type::INT));
	tokens.push_back(new Identifier(Slice("x", 1, "", 0, 0)));
	tokens.push_back(new Punctuation(Slice(",", 1, "", 0, 0), Punctuation::Type::Comma));
	tokens.push_back(new Primitive(Slice("float", 5, "", 0, 0), Type::FLOAT));
	tokens.push_back(new Identifier(Slice("x", 1, "", 0, 0)));
	it = tokens.begin();

	EXPECT_EXIT(p.parseParameters(it, function), ::testing::ExitedWithCode(EXIT_FAILURE),
	      "Re-declaration of parameter x");

	// Test 4: Missing parameter type
	tokens = {};
	tokens.push_back(
	      new Punctuation(Slice("(", 1, "", 0, 0), Punctuation::Type::OpenParen));
	tokens.push_back(new Identifier(Slice("x", 1, "", 0, 0)));
	it = tokens.begin();

	EXPECT_EXIT(p.parseParameters(it, function), ::testing::ExitedWithCode(EXIT_FAILURE),
	      "Expected primitive");

	// Test 5: Missing parameter name
	tokens = {};
	function = new AST::Function(new AST::AST);
	tokens.push_back(
	      new Punctuation(Slice("(", 1, "", 0, 0), Punctuation::Type::OpenParen));
	tokens.push_back(new Primitive(Slice("int", 3, "", 0, 0), Type::INT));
	tokens.push_back(new Punctuation(Slice(",", 1, "", 0, 0), Punctuation::Type::Comma));
	it = tokens.begin();

	EXPECT_EXIT(p.parseParameters(it, function), ::testing::ExitedWithCode(EXIT_FAILURE),
	      "Unexpected token following primitive");

	// Test 6: Unexpected token following parameter declaration
	// 6a: other punctuation
	tokens = {};
	function = new AST::Function(new AST::AST);
	tokens.push_back(
	      new Punctuation(Slice("(", 1, "", 0, 0), Punctuation::Type::OpenParen));
	tokens.push_back(new Primitive(Slice("int", 3, "", 0, 0), Type::INT));
	tokens.push_back(new Identifier(Slice("x", 1, "", 0, 0)));
	tokens.push_back(new Punctuation(Slice("+", 1, "", 0, 0), Punctuation::Type::Plus));
	it = tokens.begin();

	EXPECT_EXIT(p.parseParameters(it, function), ::testing::ExitedWithCode(EXIT_FAILURE),
	      "Unexpected token; expected '\\)' or ','");

	// 6b: Identifier
	tokens = {};
	function = new AST::Function(new AST::AST);
	tokens.push_back(
	      new Punctuation(Slice("(", 1, "", 0, 0), Punctuation::Type::OpenParen));
	tokens.push_back(new Primitive(Slice("int", 3, "", 0, 0), Type::INT));
	tokens.push_back(new Identifier(Slice("x", 1, "", 0, 0)));
	tokens.push_back(new Identifier(Slice("y", 1, "", 0, 0)));
	it = tokens.begin();

	EXPECT_EXIT(p.parseParameters(it, function), ::testing::ExitedWithCode(EXIT_FAILURE),
	      "Unexpected token; expected '\\)' or ','");

	// 6c: Primitive
	tokens = {};
	function = new AST::Function(new AST::AST);
	tokens.push_back(
	      new Punctuation(Slice("(", 1, "", 0, 0), Punctuation::Type::OpenParen));
	tokens.push_back(new Primitive(Slice("int", 3, "", 0, 0), Type::INT));
	tokens.push_back(new Identifier(Slice("x", 1, "", 0, 0)));
	tokens.push_back(new Primitive(Slice("int", 3, "", 0, 0), Type::INT));
	it = tokens.begin();

	EXPECT_EXIT(p.parseParameters(it, function), ::testing::ExitedWithCode(EXIT_FAILURE),
	      "Unexpected token; expected '\\)' or ','");

	// 6d: Keyword
	tokens = {};
	function = new AST::Function(new AST::AST);
	tokens.push_back(
	      new Punctuation(Slice("(", 1, "", 0, 0), Punctuation::Type::OpenParen));
	tokens.push_back(new Primitive(Slice("int", 3, "", 0, 0), Type::INT));
	tokens.push_back(new Identifier(Slice("x", 1, "", 0, 0)));
	tokens.push_back(new Keyword(Slice("return", 6, "", 0, 0), Keyword::Type::RETURN));
	it = tokens.begin();

	EXPECT_EXIT(p.parseParameters(it, function), ::testing::ExitedWithCode(EXIT_FAILURE),
	      "Unexpected token; expected '\\)' or ','");
}

TEST(test_parser, test_parseBlock) {
	class MockParser : public Parser {
	public:
		MOCK_METHOD(AST::Statement *, parseStatement,
		      (std::vector<Token *>::iterator & it, AST::CodeBlock *context));
	};

	std::vector<Token *> tokens;
	std::vector<Token *>::iterator it;
	AST::CodeBlock *context = new AST::CodeBlock(new AST::AST);
	AST::Statement *statement1 = new EmptyStatement;
	AST::Statement *statement2 = new EmptyStatement;
	AST::Statement *statement3 = new EmptyStatement;
	AST::Statement *statement4 = new EmptyStatement;
	AST::Statement *statement5 = new EmptyStatement;
	AST::Statement *statement6 = new EmptyStatement;
	AST::Statement *statement7 = new EmptyStatement;
	AST::Statement *statement8 = new EmptyStatement;

	tokens = {};
	MockParser p;
	tokens.push_back(
	      new Punctuation(Slice("{", 1, "", 0, 0), Punctuation::Type::OpenBrace));
	tokens.push_back(new Identifier(Slice("x", 1, "", 0, 0)));
	tokens.push_back(
	      new Punctuation(Slice("}", 1, "", 0, 0), Punctuation::Type::CloseBrace));
	EXPECT_CALL(p, parseStatement)
	      .WillOnce(::testing::Return(statement1))
	      .WillOnce(::testing::Return(nullptr))
	      .WillOnce(::testing::Return(statement2))
	      .WillOnce(::testing::Return(statement3))
	      .WillOnce(::testing::Return(statement4))
	      .WillOnce(::testing::Return(statement5))
	      .WillOnce(::testing::Return(nullptr))
	      .WillOnce(::testing::Return(nullptr))
	      .WillOnce(::testing::Return(statement6))
	      .WillOnce(::testing::Return(statement7))
	      .WillOnce(::testing::Invoke([statement8](std::vector<Token *>::iterator &it,
	                                        [[maybe_unused]]
	                                        AST::CodeBlock *context) -> AST::Statement * {
		      it++;
		      return statement8;
	      }));
	it = tokens.begin();

	p.parseBlock(it, context);
	EXPECT_EQ(it, tokens.end());
	EXPECT_EQ(context->statements.size(), 8);
	if (context->statements.size() >= 8) {
		EXPECT_EQ(context->statements[0], statement1);
		EXPECT_EQ(context->statements[1], statement2);
		EXPECT_EQ(context->statements[2], statement3);
		EXPECT_EQ(context->statements[3], statement4);
		EXPECT_EQ(context->statements[4], statement5);
		EXPECT_EQ(context->statements[5], statement6);
		EXPECT_EQ(context->statements[6], statement7);
		EXPECT_EQ(context->statements[7], statement8);
	}
}

TEST(test_parser, test_parseBlock_error) {
	std::vector<Token *> tokens;
	std::vector<Token *>::iterator it;
	AST::CodeBlock *context = new AST::CodeBlock(new AST::AST);
	Parser p;

	// Test 1: Missing open brace
	tokens = {};
	tokens.push_back(new Identifier(Slice("x", 1, "", 0, 0)));
	it = tokens.begin();

	EXPECT_EXIT(p.parseBlock(it, context), ::testing::ExitedWithCode(EXIT_FAILURE),
	      "Expected '\\{'");
}

TEST(test_parser, test_parseStatement) {
	class MockParser : public Parser {
	public:
		MOCK_METHOD(AST::rvalue *, parseRvalue,
		      (std::vector<Token *>::iterator & it, AST::CodeBlock *context));
	};

	std::vector<Token *> tokens;
	std::vector<Token *>::iterator it;
	Statement *statement;
	AST::CodeBlock *context = new AST::CodeBlock(new AST::AST);
	rvalue *toReturn;
	Expression *expression;
	Return *returnStatement;

	// Test 1: Regular expression statement
	MockParser p1;
	tokens = {};
	tokens.push_back(new Identifier(Slice("1", 1, "", 0, 0)));
	tokens.push_back(
	      new Punctuation(Slice(";", 1, "", 0, 0), Punctuation::Type::Semicolon));
	toReturn = new EmptyRvalue;
	EXPECT_CALL(p1, parseRvalue)
	      .WillOnce(::testing::Invoke([toReturn](std::vector<Token *>::iterator &it,
	                                        [[maybe_unused]]
	                                        AST::CodeBlock *context) -> rvalue * {
		      it++;
		      return toReturn;
	      }));
	it = tokens.begin();

	statement = p1.parseStatement(it, context);

	EXPECT_EQ(it, tokens.end());
	expression = dynamic_cast<Expression *>(statement);
	EXPECT_NE(expression, nullptr);
	if (expression != nullptr) {
		EXPECT_EQ(expression->rval, toReturn);
	}

	// Test 2: Return (void) statement
	MockParser p2;
	tokens = {};
	tokens.push_back(new Keyword(Slice("return", 6, "", 0, 0), Keyword::Type::RETURN));
	tokens.push_back(
	      new Punctuation(Slice(";", 1, "", 0, 0), Punctuation::Type::Semicolon));
	EXPECT_CALL(p2, parseRvalue).WillOnce(::testing::Return(nullptr));
	it = tokens.begin();

	statement = p2.parseStatement(it, context);

	EXPECT_EQ(it, tokens.end());
	returnStatement = dynamic_cast<Return *>(statement);
	EXPECT_NE(returnStatement, nullptr);
	if (returnStatement != nullptr) {
		EXPECT_EQ(returnStatement->rval, nullptr);
	}

	// Test 3: Return expression statement
	MockParser p3;
	tokens = {};
	tokens.push_back(new Keyword(Slice("return", 6, "", 0, 0), Keyword::Type::RETURN));
	tokens.push_back(new Identifier(Slice("1", 1, "", 0, 0)));
	tokens.push_back(
	      new Punctuation(Slice(";", 1, "", 0, 0), Punctuation::Type::Semicolon));
	toReturn = new EmptyRvalue;
	EXPECT_CALL(p3, parseRvalue)
	      .WillOnce(::testing::Invoke([toReturn](std::vector<Token *>::iterator &it,
	                                        [[maybe_unused]]
	                                        AST::CodeBlock *context) -> rvalue * {
		      it++;
		      return toReturn;
	      }));
	it = tokens.begin();

	statement = p3.parseStatement(it, context);

	EXPECT_EQ(it, tokens.end());
	returnStatement = dynamic_cast<Return *>(statement);
	EXPECT_NE(returnStatement, nullptr);
	if (returnStatement != nullptr) {
		EXPECT_EQ(returnStatement->rval, toReturn);
	}

	// Test 4: Skips extra semicolons
	MockParser p4;
	tokens = {};
	tokens.push_back(
	      new Punctuation(Slice(";", 1, "", 0, 0), Punctuation::Type::Semicolon));
	tokens.push_back(
	      new Punctuation(Slice(";", 1, "", 0, 0), Punctuation::Type::Semicolon));
	tokens.push_back(
	      new Punctuation(Slice(";", 1, "", 0, 0), Punctuation::Type::Semicolon));
	tokens.push_back(new Identifier(Slice("1", 1, "", 0, 0)));
	tokens.push_back(
	      new Punctuation(Slice(";", 1, "", 0, 0), Punctuation::Type::Semicolon));
	toReturn = new EmptyRvalue;
	EXPECT_CALL(p4, parseRvalue)
	      .WillOnce(::testing::Invoke([toReturn](std::vector<Token *>::iterator &it,
	                                        [[maybe_unused]]
	                                        AST::CodeBlock *context) -> rvalue * {
		      it++;
		      return toReturn;
	      }));
	it = tokens.begin();

	statement = p4.parseStatement(it, context);

	EXPECT_EQ(it, tokens.end());
	expression = dynamic_cast<Expression *>(statement);
	EXPECT_NE(expression, nullptr);
	if (expression != nullptr) {
		EXPECT_EQ(dynamic_cast<Expression *>(statement)->rval, toReturn);
	}

	// Test 5: Returns nullptr at close brace
	MockParser p5;
	tokens = {};
	tokens.push_back(
	      new Punctuation(Slice("}", 1, "", 0, 0), Punctuation::Type::CloseBrace));
	EXPECT_CALL(p5, parseRvalue).Times(0);
	it = tokens.begin();

	statement = p5.parseStatement(it, context);

	EXPECT_EQ(it, tokens.begin());
	EXPECT_EQ(statement, nullptr);

	// Test 6: Variable declaration
	MockParser p6;
	tokens = {};
	context = new AST::CodeBlock(new AST::AST);
	tokens.push_back(new Primitive(Slice("int", 3, "", 0, 0), Type::INT));
	tokens.push_back(new Identifier(Slice("x", 1, "", 0, 0)));
	tokens.push_back(
	      new Punctuation(Slice(";", 1, "", 0, 0), Punctuation::Type::Semicolon));
	EXPECT_CALL(p6, parseRvalue)
	      .WillOnce(::testing::Invoke([](std::vector<Token *>::iterator &it,
	                                        [[maybe_unused]]
	                                        AST::CodeBlock *context) -> rvalue * {
		      it++;
		      return new Variable(new Identifier(Slice("x", 1, "", 0, 0)));
	      }));
	it = tokens.begin();

	statement = p6.parseStatement(it, context);

	EXPECT_EQ(it, tokens.end());
	EXPECT_EQ(statement, nullptr);
	EXPECT_NE(context->locals->find(new Identifier(Slice("x", 1, "", 0, 0))),
	      context->locals->end());

	// Test 7: Variable declaration assignment
	MockParser p7;
	tokens = {};
	context = new AST::CodeBlock(new AST::AST);
	tokens.push_back(new Primitive(Slice("int", 3, "", 0, 0), Type::INT));
	tokens.push_back(new Identifier(Slice("x", 1, "", 0, 0)));
	tokens.push_back(
	      new Punctuation(Slice(";", 1, "", 0, 0), Punctuation::Type::Semicolon));
	toReturn = new Assignment(new Identifier(Slice("x", 1, "", 0, 0)),
	      new Literal(new Identifier(Slice("1", 1, "", 0, 0))));
	EXPECT_CALL(p7, parseRvalue)
	      .WillOnce(::testing::Invoke([toReturn](std::vector<Token *>::iterator &it,
	                                        [[maybe_unused]]
	                                        AST::CodeBlock *context) -> rvalue * {
		      it++;
		      return toReturn;
	      }));
	it = tokens.begin();

	statement = p7.parseStatement(it, context);

	EXPECT_EQ(it, tokens.end());
	expression = dynamic_cast<Expression *>(statement);
	EXPECT_NE(expression, nullptr);
	if (expression != nullptr) {
		EXPECT_EQ(expression->rval, toReturn);
	}
	EXPECT_NE(context->locals->find(new Identifier(Slice("x", 1, "", 0, 0))),
	      context->locals->end());
}

TEST(test_parser, test_parseStatement_error) {
	std::vector<Token *> tokens;
	std::vector<Token *>::iterator it;
	AST::CodeBlock *context = new AST::CodeBlock(new AST::AST);
	Parser p;

	// Test 1: Redeclaration of variable of same type
	tokens = {};
	context = new AST::CodeBlock(new AST::AST);
	context->locals->insert({
	      new Identifier(Slice("x", 1, "", 0, 0)), {Type::INT, false}
    });
	tokens.push_back(new Primitive(Slice("int", 3, "", 0, 0), Type::INT));
	tokens.push_back(new Identifier(Slice("x", 1, "", 0, 0)));
	tokens.push_back(
	      new Punctuation(Slice(";", 1, "", 0, 0), Punctuation::Type::Semicolon));
	it = tokens.begin();

	EXPECT_EXIT(p.parseStatement(it, context), ::testing::ExitedWithCode(EXIT_FAILURE),
	      "Re-declaration of variable x");

	// Test 2: Redeclaration of variable with different type
	tokens = {};
	context = new AST::CodeBlock(new AST::AST);
	context->locals->insert({
	      new Identifier(Slice("x", 1, "", 0, 0)), {Type::INT, false}
    });
	tokens.push_back(new Primitive(Slice("float", 5, "", 0, 0), Type::FLOAT));
	tokens.push_back(new Identifier(Slice("x", 1, "", 0, 0)));
	tokens.push_back(
	      new Punctuation(Slice(";", 1, "", 0, 0), Punctuation::Type::Semicolon));
	it = tokens.begin();

	EXPECT_EXIT(p.parseStatement(it, context), ::testing::ExitedWithCode(EXIT_FAILURE),
	      "Re-declaration of variable x");

	// Test 3: Statement is not declaration or declaration assignment
	context = new AST::CodeBlock(new AST::AST);
	// 3a: Literal
	tokens = {};
	tokens.push_back(new Primitive(Slice("int", 3, "", 0, 0), Type::INT));
	tokens.push_back(new Identifier(Slice("1", 1, "", 0, 0)));
	tokens.push_back(
	      new Punctuation(Slice(";", 1, "", 0, 0), Punctuation::Type::Semicolon));
	it = tokens.begin();

	EXPECT_EXIT(p.parseStatement(it, context), ::testing::ExitedWithCode(EXIT_FAILURE),
	      "Unexpected expression following declaration");

	// 3b: Operator
	tokens = {};
	tokens.push_back(new Primitive(Slice("int", 3, "", 0, 0), Type::INT));
	tokens.push_back(new Punctuation(Slice("+", 1, "", 0, 0), Punctuation::Type::Plus));
	tokens.push_back(
	      new Punctuation(Slice(";", 1, "", 0, 0), Punctuation::Type::Semicolon));
	it = tokens.begin();

	EXPECT_EXIT(p.parseStatement(it, context), ::testing::ExitedWithCode(EXIT_FAILURE),
	      "Unexpected token following primitive");

	// 3c: Other punctuation
	tokens = {};
	tokens.push_back(new Primitive(Slice("int", 3, "", 0, 0), Type::INT));
	tokens.push_back(
	      new Punctuation(Slice(";", 1, "", 0, 0), Punctuation::Type::Semicolon));
	it = tokens.begin();

	EXPECT_EXIT(p.parseStatement(it, context), ::testing::ExitedWithCode(EXIT_FAILURE),
	      "Unexpected token following primitive");

	// 3d: Non-assignment expression
	tokens = {};
	tokens.push_back(new Primitive(Slice("int", 3, "", 0, 0), Type::INT));
	tokens.push_back(new Identifier(Slice("x", 1, "", 0, 0)));
	tokens.push_back(new Punctuation(Slice("+", 1, "", 0, 0), Punctuation::Type::Plus));
	tokens.push_back(new Identifier(Slice("1", 1, "", 0, 0)));
	tokens.push_back(
	      new Punctuation(Slice(";", 1, "", 0, 0), Punctuation::Type::Semicolon));
	it = tokens.begin();

	EXPECT_EXIT(p.parseStatement(it, context), ::testing::ExitedWithCode(EXIT_FAILURE),
	      "Unexpected expression following declaration");

	// Test 4: Assignment LHS not variable
	tokens = {};
	tokens.push_back(new Keyword(Slice("return", 6, "", 0, 0), Keyword::Type::RETURN));
	tokens.push_back(new Punctuation(Slice("=", 1, "", 0, 0), Punctuation::Type::Equals));
	it = tokens.begin();

	EXPECT_EXIT(p.parseStatement(it, context), ::testing::ExitedWithCode(EXIT_FAILURE),
	      "LHS of assignment is not a variable");

	tokens = {};
	tokens.push_back(new Identifier(Slice("1", 1, "", 0, 0)));
	tokens.push_back(new Punctuation(Slice("=", 1, "", 0, 0), Punctuation::Type::Equals));
	tokens.push_back(new Identifier(Slice("1", 1, "", 0, 0)));
	tokens.push_back(
	      new Punctuation(Slice(";", 1, "", 0, 0), Punctuation::Type::Semicolon));
	it = tokens.begin();

	// TODO change this to check if the LHS is not a literal - this is currently an e14
	// test here
	EXPECT_EXIT(p.parseStatement(it, context), ::testing::ExitedWithCode(EXIT_FAILURE),
	      "Undeclared variable 1");

	// Test 5: Missing ; after declaration
	tokens = {};
	tokens.push_back(new Primitive(Slice("int", 3, "", 0, 0), Type::INT));
	tokens.push_back(new Identifier(Slice("x", 1, "", 0, 0)));
	tokens.push_back(
	      new Punctuation(Slice("}", 1, "", 0, 0), Punctuation::Type::CloseBrace));
	it = tokens.begin();

	EXPECT_EXIT(p.parseStatement(it, context), ::testing::ExitedWithCode(EXIT_FAILURE),
	      "Expected ';' after statement");

	// Test 6: Missing ; after declaration assignment
	tokens = {};
	tokens.push_back(new Primitive(Slice("int", 3, "", 0, 0), Type::INT));
	tokens.push_back(new Identifier(Slice("x", 1, "", 0, 0)));
	tokens.push_back(new Punctuation(Slice("=", 1, "", 0, 0), Punctuation::Type::Equals));
	tokens.push_back(new Identifier(Slice("1", 1, "", 0, 0)));
	tokens.push_back(
	      new Punctuation(Slice("}", 1, "", 0, 0), Punctuation::Type::CloseBrace));
	it = tokens.begin();

	EXPECT_EXIT(p.parseStatement(it, context), ::testing::ExitedWithCode(EXIT_FAILURE),
	      "Expected ';' after statement");

	// Test 7: Missing ; after expression statement
	tokens = {};
	context->locals->insert({
	      new Identifier(Slice("x", 1, "", 0, 0)), {Type::INT, false}
    });
	tokens.push_back(new Identifier(Slice("x", 1, "", 0, 0)));
	tokens.push_back(new Punctuation(Slice("=", 1, "", 0, 0), Punctuation::Type::Equals));
	tokens.push_back(new Identifier(Slice("1", 1, "", 0, 0)));
	tokens.push_back(
	      new Punctuation(Slice("}", 1, "", 0, 0), Punctuation::Type::CloseBrace));
	it = tokens.begin();

	EXPECT_EXIT(p.parseStatement(it, context), ::testing::ExitedWithCode(EXIT_FAILURE),
	      "Expected ';' after statement");

	// Test 8: Missing ; after return statement
	tokens = {};
	tokens.push_back(new Keyword(Slice("return", 6, "", 0, 0), Keyword::Type::RETURN));
	tokens.push_back(
	      new Punctuation(Slice("}", 1, "", 0, 0), Punctuation::Type::CloseBrace));
	it = tokens.begin();

	EXPECT_EXIT(p.parseStatement(it, context), ::testing::ExitedWithCode(EXIT_FAILURE),
	      "Expected ';' after statement");
}

TEST(test_parser, test_e0) {
	Parser p;
	std::vector<Token *> tokens;
	const char *identifier;
	Slice s = {"", static_cast<size_t>(0), "", 0, 0};
	std::vector<Token *>::iterator it;
	rvalue *rval;
	Variable *var;
	char *copy;
	Literal *val;

	// Test 1: All alphabetical identifier
	tokens = {};
	identifier = "xyz";
	s = Slice(identifier, strlen(identifier), "", 0, 0);
	tokens.push_back(new Identifier(s));
	it = tokens.begin();
	rval = p.e0(it);
	EXPECT_EQ(it, tokens.end());
	var = dynamic_cast<Variable *>(rval);
	EXPECT_NE(var, nullptr);
	if (var != nullptr) {
		copy = new char[strlen(identifier) + 1];
		strcpy(copy, identifier);
		EXPECT_EQ(var->variable->s, copy);
		delete[] copy;
	}

	// Test 2: All numerical identifier
	tokens = {};
	identifier = "123";
	s = Slice(identifier, strlen(identifier), "", 0, 0);
	tokens.push_back(new Identifier(s));
	it = tokens.begin();
	rval = p.e0(it);
	EXPECT_EQ(it, tokens.end());
	val = dynamic_cast<Literal *>(rval);
	EXPECT_NE(val, nullptr);
	if (val != nullptr) {
		EXPECT_EQ(val->value, 123);
	}

	// Test 3: Alphanumeric identifier
	tokens = {};
	identifier = "abc123";
	s = Slice(identifier, strlen(identifier), "", 0, 0);
	tokens.push_back(new Identifier(s));
	it = tokens.begin();
	rval = p.e0(it);
	EXPECT_EQ(it, tokens.end());
	var = dynamic_cast<Variable *>(rval);
	EXPECT_NE(var, nullptr);
	if (var != nullptr) {
		copy = new char[strlen(identifier) + 1];
		strcpy(copy, identifier);
		EXPECT_EQ(var->variable->s, copy);
		delete[] copy;
	}

	// Test 4: Not an identifier
	tokens = {};
	identifier = "int";
	s = Slice(identifier, strlen(identifier), "", 0, 0);
	tokens.push_back(new Primitive(s, Type::INT));
	it = tokens.begin();
	rval = p.e0(it);
	// Should not have advanced the iterator if it was not an identifier
	EXPECT_EQ(it, tokens.begin());
	EXPECT_EQ(rval, nullptr);
}

TEST(test_parser, test_e1) {
	class MockParser : public Parser {
	public:
		MOCK_METHOD(AST::rvalue *, e0, (std::vector<Token *>::iterator & it));
		MOCK_METHOD(AST::rvalue *, parseRvalue,
		      (std::vector<Token *>::iterator & it, AST::CodeBlock *context));
	};

	std::vector<Token *> tokens;
	std::vector<Token *>::iterator it;
	rvalue *rval;
	AST::CodeBlock *context = new AST::CodeBlock(new AST::AST);
	Variable *functionName;
	FunctionCall *call;
	rvalue *argument;
	rvalue *argument1;
	rvalue *argument2;
	rvalue *argument3;

	// Test 1: Parenthesized expressions call parseRvalue and return what they receive
	// from it
	MockParser p1;
	tokens = {};
	EXPECT_CALL(p1, e0).WillOnce(::testing::Return(nullptr));
	tokens.push_back(
	      new Punctuation(Slice("(", 1, "", 0, 0), Punctuation::Type::OpenParen));
	rvalue *toReturn = new EmptyRvalue;
	EXPECT_CALL(p1, parseRvalue).WillOnce(::testing::Return(toReturn));
	tokens.push_back(
	      new Punctuation(Slice(")", 1, "", 0, 0), Punctuation::Type::CloseParen));

	it = tokens.begin();

	rval = p1.e1(it, context);

	EXPECT_EQ(it, tokens.end());
	EXPECT_EQ(rval, toReturn);

	// Test 2: Function calls do not need arguments
	MockParser p2;
	tokens = {};
	functionName = new Variable(new Identifier(Slice("function", 8, "", 0, 0)));
	EXPECT_CALL(p2, e0).WillOnce(::testing::Return(functionName));
	tokens.push_back(
	      new Punctuation(Slice("(", 1, "", 0, 0), Punctuation::Type::OpenParen));
	EXPECT_CALL(p2, parseRvalue).WillOnce(::testing::Return(nullptr));
	tokens.push_back(
	      new Punctuation(Slice(")", 1, "", 0, 0), Punctuation::Type::CloseParen));
	it = tokens.begin();

	rval = p2.e1(it, context);

	EXPECT_EQ(it, tokens.end());
	call = dynamic_cast<FunctionCall *>(rval);
	EXPECT_NE(call, nullptr);
	if (call != nullptr) {
		EXPECT_EQ(call->name, functionName);
		EXPECT_EQ(call->arguments.size(), 0);
	}

	// Test 3: Function calls may take an argument
	MockParser p3;
	tokens = {};
	functionName = new Variable(new Identifier(Slice("function", 8, "", 0, 0)));
	EXPECT_CALL(p3, e0).WillOnce(::testing::Return(functionName));
	tokens.push_back(
	      new Punctuation(Slice("(", 1, "", 0, 0), Punctuation::Type::OpenParen));
	argument = new EmptyRvalue;
	EXPECT_CALL(p3, parseRvalue).WillOnce(::testing::Return(argument));
	tokens.push_back(
	      new Punctuation(Slice(")", 1, "", 0, 0), Punctuation::Type::CloseParen));
	it = tokens.begin();

	rval = p3.e1(it, context);

	EXPECT_EQ(it, tokens.end());
	call = dynamic_cast<FunctionCall *>(rval);
	EXPECT_NE(call, nullptr);
	if (call != nullptr) {
		EXPECT_EQ(call->name, functionName);
		EXPECT_EQ(call->arguments.size(), 1);
		if (call->arguments.size() >= 1) {
			EXPECT_EQ(call->arguments[0], argument);
		}
	}

	// Test 4: Function calls may take multiple arguments
	MockParser p4_1;
	tokens = {};
	functionName = new Variable(new Identifier(Slice("function", 8, "", 0, 0)));
	EXPECT_CALL(p4_1, e0).WillOnce(::testing::Return(functionName));
	tokens.push_back(
	      new Punctuation(Slice("(", 1, "", 0, 0), Punctuation::Type::OpenParen));
	argument1 = new EmptyRvalue;
	argument2 = new EmptyRvalue;
	EXPECT_CALL(p4_1, parseRvalue)
	      .WillOnce(::testing::Return(argument1))
	      .WillOnce(::testing::Return(argument2));
	tokens.push_back(new Punctuation(Slice(",", 1, "", 0, 0), Punctuation::Type::Comma));
	tokens.push_back(
	      new Punctuation(Slice(")", 1, "", 0, 0), Punctuation::Type::CloseParen));
	it = tokens.begin();

	rval = p4_1.e1(it, context);

	EXPECT_EQ(it, tokens.end());
	call = dynamic_cast<FunctionCall *>(rval);
	EXPECT_NE(call, nullptr);
	if (call != nullptr) {
		EXPECT_EQ(call->name, functionName);
		EXPECT_EQ(call->arguments.size(), 2);
		if (call->arguments.size() >= 2) {
			EXPECT_EQ(call->arguments[0], argument1);
			EXPECT_EQ(call->arguments[1], argument2);
		}
	}

	MockParser p4_2;
	tokens = {};
	functionName = new Variable(new Identifier(Slice("function", 8, "", 0, 0)));
	EXPECT_CALL(p4_2, e0).WillOnce(::testing::Return(functionName));
	tokens.push_back(
	      new Punctuation(Slice("(", 1, "", 0, 0), Punctuation::Type::OpenParen));
	argument1 = new EmptyRvalue;
	argument2 = new EmptyRvalue;
	argument3 = new EmptyRvalue;
	EXPECT_CALL(p4_2, parseRvalue)
	      .WillOnce(::testing::Return(argument1))
	      .WillOnce(::testing::Return(argument2))
	      .WillOnce(::testing::Return(argument3));
	tokens.push_back(new Punctuation(Slice(",", 1, "", 0, 0), Punctuation::Type::Comma));
	tokens.push_back(new Punctuation(Slice(",", 1, "", 0, 0), Punctuation::Type::Comma));
	tokens.push_back(
	      new Punctuation(Slice(")", 1, "", 0, 0), Punctuation::Type::CloseParen));
	it = tokens.begin();

	rval = p4_2.e1(it, context);

	EXPECT_EQ(it, tokens.end());
	call = dynamic_cast<FunctionCall *>(rval);
	if (call != nullptr) {
		EXPECT_EQ(call->name, functionName);
		EXPECT_EQ(call->arguments.size(), 3);
		if (call->arguments.size() >= 3) {
			EXPECT_EQ(call->arguments[0], argument1);
			EXPECT_EQ(call->arguments[1], argument2);
			EXPECT_EQ(call->arguments[2], argument3);
		}
	}
}

TEST(test_parser, test_e1_error) {
	std::vector<Token *> tokens;
	std::vector<Token *>::iterator it;
	AST::CodeBlock *context;
	Parser p;

	// Test 1: Using variable as function call
	tokens = {};
	context = new AST::CodeBlock(new AST::AST);
	context->locals->insert({
	      new Identifier(Slice("var", 3, "", 0, 0)), {Type::INT, true}
    });
	tokens.push_back(new Identifier(Slice("var", 3, "", 0, 0)));
	tokens.push_back(
	      new Punctuation(Slice("(", 1, "", 0, 0), Punctuation::Type::OpenParen));
	tokens.push_back(
	      new Punctuation(Slice(")", 1, "", 0, 0), Punctuation::Type::CloseParen));
	it = tokens.begin();

	EXPECT_EXIT(p.e1(it, context), ::testing::ExitedWithCode(EXIT_FAILURE),
	      "var is not callable");

	// Test 2: Using function name as variable
	AST::AST *ast = new AST::AST;
	context = new AST::CodeBlock(ast);
	Function *foo = new Function(ast);
	ast->functions.insert({"foo", foo});
	tokens = {};
	tokens.push_back(new Identifier(Slice("foo", 3, "", 0, 0)));
	tokens.push_back(
	      new Punctuation(Slice(";", 1, "", 0, 0), Punctuation::Type::Semicolon));
	it = tokens.begin();

	EXPECT_EXIT(p.e1(it, context), ::testing::ExitedWithCode(EXIT_FAILURE),
	      "foo is not a variable");

	// Test 3: Empty parenthesis for non-function call
	tokens = {};
	context = new AST::CodeBlock(new AST::AST);
	tokens.push_back(
	      new Punctuation(Slice("(", 1, "", 0, 0), Punctuation::Type::OpenParen));
	tokens.push_back(
	      new Punctuation(Slice(")", 1, "", 0, 0), Punctuation::Type::CloseParen));
	it = tokens.begin();

	EXPECT_EXIT(p.e1(it, context), ::testing::ExitedWithCode(EXIT_FAILURE),
	      "Expected expression");

	// Test 4: Unmatched parenthesis in grouping
	tokens = {};
	context = new AST::CodeBlock(new AST::AST);
	tokens.push_back(
	      new Punctuation(Slice("(", 1, "", 0, 0), Punctuation::Type::OpenParen));
	tokens.push_back(new Identifier(Slice("var", 3, "", 0, 0)));
	tokens.push_back(
	      new Punctuation(Slice(";", 1, "", 0, 0), Punctuation::Type::Semicolon));
	it = tokens.begin();

	EXPECT_EXIT(p.e1(it, context), ::testing::ExitedWithCode(EXIT_FAILURE),
	      "Expected ')'");

	// Test 5: Unmatched parenthesis in function call
	tokens = {};
	context = new AST::CodeBlock(new AST::AST);
	tokens.push_back(new Identifier(Slice("foo", 3, "", 0, 0)));
	tokens.push_back(
	      new Punctuation(Slice("(", 1, "", 0, 0), Punctuation::Type::OpenParen));
	tokens.push_back(
	      new Punctuation(Slice(";", 1, "", 0, 0), Punctuation::Type::Semicolon));
	it = tokens.begin();

	EXPECT_EXIT(p.e1(it, context), ::testing::ExitedWithCode(EXIT_FAILURE),
	      "Expected ')'");
}

TEST(test_parser, test_e2) {
	std::vector<Token *> tokens;
	std::vector<Token *>::iterator it;
	rvalue *rval;
	AST::CodeBlock *context = new AST::CodeBlock(new AST::AST);

	class MockParser : public Parser {
	public:
		MOCK_METHOD(AST::rvalue *, e1,
		      (std::vector<Token *>::iterator & it, AST::CodeBlock *context));
	};

	MockParser p;
	rvalue *toReturn = new EmptyRvalue;
	EXPECT_CALL(p, e1).WillOnce(::testing::Return(toReturn));
	it = tokens.begin();
	rval = p.e2(it, context);
	EXPECT_EQ(it, tokens.begin());
	EXPECT_EQ(rval, toReturn);
}

TEST(test_parser, test_e3) {
	std::vector<Token *> tokens;
	std::vector<Token *>::iterator it;
	rvalue *rval;
	AST::CodeBlock *context = new AST::CodeBlock(new AST::AST);
	Multiplication *multiplication;
	Division *division;
	Modulo *modulo;
	rvalue *operand1;
	rvalue *operand2;
	rvalue *operand3;
	rvalue *operand4;
	rvalue *operand5;
	rvalue *operand6;
	rvalue *operand7;
	rvalue *operand8;
	rvalue *toReturn;

	class MockParser : public Parser {
	public:
		MOCK_METHOD(AST::rvalue *, e2,
		      (std::vector<Token *>::iterator & it, AST::CodeBlock *context));
	};

	// Test 1: Multiplication
	MockParser p1;
	tokens = {};
	tokens.push_back(new Punctuation(Slice("*", 1, "", 0, 0), Punctuation::Type::Times));
	operand1 = new EmptyRvalue;
	operand2 = new EmptyRvalue;
	EXPECT_CALL(p1, e2)
	      .WillOnce(::testing::Return(operand1))
	      .WillOnce(::testing::Return(operand2));
	it = tokens.begin();

	rval = p1.e3(it, context);

	EXPECT_EQ(it, tokens.end());
	multiplication = dynamic_cast<Multiplication *>(rval);
	EXPECT_NE(multiplication, nullptr);
	if (multiplication != nullptr) {
		EXPECT_EQ(multiplication->operand1, operand1);
		EXPECT_EQ(multiplication->operand2, operand2);
	}

	// Test 2: Subtraction
	MockParser p2;
	tokens = {};
	tokens.push_back(new Punctuation(Slice("/", 1, "", 0, 0), Punctuation::Type::Divide));
	operand1 = new EmptyRvalue;
	operand2 = new EmptyRvalue;
	EXPECT_CALL(p2, e2)
	      .WillOnce(::testing::Return(operand1))
	      .WillOnce(::testing::Return(operand2));
	it = tokens.begin();

	rval = p2.e3(it, context);

	EXPECT_EQ(it, tokens.end());
	division = dynamic_cast<Division *>(rval);
	EXPECT_NE(division, nullptr);
	if (division != nullptr) {
		EXPECT_EQ(division->operand1, operand1);
		EXPECT_EQ(division->operand2, operand2);
	}

	// Test 3: Modulo
	MockParser p3;
	tokens = {};
	tokens.push_back(new Punctuation(Slice("%", 1, "", 0, 0), Punctuation::Type::Mod));
	operand1 = new EmptyRvalue;
	operand2 = new EmptyRvalue;
	EXPECT_CALL(p3, e2)
	      .WillOnce(::testing::Return(operand1))
	      .WillOnce(::testing::Return(operand2));
	it = tokens.begin();

	rval = p3.e3(it, context);

	EXPECT_EQ(it, tokens.end());
	modulo = dynamic_cast<Modulo *>(rval);
	EXPECT_NE(modulo, nullptr);
	if (modulo != nullptr) {
		EXPECT_EQ(modulo->operand1, operand1);
		EXPECT_EQ(modulo->operand2, operand2);
	}

	// Test 4: Multiple Operators
	MockParser p4;
	tokens = {};
	tokens.push_back(new Punctuation(Slice("*", 1, "", 0, 0), Punctuation::Type::Times));
	tokens.push_back(new Punctuation(Slice("/", 1, "", 0, 0), Punctuation::Type::Divide));
	tokens.push_back(new Punctuation(Slice("/", 1, "", 0, 0), Punctuation::Type::Divide));
	tokens.push_back(new Punctuation(Slice("%", 1, "", 0, 0), Punctuation::Type::Mod));
	tokens.push_back(new Punctuation(Slice("*", 1, "", 0, 0), Punctuation::Type::Times));
	tokens.push_back(new Punctuation(Slice("%", 1, "", 0, 0), Punctuation::Type::Mod));
	tokens.push_back(new Punctuation(Slice("/", 1, "", 0, 0), Punctuation::Type::Divide));
	tokens.push_back(
	      new Punctuation(Slice(";", 1, "", 0, 0), Punctuation::Type::Semicolon));
	operand1 = new EmptyRvalue;
	operand2 = new EmptyRvalue;
	operand3 = new EmptyRvalue;
	operand4 = new EmptyRvalue;
	operand5 = new EmptyRvalue;
	operand6 = new EmptyRvalue;
	operand7 = new EmptyRvalue;
	operand8 = new EmptyRvalue;
	EXPECT_CALL(p4, e2)
	      .WillOnce(::testing::Return(operand1))
	      .WillOnce(::testing::Return(operand2))
	      .WillOnce(::testing::Return(operand3))
	      .WillOnce(::testing::Return(operand4))
	      .WillOnce(::testing::Return(operand5))
	      .WillOnce(::testing::Return(operand6))
	      .WillOnce(::testing::Return(operand7))
	      .WillOnce(::testing::Return(operand8));
	it = tokens.begin();

	rval = p4.e4(it, context);

	EXPECT_EQ(it + 1, tokens.end());
	division = dynamic_cast<Division *>(rval);
	EXPECT_NE(division, nullptr);
	if (division != nullptr) {
		EXPECT_EQ(division->operand2, operand8);
		modulo = dynamic_cast<Modulo *>(division->operand1);
		EXPECT_NE(modulo, nullptr);
		if (modulo != nullptr) {
			EXPECT_EQ(modulo->operand2, operand7);
			multiplication = dynamic_cast<Multiplication *>(modulo->operand1);
			EXPECT_NE(multiplication, nullptr);
			if (multiplication != nullptr) {
				EXPECT_EQ(multiplication->operand2, operand6);
				modulo = dynamic_cast<Modulo *>(multiplication->operand1);
				EXPECT_NE(modulo, nullptr);
				if (modulo != nullptr) {
					EXPECT_EQ(modulo->operand2, operand5);
					division = dynamic_cast<Division *>(modulo->operand1);
					EXPECT_NE(division, nullptr);
					if (division != nullptr) {
						EXPECT_EQ(division->operand2, operand4);
						division = dynamic_cast<Division *>(division->operand1);
						EXPECT_NE(division, nullptr);
						if (division != nullptr) {
							EXPECT_EQ(division->operand2, operand3);
							multiplication
							      = dynamic_cast<Multiplication *>(division->operand1);
							EXPECT_NE(multiplication, nullptr);
							if (multiplication != nullptr) {
								EXPECT_EQ(multiplication->operand2, operand2);
								EXPECT_EQ(multiplication->operand1, operand1);
							}
						}
					}
				}
			}
		}
	}

	// Test 5: Other punctuation
	MockParser p5;
	tokens = {};
	tokens.push_back(
	      new Punctuation(Slice(";", 1, "", 0, 0), Punctuation::Type::Semicolon));
	toReturn = new EmptyRvalue;
	EXPECT_CALL(p5, e2).WillOnce(::testing::Return(toReturn));
	it = tokens.begin();

	rval = p5.e3(it, context);

	EXPECT_EQ(it, tokens.begin());
	EXPECT_EQ(rval, toReturn);

	// Test 6: Non-punctuation
	MockParser p6;
	tokens = {};
	tokens.push_back(new Identifier(Slice("x", 1, "", 0, 0)));
	toReturn = new EmptyRvalue;
	EXPECT_CALL(p6, e2).WillOnce(::testing::Return(toReturn));
	it = tokens.begin();

	rval = p6.e3(it, context);

	EXPECT_EQ(it, tokens.begin());
	EXPECT_EQ(rval, toReturn);
}

TEST(test_parser, test_e4) {
	std::vector<Token *> tokens;
	std::vector<Token *>::iterator it;
	rvalue *rval;
	AST::CodeBlock *context = new AST::CodeBlock(new AST::AST);
	Addition *addition;
	Subtraction *subtraction;
	rvalue *operand1;
	rvalue *operand2;
	rvalue *operand3;
	rvalue *operand4;
	rvalue *operand5;
	rvalue *operand6;
	rvalue *operand7;
	rvalue *operand8;
	rvalue *toReturn;

	class MockParser : public Parser {
	public:
		MOCK_METHOD(AST::rvalue *, e3,
		      (std::vector<Token *>::iterator & it, AST::CodeBlock *context));
	};

	// Test 1: Addition
	MockParser p1;
	tokens = {};
	tokens.push_back(new Punctuation(Slice("+", 1, "", 0, 0), Punctuation::Type::Plus));
	operand1 = new EmptyRvalue;
	operand2 = new EmptyRvalue;
	EXPECT_CALL(p1, e3)
	      .WillOnce(::testing::Return(operand1))
	      .WillOnce(::testing::Return(operand2));
	it = tokens.begin();

	rval = p1.e4(it, context);

	EXPECT_EQ(it, tokens.end());
	addition = dynamic_cast<Addition *>(rval);
	EXPECT_NE(addition, nullptr);
	if (addition != nullptr) {
		EXPECT_EQ(addition->operand1, operand1);
		EXPECT_EQ(addition->operand2, operand2);
	}

	// Test 2: Subtraction
	MockParser p2;
	tokens = {};
	tokens.push_back(new Punctuation(Slice("-", 1, "", 0, 0), Punctuation::Type::Minus));
	operand1 = new EmptyRvalue;
	operand2 = new EmptyRvalue;
	EXPECT_CALL(p2, e3)
	      .WillOnce(::testing::Return(operand1))
	      .WillOnce(::testing::Return(operand2));
	it = tokens.begin();

	rval = p2.e4(it, context);

	EXPECT_EQ(it, tokens.end());
	subtraction = dynamic_cast<Subtraction *>(rval);
	EXPECT_NE(subtraction, nullptr);
	if (subtraction != nullptr) {
		EXPECT_EQ(subtraction->operand1, operand1);
		EXPECT_EQ(subtraction->operand2, operand2);
	}

	// Test 3: Multiple Operators
	MockParser p3;
	tokens = {};
	tokens.push_back(new Punctuation(Slice("+", 1, "", 0, 0), Punctuation::Type::Plus));
	tokens.push_back(new Punctuation(Slice("-", 1, "", 0, 0), Punctuation::Type::Minus));
	tokens.push_back(new Punctuation(Slice("-", 1, "", 0, 0), Punctuation::Type::Minus));
	tokens.push_back(new Punctuation(Slice("+", 1, "", 0, 0), Punctuation::Type::Plus));
	tokens.push_back(new Punctuation(Slice("+", 1, "", 0, 0), Punctuation::Type::Plus));
	tokens.push_back(new Punctuation(Slice("-", 1, "", 0, 0), Punctuation::Type::Minus));
	tokens.push_back(new Punctuation(Slice("+", 1, "", 0, 0), Punctuation::Type::Plus));
	tokens.push_back(
	      new Punctuation(Slice(";", 1, "", 0, 0), Punctuation::Type::Semicolon));
	operand1 = new EmptyRvalue;
	operand2 = new EmptyRvalue;
	operand3 = new EmptyRvalue;
	operand4 = new EmptyRvalue;
	operand5 = new EmptyRvalue;
	operand6 = new EmptyRvalue;
	operand7 = new EmptyRvalue;
	operand8 = new EmptyRvalue;
	EXPECT_CALL(p3, e3)
	      .WillOnce(::testing::Return(operand1))
	      .WillOnce(::testing::Return(operand2))
	      .WillOnce(::testing::Return(operand3))
	      .WillOnce(::testing::Return(operand4))
	      .WillOnce(::testing::Return(operand5))
	      .WillOnce(::testing::Return(operand6))
	      .WillOnce(::testing::Return(operand7))
	      .WillOnce(::testing::Return(operand8));
	it = tokens.begin();

	rval = p3.e4(it, context);

	EXPECT_EQ(it + 1, tokens.end());
	addition = dynamic_cast<Addition *>(rval);
	EXPECT_NE(addition, nullptr);
	if (addition != nullptr) {
		EXPECT_EQ(addition->operand2, operand8);
		subtraction = dynamic_cast<Subtraction *>(addition->operand1);
		EXPECT_NE(subtraction, nullptr);
		if (subtraction != nullptr) {
			EXPECT_EQ(subtraction->operand2, operand7);
			addition = dynamic_cast<Addition *>(subtraction->operand1);
			EXPECT_NE(addition, nullptr);
			if (addition != nullptr) {
				EXPECT_EQ(addition->operand2, operand6);
				addition = dynamic_cast<Addition *>(addition->operand1);
				EXPECT_NE(addition, nullptr);
				if (addition != nullptr) {
					EXPECT_EQ(addition->operand2, operand5);
					subtraction = dynamic_cast<Subtraction *>(addition->operand1);
					EXPECT_NE(subtraction, nullptr);
					if (subtraction != nullptr) {
						EXPECT_EQ(subtraction->operand2, operand4);
						subtraction = dynamic_cast<Subtraction *>(subtraction->operand1);
						EXPECT_NE(subtraction, nullptr);
						if (subtraction != nullptr) {
							EXPECT_EQ(subtraction->operand2, operand3);
							addition = dynamic_cast<Addition *>(subtraction->operand1);
							EXPECT_NE(addition, nullptr);
							if (addition != nullptr) {
								EXPECT_EQ(addition->operand2, operand2);
								EXPECT_EQ(addition->operand1, operand1);
							}
						}
					}
				}
			}
		}
	}

	// Test 4: Other punctuation
	MockParser p4;
	tokens = {};
	tokens.push_back(
	      new Punctuation(Slice(";", 1, "", 0, 0), Punctuation::Type::Semicolon));
	toReturn = new EmptyRvalue;
	EXPECT_CALL(p4, e3).WillOnce(::testing::Return(toReturn));
	it = tokens.begin();

	rval = p4.e4(it, context);

	EXPECT_EQ(it, tokens.begin());
	EXPECT_EQ(rval, toReturn);

	// Test 5: Non-punctuation
	MockParser p5;
	tokens = {};
	tokens.push_back(new Identifier(Slice("x", 1, "", 0, 0)));
	toReturn = new EmptyRvalue;
	EXPECT_CALL(p5, e3).WillOnce(::testing::Return(toReturn));
	it = tokens.begin();

	rval = p5.e4(it, context);

	EXPECT_EQ(it, tokens.begin());
	EXPECT_EQ(rval, toReturn);
}

TEST(test_parser, test_e5) {
	std::vector<Token *> tokens;
	std::vector<Token *>::iterator it;
	rvalue *rval;
	AST::CodeBlock *context = new AST::CodeBlock(new AST::AST);

	class MockParser : public Parser {
	public:
		MOCK_METHOD(AST::rvalue *, e4,
		      (std::vector<Token *>::iterator & it, AST::CodeBlock *context));
	};

	MockParser p;
	rvalue *toReturn = new EmptyRvalue;
	EXPECT_CALL(p, e4).WillOnce(::testing::Return(toReturn));
	it = tokens.begin();
	rval = p.e5(it, context);
	EXPECT_EQ(it, tokens.begin());
	EXPECT_EQ(rval, toReturn);
}

TEST(test_parser, test_e6) {
	std::vector<Token *> tokens;
	std::vector<Token *>::iterator it;
	rvalue *rval;
	AST::CodeBlock *context = new AST::CodeBlock(new AST::AST);

	class MockParser : public Parser {
	public:
		MOCK_METHOD(AST::rvalue *, e5,
		      (std::vector<Token *>::iterator & it, AST::CodeBlock *context));
	};

	MockParser p;
	rvalue *toReturn = new EmptyRvalue;
	EXPECT_CALL(p, e5).WillOnce(::testing::Return(toReturn));
	it = tokens.begin();
	rval = p.e6(it, context);
	EXPECT_EQ(it, tokens.begin());
	EXPECT_EQ(rval, toReturn);
}

TEST(test_parser, test_e7) {
	std::vector<Token *> tokens;
	std::vector<Token *>::iterator it;
	rvalue *rval;
	AST::CodeBlock *context = new AST::CodeBlock(new AST::AST);

	class MockParser : public Parser {
	public:
		MOCK_METHOD(AST::rvalue *, e6,
		      (std::vector<Token *>::iterator & it, AST::CodeBlock *context));
	};

	MockParser p;
	rvalue *toReturn = new EmptyRvalue;
	EXPECT_CALL(p, e6).WillOnce(::testing::Return(toReturn));
	it = tokens.begin();
	rval = p.e7(it, context);
	EXPECT_EQ(it, tokens.begin());
	EXPECT_EQ(rval, toReturn);
}

TEST(test_parser, test_e8) {
	std::vector<Token *> tokens;
	std::vector<Token *>::iterator it;
	rvalue *rval;
	AST::CodeBlock *context = new AST::CodeBlock(new AST::AST);

	class MockParser : public Parser {
	public:
		MOCK_METHOD(AST::rvalue *, e7,
		      (std::vector<Token *>::iterator & it, AST::CodeBlock *context));
	};

	MockParser p;
	rvalue *toReturn = new EmptyRvalue;
	EXPECT_CALL(p, e7).WillOnce(::testing::Return(toReturn));
	it = tokens.begin();
	rval = p.e8(it, context);
	EXPECT_EQ(it, tokens.begin());
	EXPECT_EQ(rval, toReturn);
}

TEST(test_parser, test_e9) {
	std::vector<Token *> tokens;
	std::vector<Token *>::iterator it;
	rvalue *rval;
	AST::CodeBlock *context = new AST::CodeBlock(new AST::AST);

	class MockParser : public Parser {
	public:
		MOCK_METHOD(AST::rvalue *, e8,
		      (std::vector<Token *>::iterator & it, AST::CodeBlock *context));
	};

	MockParser p;
	rvalue *toReturn = new EmptyRvalue;
	EXPECT_CALL(p, e8).WillOnce(::testing::Return(toReturn));
	it = tokens.begin();
	rval = p.e9(it, context);
	EXPECT_EQ(it, tokens.begin());
	EXPECT_EQ(rval, toReturn);
}

TEST(test_parser, test_e10) {
	std::vector<Token *> tokens;
	std::vector<Token *>::iterator it;
	rvalue *rval;
	AST::CodeBlock *context = new AST::CodeBlock(new AST::AST);

	class MockParser : public Parser {
	public:
		MOCK_METHOD(AST::rvalue *, e9,
		      (std::vector<Token *>::iterator & it, AST::CodeBlock *context));
	};

	MockParser p;
	rvalue *toReturn = new EmptyRvalue;
	EXPECT_CALL(p, e9).WillOnce(::testing::Return(toReturn));
	it = tokens.begin();
	rval = p.e10(it, context);
	EXPECT_EQ(it, tokens.begin());
	EXPECT_EQ(rval, toReturn);
}

TEST(test_parser, test_e11) {
	std::vector<Token *> tokens;
	std::vector<Token *>::iterator it;
	rvalue *rval;
	AST::CodeBlock *context = new AST::CodeBlock(new AST::AST);

	class MockParser : public Parser {
	public:
		MOCK_METHOD(AST::rvalue *, e10,
		      (std::vector<Token *>::iterator & it, AST::CodeBlock *context));
	};

	MockParser p;
	rvalue *toReturn = new EmptyRvalue;
	EXPECT_CALL(p, e10).WillOnce(::testing::Return(toReturn));
	it = tokens.begin();
	rval = p.e11(it, context);
	EXPECT_EQ(it, tokens.begin());
	EXPECT_EQ(rval, toReturn);
}

TEST(test_parser, test_e12) {
	std::vector<Token *> tokens;
	std::vector<Token *>::iterator it;
	rvalue *rval;
	AST::CodeBlock *context = new AST::CodeBlock(new AST::AST);

	class MockParser : public Parser {
	public:
		MOCK_METHOD(AST::rvalue *, e11,
		      (std::vector<Token *>::iterator & it, AST::CodeBlock *context));
	};

	MockParser p;
	rvalue *toReturn = new EmptyRvalue;
	EXPECT_CALL(p, e11).WillOnce(::testing::Return(toReturn));
	it = tokens.begin();
	rval = p.e12(it, context);
	EXPECT_EQ(it, tokens.begin());
	EXPECT_EQ(rval, toReturn);
}

TEST(test_parser, test_e13) {
	std::vector<Token *> tokens;
	std::vector<Token *>::iterator it;
	rvalue *rval;
	AST::CodeBlock *context = new AST::CodeBlock(new AST::AST);

	class MockParser : public Parser {
	public:
		MOCK_METHOD(AST::rvalue *, e12,
		      (std::vector<Token *>::iterator & it, AST::CodeBlock *context));
	};

	MockParser p;
	rvalue *toReturn = new EmptyRvalue;
	EXPECT_CALL(p, e12).WillOnce(::testing::Return(toReturn));
	it = tokens.begin();
	rval = p.e13(it, context);
	EXPECT_EQ(it, tokens.begin());
	EXPECT_EQ(rval, toReturn);
}

TEST(test_parser, test_e14) {
	class MockParser : public Parser {
	public:
		MOCK_METHOD(AST::rvalue *, e13,
		      (std::vector<Token *>::iterator & it, AST::CodeBlock *context));
		MOCK_METHOD(AST::rvalue *, e14,
		      (std::vector<Token *>::iterator & it, AST::CodeBlock *context));
	};

	std::vector<Token *> tokens;
	std::vector<Token *>::iterator it;
	rvalue *rval;
	AST::CodeBlock *context = new AST::CodeBlock(new AST::AST);
	Assignment *assignment;
	rvalue *toReturn;

	// Test 1: Assignment
	MockParser p1;
	tokens = {};
	tokens.push_back(new Identifier(Slice("x", 1, "", 0, 0)));
	tokens.push_back(new Punctuation(Slice("=", 1, "", 0, 0), Punctuation::Type::Equals));
	toReturn = new EmptyRvalue;
	context->locals->insert({
	      new Identifier(Slice("x", 1, "", 0, 0)), {Type::INT, true}
    });
	EXPECT_CALL(p1, e14)
	      // Test multiple assignment for free thanks to recursion here
	      .WillOnce(::testing::Invoke([&p1](std::vector<Token *>::iterator &it,
	                                        AST::CodeBlock *context) -> rvalue * {
		      return p1.Parser::e14(it, context);
	      }))
	      .WillOnce(::testing::Return(toReturn));
	it = tokens.begin();

	rval = p1.e14(it, context);

	EXPECT_EQ(it, tokens.end());
	assignment = dynamic_cast<Assignment *>(rval);
	EXPECT_NE(assignment, nullptr);
	if (assignment != nullptr) {
		EXPECT_EQ(assignment->variable->s, "x");
		EXPECT_EQ(assignment->expression, toReturn);
	}

	// Test 2: No Assignment with leading identifier
	MockParser p2;
	tokens = {};
	tokens.push_back(new Identifier(Slice("x", 1, "", 0, 0)));
	tokens.push_back(new Punctuation(Slice("+", 1, "", 0, 0), Punctuation::Type::Plus));
	toReturn = new EmptyRvalue;
	EXPECT_CALL(p2, e14).WillOnce(::testing::Invoke(
	      [&p2](std::vector<Token *>::iterator &it, AST::CodeBlock *context) -> rvalue * {
		      return p2.Parser::e14(it, context);
	      }));
	EXPECT_CALL(p2, e13).WillOnce(::testing::Return(toReturn));
	it = tokens.begin();

	rval = p2.e14(it, context);

	EXPECT_EQ(it, tokens.begin());
	EXPECT_EQ(rval, toReturn);

	// Test 3: No Assignment without leading identifier
	MockParser p3;
	tokens = {};
	tokens.push_back(new Punctuation(Slice("+", 1, "", 0, 0), Punctuation::Type::Plus));
	toReturn = new EmptyRvalue;
	EXPECT_CALL(p3, e14).WillOnce(::testing::Invoke(
	      [&p3](std::vector<Token *>::iterator &it, AST::CodeBlock *context) -> rvalue * {
		      return p3.Parser::e14(it, context);
	      }));
	EXPECT_CALL(p3, e13).WillOnce(::testing::Return(toReturn));
	it = tokens.begin();

	rval = p3.e14(it, context);

	EXPECT_EQ(it, tokens.begin());
	EXPECT_EQ(rval, toReturn);

	// Test 4: Assignment to undeclared variable
	Parser p4;
	tokens = {};
	context = new AST::CodeBlock(new AST::AST);
	tokens.push_back(new Identifier(Slice("x", 1, "", 0, 0)));
	tokens.push_back(new Punctuation(Slice("=", 1, "", 0, 0), Punctuation::Type::Equals));
	it = tokens.begin();

	EXPECT_EXIT(p4.e14(it, context), ::testing::ExitedWithCode(EXIT_FAILURE),
	      "Undeclared variable x");
}

TEST(test_parser, test_e15) {
	std::vector<Token *> tokens;
	std::vector<Token *>::iterator it;
	rvalue *rval;
	AST::CodeBlock *context = new AST::CodeBlock(new AST::AST);

	class MockParser : public Parser {
	public:
		MOCK_METHOD(AST::rvalue *, e14,
		      (std::vector<Token *>::iterator & it, AST::CodeBlock *context));
	};

	MockParser p;
	rvalue *toReturn = new EmptyRvalue;
	EXPECT_CALL(p, e14).WillOnce(::testing::Return(toReturn));
	it = tokens.begin();
	rval = p.e15(it, context);
	EXPECT_EQ(it, tokens.begin());
	EXPECT_EQ(rval, toReturn);
}

TEST(test_parser, test_parseRvalue) {
	std::vector<Token *> tokens;
	std::vector<Token *>::iterator it;
	rvalue *rval;
	AST::CodeBlock *context = new AST::CodeBlock(new AST::AST);

	class MockParser : public Parser {
	public:
		MOCK_METHOD(AST::rvalue *, e15,
		      (std::vector<Token *>::iterator & it, AST::CodeBlock *context));
	};

	MockParser p;
	rvalue *toReturn = new EmptyRvalue;
	EXPECT_CALL(p, e15).WillOnce(::testing::Return(toReturn));
	it = tokens.begin();
	rval = p.parseRvalue(it, context);
	EXPECT_EQ(it, tokens.begin());
	EXPECT_EQ(rval, toReturn);
}
