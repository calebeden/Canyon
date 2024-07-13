#ifndef DEBUG_TEST_MODE
#	error "DEBUG_TEST_MODE not defined"
#endif

#include "ast.h"
#include "parser.h"
#include "test_utilities.h"
#include "tokens.h"

#include "gtest/gtest.h"

#include <array>
#include <filesystem>
#include <initializer_list>
#include <memory>
#include <queue>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

using namespace ::testing;

class TestParser : public testing::Test {
public:
	TestParser() : s(Slice("", "", 0, 0)), dummy(s) {
	}
protected:
	NoErrorHandler e;
	Slice s;
	Whitespace dummy;
};

class TestParserError : public testing::Test {
public:
	TestParserError() : s(Slice("", "", 0, 0)), dummy(s) {
	}
protected:
	HasErrorHandler e;
	Slice s;
	Whitespace dummy;
};

class TestParserUnaryOperators : public TestParser,
                                 public testing::WithParamInterface<Operator::Type> { };

class TestParserBinaryOperators : public TestParser,
                                  public testing::WithParamInterface<Operator::Type> { };

class TestParserAssociativityLeftBinary
    : public TestParser,
      public testing::WithParamInterface<std::array<Operator::Type, 6>> { };

TEST_F(TestParser, testEmpty) {
	std::vector<std::unique_ptr<Token>> tokens;
	tokens.push_back(std::make_unique<EndOfFile>(s));
	Parser p = Parser("", std::move(tokens), &e);
	std::unique_ptr<Module> mod = p.parse();
	mod->forEachFunction([]([[maybe_unused]] std::string_view s, [[maybe_unused]]
	                                                             Function &f) {
		ADD_FAILURE() << "Expected no functions";
	});
}

TEST_F(TestParser, testEmptyFunction) {
	std::vector<std::unique_ptr<Token>> tokens;
	tokens.push_back(std::make_unique<Keyword>(s, Keyword::Type::FUN));
	tokens.push_back(std::make_unique<Symbol>(Slice("foo", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::OpenParen));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::CloseParen));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::Colon));
	tokens.push_back(std::make_unique<Symbol>(Slice("i32", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::OpenBrace));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::CloseBrace));
	tokens.push_back(std::make_unique<EndOfFile>(s));
	Parser p = Parser("", std::move(tokens), &e);
	std::unique_ptr<Module> mod = p.parse();
	int functionCount = 0;
	mod->forEachFunction([&functionCount](std::string_view name, Function &f) {
		functionCount++;
		EXPECT_EQ(name, "foo");
		EXPECT_EQ(f.getReturnTypeAnnotation()->s.contents, "i32");
		BlockExpression &body = f.getBody();
		int statementCount = 0;
		body.forEachStatement([&statementCount]([[maybe_unused]]
		                            Statement &s) {
			statementCount++;
			;
		});
		EXPECT_EQ(statementCount, 0);
		EXPECT_EQ(body.getFinalExpression(), nullptr);
	});
	EXPECT_EQ(functionCount, 1);
}

TEST_F(TestParser, testFunctionWithExpression) {
	std::vector<std::unique_ptr<Token>> tokens;
	tokens.push_back(std::make_unique<Keyword>(s, Keyword::Type::FUN));
	tokens.push_back(std::make_unique<Symbol>(Slice("foo", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::OpenParen));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::CloseParen));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::Colon));
	tokens.push_back(std::make_unique<Symbol>(Slice("i32", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::OpenBrace));
	tokens.push_back(std::make_unique<Symbol>(Slice("x", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::CloseBrace));
	tokens.push_back(std::make_unique<EndOfFile>(s));
	Parser p = Parser("", std::move(tokens), &e);
	std::unique_ptr<Module> mod = p.parse();
	int functionCount = 0;
	mod->forEachFunction([&functionCount](std::string_view name, Function &f) {
		functionCount++;
		EXPECT_EQ(name, "foo");
		EXPECT_EQ(f.getReturnTypeAnnotation()->s.contents, "i32");
		BlockExpression &body = f.getBody();
		int statementCount = 0;
		body.forEachStatement([&statementCount]([[maybe_unused]]
		                            Statement &s) {
			statementCount++;
		});
		EXPECT_EQ(statementCount, 0);
		Expression *expr = body.getFinalExpression();
		EXPECT_NE(expr, nullptr);
		auto *symbol = dynamic_cast<SymbolExpression *>(expr);
		EXPECT_NE(symbol, nullptr);
		EXPECT_EQ(symbol->getSymbol().s.contents, "x");
	});
	EXPECT_EQ(functionCount, 1);
}

TEST_F(TestParser, testFunctionWithStatement) {
	std::vector<std::unique_ptr<Token>> tokens;
	tokens.push_back(std::make_unique<Keyword>(s, Keyword::Type::FUN));
	tokens.push_back(std::make_unique<Symbol>(Slice("foo", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::OpenParen));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::CloseParen));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::Colon));
	tokens.push_back(std::make_unique<Symbol>(Slice("i32", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::OpenBrace));
	tokens.push_back(std::make_unique<Symbol>(Slice("x", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::Semicolon));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::CloseBrace));
	tokens.push_back(std::make_unique<EndOfFile>(s));
	Parser p = Parser("", std::move(tokens), &e);
	std::unique_ptr<Module> mod = p.parse();
	int functionCount = 0;
	mod->forEachFunction([&functionCount](std::string_view name, Function &f) {
		functionCount++;
		EXPECT_EQ(name, "foo");
		EXPECT_EQ(f.getReturnTypeAnnotation()->s.contents, "i32");
		BlockExpression &body = f.getBody();
		int statementCount = 0;
		body.forEachStatement([&statementCount](Statement &s) {
			statementCount++;
			auto &expr = dynamic_cast<ExpressionStatement &>(s);
			auto &symbol = dynamic_cast<SymbolExpression &>(expr.getExpression());
			EXPECT_EQ(symbol.getSymbol().s.contents, "x");
		});
		EXPECT_EQ(statementCount, 1);
		Expression *expr = body.getFinalExpression();
		EXPECT_EQ(expr, nullptr);
	});
	EXPECT_EQ(functionCount, 1);
}

TEST_F(TestParser, testFunctionWithStatementAndExpression) {
	std::vector<std::unique_ptr<Token>> tokens;
	tokens.push_back(std::make_unique<Keyword>(s, Keyword::Type::FUN));
	tokens.push_back(std::make_unique<Symbol>(Slice("foo", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::OpenParen));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::CloseParen));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::Colon));
	tokens.push_back(std::make_unique<Symbol>(Slice("i32", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::OpenBrace));
	tokens.push_back(std::make_unique<Symbol>(Slice("x", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::Semicolon));
	tokens.push_back(std::make_unique<Symbol>(Slice("y", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::CloseBrace));
	tokens.push_back(std::make_unique<EndOfFile>(s));
	Parser p = Parser("", std::move(tokens), &e);
	std::unique_ptr<Module> mod = p.parse();
	int functionCount = 0;
	mod->forEachFunction([&functionCount](std::string_view name, Function &f) {
		functionCount++;
		EXPECT_EQ(name, "foo");
		EXPECT_EQ(f.getReturnTypeAnnotation()->s.contents, "i32");
		BlockExpression &body = f.getBody();
		int statementCount = 0;
		body.forEachStatement([&statementCount](Statement &s) {
			statementCount++;
			auto &expr = dynamic_cast<ExpressionStatement &>(s);
			auto &symbol = dynamic_cast<SymbolExpression &>(expr.getExpression());
			EXPECT_EQ(symbol.getSymbol().s.contents, "x");
		});
		EXPECT_EQ(statementCount, 1);
		Expression *expr = body.getFinalExpression();
		EXPECT_NE(expr, nullptr);
		auto *symbol = dynamic_cast<SymbolExpression *>(expr);
		EXPECT_NE(symbol, nullptr);
		EXPECT_EQ(symbol->getSymbol().s.contents, "y");
	});
	EXPECT_EQ(functionCount, 1);
}

TEST_F(TestParser, testFunctionWithMultipleStatements) {
	std::vector<std::unique_ptr<Token>> tokens;
	tokens.push_back(std::make_unique<Keyword>(s, Keyword::Type::FUN));
	tokens.push_back(std::make_unique<Symbol>(Slice("foo", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::OpenParen));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::CloseParen));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::Colon));
	tokens.push_back(std::make_unique<Symbol>(Slice("i32", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::OpenBrace));
	tokens.push_back(std::make_unique<Symbol>(Slice("x", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::Semicolon));
	tokens.push_back(std::make_unique<Symbol>(Slice("y", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::Semicolon));
	tokens.push_back(std::make_unique<Symbol>(Slice("z", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::Semicolon));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::CloseBrace));
	tokens.push_back(std::make_unique<EndOfFile>(s));
	Parser p = Parser("", std::move(tokens), &e);
	std::unique_ptr<Module> mod = p.parse();
	int functionCount = 0;
	mod->forEachFunction([&functionCount](std::string_view name, Function &f) {
		functionCount++;
		EXPECT_EQ(name, "foo");
		EXPECT_EQ(f.getReturnTypeAnnotation()->s.contents, "i32");
		BlockExpression &body = f.getBody();
		int statementCount = 0;
		body.forEachStatement([&statementCount](Statement &s) {
			statementCount++;
			auto &expr = dynamic_cast<ExpressionStatement &>(s);
			auto &symbol = dynamic_cast<SymbolExpression &>(expr.getExpression());
			EXPECT_EQ(symbol.getSymbol().s.contents,
			      std::string(1, 'x' + statementCount - 1));
		});
		EXPECT_EQ(statementCount, 3);
		Expression *expr = body.getFinalExpression();
		EXPECT_EQ(expr, nullptr);
	});
	EXPECT_EQ(functionCount, 1);
}

TEST_F(TestParser, testFunctionWithMultipleStatementsAndExpression) {
	std::vector<std::unique_ptr<Token>> tokens;
	tokens.push_back(std::make_unique<Keyword>(s, Keyword::Type::FUN));
	tokens.push_back(std::make_unique<Symbol>(Slice("foo", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::OpenParen));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::CloseParen));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::Colon));
	tokens.push_back(std::make_unique<Symbol>(Slice("i32", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::OpenBrace));
	tokens.push_back(std::make_unique<Symbol>(Slice("x", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::Semicolon));
	tokens.push_back(std::make_unique<Symbol>(Slice("y", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::Semicolon));
	tokens.push_back(std::make_unique<Symbol>(Slice("z", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::Semicolon));
	tokens.push_back(std::make_unique<Symbol>(Slice("w", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::CloseBrace));
	tokens.push_back(std::make_unique<EndOfFile>(s));
	Parser p = Parser("", std::move(tokens), &e);
	std::unique_ptr<Module> mod = p.parse();
	int functionCount = 0;
	mod->forEachFunction([&functionCount](std::string_view name, Function &f) {
		functionCount++;
		EXPECT_EQ(name, "foo");
		EXPECT_EQ(f.getReturnTypeAnnotation()->s.contents, "i32");
		BlockExpression &body = f.getBody();
		int statementCount = 0;
		body.forEachStatement([&statementCount](Statement &s) {
			statementCount++;
			auto &expr = dynamic_cast<ExpressionStatement &>(s);
			auto &symbol = dynamic_cast<SymbolExpression &>(expr.getExpression());
			EXPECT_EQ(symbol.getSymbol().s.contents,
			      std::string(1, 'x' + statementCount - 1));
		});
		EXPECT_EQ(statementCount, 3);
		Expression *expr = body.getFinalExpression();
		EXPECT_NE(expr, nullptr);
		auto *symbol = dynamic_cast<SymbolExpression *>(expr);
		EXPECT_NE(symbol, nullptr);
		EXPECT_EQ(symbol->getSymbol().s.contents, "w");
	});
	EXPECT_EQ(functionCount, 1);
}

TEST_F(TestParser, testMultipleFunctions) {
	std::vector<std::unique_ptr<Token>> tokens;
	tokens.push_back(std::make_unique<Keyword>(s, Keyword::Type::FUN));
	tokens.push_back(std::make_unique<Symbol>(Slice("foo", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::OpenParen));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::CloseParen));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::Colon));
	tokens.push_back(std::make_unique<Symbol>(Slice("i32", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::OpenBrace));
	tokens.push_back(std::make_unique<Symbol>(Slice("x", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::CloseBrace));
	tokens.push_back(std::make_unique<Keyword>(s, Keyword::Type::FUN));
	tokens.push_back(std::make_unique<Symbol>(Slice("bar", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::OpenParen));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::CloseParen));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::Colon));
	tokens.push_back(std::make_unique<Symbol>(Slice("i32", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::OpenBrace));
	tokens.push_back(std::make_unique<Symbol>(Slice("y", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::CloseBrace));
	tokens.push_back(std::make_unique<EndOfFile>(s));
	Parser p = Parser("", std::move(tokens), &e);
	std::unique_ptr<Module> mod = p.parse();
	int functionCount = 0;
	bool seenFoo = false;
	bool seenBar = false;
	mod->forEachFunction(
	      [&functionCount, &seenFoo, &seenBar](std::string_view name, Function &f) {
		      functionCount++;
		      if (name == "foo") {
			      seenFoo = true;
			      EXPECT_EQ(f.getReturnTypeAnnotation()->s.contents, "i32");
			      BlockExpression &body = f.getBody();
			      int statementCount = 0;
			      body.forEachStatement([&statementCount]([[maybe_unused]]
			                                  Statement &s) {
				      statementCount++;
			      });
			      EXPECT_EQ(statementCount, 0);
			      Expression *expr = body.getFinalExpression();
			      EXPECT_NE(expr, nullptr);
			      auto *symbol = dynamic_cast<SymbolExpression *>(expr);
			      EXPECT_NE(symbol, nullptr);
			      EXPECT_EQ(symbol->getSymbol().s.contents, "x");
		      } else if (name == "bar") {
			      seenBar = true;
			      EXPECT_EQ(f.getReturnTypeAnnotation()->s.contents, "i32");
			      BlockExpression &body = f.getBody();
			      int statementCount = 0;
			      body.forEachStatement([&statementCount]([[maybe_unused]]
			                                  Statement &s) {
				      statementCount++;
			      });
			      EXPECT_EQ(statementCount, 0);
			      Expression *expr = body.getFinalExpression();
			      EXPECT_NE(expr, nullptr);
			      auto *symbol = dynamic_cast<SymbolExpression *>(expr);
			      EXPECT_NE(symbol, nullptr);
			      EXPECT_EQ(symbol->getSymbol().s.contents, "y");
		      } else {
			      ADD_FAILURE() << "Unexpected function name: " << name;
		      }
	      });
	EXPECT_EQ(functionCount, 2);
	EXPECT_TRUE(seenFoo);
	EXPECT_TRUE(seenBar);
}

TEST_F(TestParser, testIntegerLiteral) {
	std::vector<std::unique_ptr<Token>> tokens;
	tokens.push_back(std::make_unique<Keyword>(s, Keyword::Type::FUN));
	tokens.push_back(std::make_unique<Symbol>(Slice("foo", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::OpenParen));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::CloseParen));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::Colon));
	tokens.push_back(std::make_unique<Symbol>(Slice("i32", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::OpenBrace));
	tokens.push_back(
	      std::make_unique<IntegerLiteral>(dummy, IntegerLiteral::Type::I32, 1));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::CloseBrace));
	tokens.push_back(std::make_unique<EndOfFile>(s));
	Parser p = Parser("", std::move(tokens), &e);
	std::unique_ptr<Module> mod = p.parse();
	mod->forEachFunction([]([[maybe_unused]]
	                           std::string_view name,
	                           Function &f) {
		BlockExpression &body = f.getBody();
		Expression *expr = body.getFinalExpression();
		EXPECT_NE(expr, nullptr);
		auto *literal = dynamic_cast<LiteralExpression *>(expr);
		EXPECT_NE(literal, nullptr);
		EXPECT_EQ(literal->getLiteral().type, IntegerLiteral::Type::I32);
		EXPECT_EQ(literal->getLiteral().value, 1);
	});
}

TEST_F(TestParser, testSymbol) {
	std::vector<std::unique_ptr<Token>> tokens;
	tokens.push_back(std::make_unique<Keyword>(s, Keyword::Type::FUN));
	tokens.push_back(std::make_unique<Symbol>(Slice("foo", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::OpenParen));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::CloseParen));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::Colon));
	tokens.push_back(std::make_unique<Symbol>(Slice("i32", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::OpenBrace));
	tokens.push_back(std::make_unique<Symbol>(Slice("x", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::CloseBrace));
	tokens.push_back(std::make_unique<EndOfFile>(s));
	Parser p = Parser("", std::move(tokens), &e);
	std::unique_ptr<Module> mod = p.parse();
	mod->forEachFunction([]([[maybe_unused]]
	                           std::string_view name,
	                           Function &f) {
		BlockExpression &body = f.getBody();
		Expression *expr = body.getFinalExpression();
		EXPECT_NE(expr, nullptr);
		auto *symbol = dynamic_cast<SymbolExpression *>(expr);
		EXPECT_NE(symbol, nullptr);
		EXPECT_EQ(symbol->getSymbol().s.contents, "x");
	});
}

TEST_F(TestParser, testParenthesizedExpression) {
	std::vector<std::unique_ptr<Token>> tokens;
	tokens.push_back(std::make_unique<Keyword>(s, Keyword::Type::FUN));
	tokens.push_back(std::make_unique<Symbol>(Slice("foo", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::OpenParen));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::CloseParen));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::Colon));
	tokens.push_back(std::make_unique<Symbol>(Slice("i32", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::OpenBrace));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::OpenParen));
	tokens.push_back(
	      std::make_unique<IntegerLiteral>(dummy, IntegerLiteral::Type::I32, 1));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::CloseParen));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::CloseBrace));
	tokens.push_back(std::make_unique<EndOfFile>(s));
	Parser p = Parser("", std::move(tokens), &e);
	std::unique_ptr<Module> mod = p.parse();
	mod->forEachFunction([]([[maybe_unused]]
	                           std::string_view name,
	                           Function &f) {
		BlockExpression &body = f.getBody();
		Expression *expr = body.getFinalExpression();
		EXPECT_NE(expr, nullptr);
		auto *paren = dynamic_cast<ParenthesizedExpression *>(expr);
		EXPECT_NE(paren, nullptr);
		auto &literal = dynamic_cast<LiteralExpression &>(paren->getExpression());
		EXPECT_EQ(literal.getLiteral().type, IntegerLiteral::Type::I32);
		EXPECT_EQ(literal.getLiteral().value, 1);
	});
}

TEST_F(TestParser, testFunctionCall) {
	std::vector<std::unique_ptr<Token>> tokens;
	tokens.push_back(std::make_unique<Keyword>(s, Keyword::Type::FUN));
	tokens.push_back(std::make_unique<Symbol>(Slice("foo", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::OpenParen));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::CloseParen));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::Colon));
	tokens.push_back(std::make_unique<Symbol>(Slice("i32", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::OpenBrace));
	tokens.push_back(std::make_unique<Symbol>(Slice("bar", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::OpenParen));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::CloseParen));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::CloseBrace));
	tokens.push_back(std::make_unique<EndOfFile>(s));
	Parser p = Parser("", std::move(tokens), &e);
	std::unique_ptr<Module> mod = p.parse();
	mod->forEachFunction([]([[maybe_unused]]
	                           std::string_view s,
	                           Function &f) {
		BlockExpression &body = f.getBody();
		Expression *expr = body.getFinalExpression();
		EXPECT_NE(expr, nullptr);
		auto *call = dynamic_cast<FunctionCallExpression *>(expr);
		EXPECT_NE(call, nullptr);
		auto &name = dynamic_cast<SymbolExpression &>(call->getFunction());
		EXPECT_EQ(name.getSymbol().s.contents, "bar");
	});
}

TEST_P(TestParserUnaryOperators, testUnary) {
	std::vector<std::unique_ptr<Token>> tokens;
	Operator::Type op = GetParam();
	tokens.push_back(std::make_unique<Keyword>(s, Keyword::Type::FUN));
	tokens.push_back(std::make_unique<Symbol>(Slice("foo", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::OpenParen));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::CloseParen));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::Colon));
	tokens.push_back(std::make_unique<Symbol>(Slice("i32", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::OpenBrace));
	tokens.push_back(std::make_unique<Operator>(dummy, op));
	tokens.push_back(
	      std::make_unique<IntegerLiteral>(dummy, IntegerLiteral::Type::I32, 1));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::CloseBrace));
	tokens.push_back(std::make_unique<EndOfFile>(s));
	Parser p = Parser("", std::move(tokens), &e);
	std::unique_ptr<Module> mod = p.parse();
	mod->forEachFunction([op]([[maybe_unused]]
	                           std::string_view name,
	                           Function &f) {
		BlockExpression &body = f.getBody();
		Expression *expr = body.getFinalExpression();
		EXPECT_NE(expr, nullptr);
		auto *unary = dynamic_cast<UnaryExpression *>(expr);
		EXPECT_NE(unary, nullptr);
		EXPECT_EQ(unary->getOperator().type, op);
		auto &literal = dynamic_cast<LiteralExpression &>(unary->getExpression());
		EXPECT_EQ(literal.getLiteral().type, IntegerLiteral::Type::I32);
		EXPECT_EQ(literal.getLiteral().value, 1);
	});
}

INSTANTIATE_TEST_SUITE_P(testUnary, TestParserUnaryOperators,
      testing::Values(Operator::Type::LogicalNot, Operator::Type::BitwiseNot,
            Operator::Type::Subtraction, Operator::Type::Addition));

TEST_P(TestParserBinaryOperators, testBinary) {
	std::vector<std::unique_ptr<Token>> tokens;
	Operator::Type op = GetParam();
	tokens.push_back(std::make_unique<Keyword>(s, Keyword::Type::FUN));
	tokens.push_back(std::make_unique<Symbol>(Slice("foo", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::OpenParen));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::CloseParen));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::Colon));
	tokens.push_back(std::make_unique<Symbol>(Slice("i32", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::OpenBrace));
	tokens.push_back(
	      std::make_unique<IntegerLiteral>(dummy, IntegerLiteral::Type::I32, 1));
	tokens.push_back(std::make_unique<Operator>(dummy, op));
	tokens.push_back(
	      std::make_unique<IntegerLiteral>(dummy, IntegerLiteral::Type::I32, 2));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::CloseBrace));
	tokens.push_back(std::make_unique<EndOfFile>(s));
	Parser p = Parser("", std::move(tokens), &e);
	std::unique_ptr<Module> mod = p.parse();
	mod->forEachFunction([op]([[maybe_unused]]
	                           std::string_view name,
	                           Function &f) {
		BlockExpression &body = f.getBody();
		Expression *expr = body.getFinalExpression();
		EXPECT_NE(expr, nullptr);
		auto *binary = dynamic_cast<BinaryExpression *>(expr);
		EXPECT_NE(binary, nullptr);
		EXPECT_EQ(binary->getOperator().type, op);
		auto &left = dynamic_cast<LiteralExpression &>(binary->getLeft());
		EXPECT_EQ(left.getLiteral().type, IntegerLiteral::Type::I32);
		EXPECT_EQ(left.getLiteral().value, 1);
		auto &right = dynamic_cast<LiteralExpression &>(binary->getRight());
		EXPECT_EQ(right.getLiteral().type, IntegerLiteral::Type::I32);
		EXPECT_EQ(right.getLiteral().value, 2);
	});
}

INSTANTIATE_TEST_SUITE_P(testBinary, TestParserBinaryOperators,
      testing::Values(Operator::Type::Multiplication, Operator::Type::Division,
            Operator::Type::Modulus, Operator::Type::Addition,
            Operator::Type::Subtraction, Operator::Type::BitwiseShiftLeft,
            Operator::Type::BitwiseShiftRight, Operator::Type::BitwiseAnd,
            Operator::Type::BitwiseXor, Operator::Type::BitwiseOr,
            Operator::Type::Equality, Operator::Type::Inequality,
            Operator::Type::LessThan, Operator::Type::LessThanOrEqual,
            Operator::Type::GreaterThan, Operator::Type::GreaterThanOrEqual,
            Operator::Type::LogicalAnd, Operator::Type::LogicalOr,
            Operator::Type::Assignment));

TEST_F(TestParser, testReturnExpression) {
	std::vector<std::unique_ptr<Token>> tokens;
	tokens.push_back(std::make_unique<Keyword>(s, Keyword::Type::FUN));
	tokens.push_back(std::make_unique<Symbol>(Slice("foo", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::OpenParen));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::CloseParen));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::Colon));
	tokens.push_back(std::make_unique<Symbol>(Slice("i32", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::OpenBrace));
	tokens.push_back(std::make_unique<Keyword>(s, Keyword::Type::RETURN));
	tokens.push_back(
	      std::make_unique<IntegerLiteral>(dummy, IntegerLiteral::Type::I32, 1));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::CloseBrace));
	tokens.push_back(std::make_unique<EndOfFile>(s));
	Parser p = Parser("", std::move(tokens), &e);
	std::unique_ptr<Module> mod = p.parse();
	mod->forEachFunction([]([[maybe_unused]]
	                           std::string_view name,
	                           Function &f) {
		BlockExpression &body = f.getBody();
		Expression *expr = body.getFinalExpression();
		EXPECT_NE(expr, nullptr);
		auto *ret = dynamic_cast<ReturnExpression *>(expr);
		EXPECT_NE(ret, nullptr);
		auto *literal = dynamic_cast<LiteralExpression *>(ret->getExpression());
		EXPECT_NE(literal, nullptr);
		EXPECT_EQ(literal->getLiteral().type, IntegerLiteral::Type::I32);
		EXPECT_EQ(literal->getLiteral().value, 1);
	});
}

TEST_F(TestParser, testReturnUnit) {
	std::vector<std::unique_ptr<Token>> tokens;
	tokens.push_back(std::make_unique<Keyword>(s, Keyword::Type::FUN));
	tokens.push_back(std::make_unique<Symbol>(Slice("foo", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::OpenParen));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::CloseParen));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::Colon));
	tokens.push_back(std::make_unique<Symbol>(Slice("i32", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::OpenBrace));
	tokens.push_back(std::make_unique<Keyword>(s, Keyword::Type::RETURN));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::CloseBrace));
	tokens.push_back(std::make_unique<EndOfFile>(s));
	Parser p = Parser("", std::move(tokens), &e);
	std::unique_ptr<Module> mod = p.parse();
	mod->forEachFunction([]([[maybe_unused]]
	                           std::string_view name,
	                           Function &f) {
		BlockExpression &body = f.getBody();
		Expression *expr = body.getFinalExpression();
		EXPECT_NE(expr, nullptr);
		auto *ret = dynamic_cast<ReturnExpression *>(expr);
		EXPECT_NE(ret, nullptr);
		EXPECT_EQ(ret->getExpression(), nullptr);
	});
}

TEST_F(TestParser, testBlockExpression) {
	std::vector<std::unique_ptr<Token>> tokens;
	tokens.push_back(std::make_unique<Keyword>(s, Keyword::Type::FUN));
	tokens.push_back(std::make_unique<Symbol>(Slice("foo", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::OpenParen));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::CloseParen));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::Colon));
	tokens.push_back(std::make_unique<Symbol>(Slice("i32", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::OpenBrace));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::OpenBrace));
	tokens.push_back(
	      std::make_unique<IntegerLiteral>(dummy, IntegerLiteral::Type::I32, 1));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::CloseBrace));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::CloseBrace));
	tokens.push_back(std::make_unique<EndOfFile>(s));
	Parser p = Parser("", std::move(tokens), &e);
	std::unique_ptr<Module> mod = p.parse();
	mod->forEachFunction([]([[maybe_unused]]
	                           std::string_view name,
	                           Function &f) {
		BlockExpression &body = f.getBody();
		Expression *expr = body.getFinalExpression();
		EXPECT_NE(expr, nullptr);
		auto *block = dynamic_cast<BlockExpression *>(expr);
		EXPECT_NE(block, nullptr);
		Expression *inner = block->getFinalExpression();
		EXPECT_NE(inner, nullptr);
		auto *literal = dynamic_cast<LiteralExpression *>(inner);
		EXPECT_NE(literal, nullptr);
		EXPECT_EQ(literal->getLiteral().type, IntegerLiteral::Type::I32);
		EXPECT_EQ(literal->getLiteral().value, 1);
	});
}

TEST_F(TestParser, testPrecedenceUnaryFunctionCall) {
	std::vector<std::unique_ptr<Token>> tokens;
	tokens.push_back(std::make_unique<Keyword>(s, Keyword::Type::FUN));
	tokens.push_back(std::make_unique<Symbol>(Slice("foo", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::OpenParen));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::CloseParen));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::Colon));
	tokens.push_back(std::make_unique<Symbol>(Slice("i32", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::OpenBrace));
	tokens.push_back(std::make_unique<Operator>(dummy, Operator::Type::LogicalNot));
	tokens.push_back(std::make_unique<Symbol>(Slice("x", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::OpenParen));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::CloseParen));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::CloseBrace));
	tokens.push_back(std::make_unique<EndOfFile>(s));
	Parser p = Parser("", std::move(tokens), &e);
	std::unique_ptr<Module> mod = p.parse();
	mod->forEachFunction([]([[maybe_unused]]
	                           std::string_view s,
	                           Function &f) {
		BlockExpression &body = f.getBody();
		Expression *expr = body.getFinalExpression();
		EXPECT_NE(expr, nullptr);
		auto *unary = dynamic_cast<UnaryExpression *>(expr);
		EXPECT_NE(unary, nullptr);
		EXPECT_EQ(unary->getOperator().type, Operator::Type::LogicalNot);
		auto &call = dynamic_cast<FunctionCallExpression &>(unary->getExpression());
		auto &name = dynamic_cast<SymbolExpression &>(call.getFunction());
		EXPECT_EQ(name.getSymbol().s.contents, "x");
	});
}

TEST_F(TestParser, testPrecedenceFunctionCallMultiplicative) {
	std::vector<std::unique_ptr<Token>> tokens;
	tokens.push_back(std::make_unique<Keyword>(s, Keyword::Type::FUN));
	tokens.push_back(std::make_unique<Symbol>(Slice("foo", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::OpenParen));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::CloseParen));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::Colon));
	tokens.push_back(std::make_unique<Symbol>(Slice("i32", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::OpenBrace));
	tokens.push_back(std::make_unique<Symbol>(Slice("x", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::OpenParen));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::CloseParen));
	tokens.push_back(std::make_unique<Operator>(dummy, Operator::Type::Multiplication));
	tokens.push_back(std::make_unique<Symbol>(Slice("y", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::OpenParen));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::CloseParen));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::CloseBrace));
	tokens.push_back(std::make_unique<EndOfFile>(s));
	Parser p = Parser("", std::move(tokens), &e);
	std::unique_ptr<Module> mod = p.parse();
	mod->forEachFunction([]([[maybe_unused]]
	                           std::string_view name,
	                           Function &f) {
		BlockExpression &body = f.getBody();
		Expression *expr = body.getFinalExpression();
		EXPECT_NE(expr, nullptr);
		auto *binary = dynamic_cast<BinaryExpression *>(expr);
		EXPECT_NE(binary, nullptr);
		EXPECT_EQ(binary->getOperator().type, Operator::Type::Multiplication);
		auto &left = dynamic_cast<FunctionCallExpression &>(binary->getLeft());
		auto &leftName = dynamic_cast<SymbolExpression &>(left.getFunction());
		EXPECT_EQ(leftName.getSymbol().s.contents, "x");
		auto &right = dynamic_cast<FunctionCallExpression &>(binary->getRight());
		auto &rightName = dynamic_cast<SymbolExpression &>(right.getFunction());
		EXPECT_EQ(rightName.getSymbol().s.contents, "y");
	});
}

TEST_F(TestParser, testPrecedenceMultiplicativeAdditive) {
	std::vector<std::unique_ptr<Token>> tokens;
	tokens.push_back(std::make_unique<Keyword>(s, Keyword::Type::FUN));
	tokens.push_back(std::make_unique<Symbol>(Slice("foo", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::OpenParen));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::CloseParen));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::Colon));
	tokens.push_back(std::make_unique<Symbol>(Slice("i32", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::OpenBrace));
	tokens.push_back(std::make_unique<Symbol>(Slice("a", "", 0, 0)));
	tokens.push_back(std::make_unique<Operator>(dummy, Operator::Type::Multiplication));
	tokens.push_back(std::make_unique<Symbol>(Slice("b", "", 0, 0)));
	tokens.push_back(std::make_unique<Operator>(dummy, Operator::Type::Addition));
	tokens.push_back(std::make_unique<Symbol>(Slice("c", "", 0, 0)));
	tokens.push_back(std::make_unique<Operator>(dummy, Operator::Type::Division));
	tokens.push_back(std::make_unique<Symbol>(Slice("d", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::CloseBrace));
	tokens.push_back(std::make_unique<EndOfFile>(s));
	Parser p = Parser("", std::move(tokens), &e);
	std::unique_ptr<Module> mod = p.parse();
	mod->forEachFunction([]([[maybe_unused]]
	                           std::string_view name,
	                           Function &f) {
		BlockExpression &body = f.getBody();
		Expression *expr = body.getFinalExpression();
		EXPECT_NE(expr, nullptr);
		auto *outer = dynamic_cast<BinaryExpression *>(expr);
		EXPECT_NE(outer, nullptr);
		EXPECT_EQ(outer->getOperator().type, Operator::Type::Addition);
		auto &left = dynamic_cast<BinaryExpression &>(outer->getLeft());
		EXPECT_EQ(left.getOperator().type, Operator::Type::Multiplication);
		auto &leftLeft = dynamic_cast<SymbolExpression &>(left.getLeft());
		EXPECT_EQ(leftLeft.getSymbol().s.contents, "a");
		auto &leftRight = dynamic_cast<SymbolExpression &>(left.getRight());
		EXPECT_EQ(leftRight.getSymbol().s.contents, "b");
		auto &right = dynamic_cast<BinaryExpression &>(outer->getRight());
		EXPECT_EQ(right.getOperator().type, Operator::Type::Division);
		auto &rightLeft = dynamic_cast<SymbolExpression &>(right.getLeft());
		EXPECT_EQ(rightLeft.getSymbol().s.contents, "c");
		auto &rightRight = dynamic_cast<SymbolExpression &>(right.getRight());
		EXPECT_EQ(rightRight.getSymbol().s.contents, "d");
	});
}

TEST_F(TestParser, testPrecedenceAdditiveBitshift) {
	std::vector<std::unique_ptr<Token>> tokens;
	tokens.push_back(std::make_unique<Keyword>(s, Keyword::Type::FUN));
	tokens.push_back(std::make_unique<Symbol>(Slice("foo", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::OpenParen));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::CloseParen));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::Colon));
	tokens.push_back(std::make_unique<Symbol>(Slice("i32", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::OpenBrace));
	tokens.push_back(std::make_unique<Symbol>(Slice("a", "", 0, 0)));
	tokens.push_back(std::make_unique<Operator>(dummy, Operator::Type::Addition));
	tokens.push_back(std::make_unique<Symbol>(Slice("b", "", 0, 0)));
	tokens.push_back(std::make_unique<Operator>(dummy, Operator::Type::BitwiseShiftLeft));
	tokens.push_back(std::make_unique<Symbol>(Slice("c", "", 0, 0)));
	tokens.push_back(std::make_unique<Operator>(dummy, Operator::Type::Subtraction));
	tokens.push_back(std::make_unique<Symbol>(Slice("d", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::CloseBrace));
	tokens.push_back(std::make_unique<EndOfFile>(s));
	Parser p = Parser("", std::move(tokens), &e);
	std::unique_ptr<Module> mod = p.parse();
	mod->forEachFunction([]([[maybe_unused]]
	                           std::string_view name,
	                           Function &f) {
		BlockExpression &body = f.getBody();
		Expression *expr = body.getFinalExpression();
		EXPECT_NE(expr, nullptr);
		auto *outer = dynamic_cast<BinaryExpression *>(expr);
		EXPECT_NE(outer, nullptr);
		EXPECT_EQ(outer->getOperator().type, Operator::Type::BitwiseShiftLeft);
		auto &left = dynamic_cast<BinaryExpression &>(outer->getLeft());
		EXPECT_EQ(left.getOperator().type, Operator::Type::Addition);
		auto &leftLeft = dynamic_cast<SymbolExpression &>(left.getLeft());
		EXPECT_EQ(leftLeft.getSymbol().s.contents, "a");
		auto &leftRight = dynamic_cast<SymbolExpression &>(left.getRight());
		EXPECT_EQ(leftRight.getSymbol().s.contents, "b");
		auto &right = dynamic_cast<BinaryExpression &>(outer->getRight());
		EXPECT_EQ(right.getOperator().type, Operator::Type::Subtraction);
		auto &rightLeft = dynamic_cast<SymbolExpression &>(right.getLeft());
		EXPECT_EQ(rightLeft.getSymbol().s.contents, "c");
		auto &rightRight = dynamic_cast<SymbolExpression &>(right.getRight());
		EXPECT_EQ(rightRight.getSymbol().s.contents, "d");
	});
}

TEST_F(TestParser, testPrecedenceBitshiftBitwiseAnd) {
	std::vector<std::unique_ptr<Token>> tokens;
	tokens.push_back(std::make_unique<Keyword>(s, Keyword::Type::FUN));
	tokens.push_back(std::make_unique<Symbol>(Slice("foo", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::OpenParen));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::CloseParen));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::Colon));
	tokens.push_back(std::make_unique<Symbol>(Slice("i32", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::OpenBrace));
	tokens.push_back(std::make_unique<Symbol>(Slice("a", "", 0, 0)));
	tokens.push_back(std::make_unique<Operator>(dummy, Operator::Type::BitwiseShiftLeft));
	tokens.push_back(std::make_unique<Symbol>(Slice("b", "", 0, 0)));
	tokens.push_back(std::make_unique<Operator>(dummy, Operator::Type::BitwiseAnd));
	tokens.push_back(std::make_unique<Symbol>(Slice("c", "", 0, 0)));
	tokens.push_back(
	      std::make_unique<Operator>(dummy, Operator::Type::BitwiseShiftRight));
	tokens.push_back(std::make_unique<Symbol>(Slice("d", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::CloseBrace));
	tokens.push_back(std::make_unique<EndOfFile>(s));
	Parser p = Parser("", std::move(tokens), &e);
	std::unique_ptr<Module> mod = p.parse();
	mod->forEachFunction([]([[maybe_unused]]
	                           std::string_view name,
	                           Function &f) {
		BlockExpression &body = f.getBody();
		Expression *expr = body.getFinalExpression();
		EXPECT_NE(expr, nullptr);
		auto *outer = dynamic_cast<BinaryExpression *>(expr);
		EXPECT_NE(outer, nullptr);
		EXPECT_EQ(outer->getOperator().type, Operator::Type::BitwiseAnd);
		auto &left = dynamic_cast<BinaryExpression &>(outer->getLeft());
		EXPECT_EQ(left.getOperator().type, Operator::Type::BitwiseShiftLeft);
		auto &leftLeft = dynamic_cast<SymbolExpression &>(left.getLeft());
		EXPECT_EQ(leftLeft.getSymbol().s.contents, "a");
		auto &leftRight = dynamic_cast<SymbolExpression &>(left.getRight());
		EXPECT_EQ(leftRight.getSymbol().s.contents, "b");
		auto &right = dynamic_cast<BinaryExpression &>(outer->getRight());
		EXPECT_EQ(right.getOperator().type, Operator::Type::BitwiseShiftRight);
		auto &rightLeft = dynamic_cast<SymbolExpression &>(right.getLeft());
		EXPECT_EQ(rightLeft.getSymbol().s.contents, "c");
		auto &rightRight = dynamic_cast<SymbolExpression &>(right.getRight());
		EXPECT_EQ(rightRight.getSymbol().s.contents, "d");
	});
}

TEST_F(TestParser, testPrecedenceBitwiseAndBitwiseXor) {
	std::vector<std::unique_ptr<Token>> tokens;
	tokens.push_back(std::make_unique<Keyword>(s, Keyword::Type::FUN));
	tokens.push_back(std::make_unique<Symbol>(Slice("foo", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::OpenParen));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::CloseParen));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::Colon));
	tokens.push_back(std::make_unique<Symbol>(Slice("i32", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::OpenBrace));
	tokens.push_back(std::make_unique<Symbol>(Slice("a", "", 0, 0)));
	tokens.push_back(std::make_unique<Operator>(dummy, Operator::Type::BitwiseAnd));
	tokens.push_back(std::make_unique<Symbol>(Slice("b", "", 0, 0)));
	tokens.push_back(std::make_unique<Operator>(dummy, Operator::Type::BitwiseXor));
	tokens.push_back(std::make_unique<Symbol>(Slice("c", "", 0, 0)));
	tokens.push_back(std::make_unique<Operator>(dummy, Operator::Type::BitwiseAnd));
	tokens.push_back(std::make_unique<Symbol>(Slice("d", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::CloseBrace));
	tokens.push_back(std::make_unique<EndOfFile>(s));
	Parser p = Parser("", std::move(tokens), &e);
	std::unique_ptr<Module> mod = p.parse();
	mod->forEachFunction([]([[maybe_unused]]
	                           std::string_view name,
	                           Function &f) {
		BlockExpression &body = f.getBody();
		Expression *expr = body.getFinalExpression();
		EXPECT_NE(expr, nullptr);
		auto *outer = dynamic_cast<BinaryExpression *>(expr);
		EXPECT_NE(outer, nullptr);
		EXPECT_EQ(outer->getOperator().type, Operator::Type::BitwiseXor);
		auto &left = dynamic_cast<BinaryExpression &>(outer->getLeft());
		EXPECT_EQ(left.getOperator().type, Operator::Type::BitwiseAnd);
		auto &leftLeft = dynamic_cast<SymbolExpression &>(left.getLeft());
		EXPECT_EQ(leftLeft.getSymbol().s.contents, "a");
		auto &leftRight = dynamic_cast<SymbolExpression &>(left.getRight());
		EXPECT_EQ(leftRight.getSymbol().s.contents, "b");
		auto &right = dynamic_cast<BinaryExpression &>(outer->getRight());
		EXPECT_EQ(right.getOperator().type, Operator::Type::BitwiseAnd);
		auto &rightLeft = dynamic_cast<SymbolExpression &>(right.getLeft());
		EXPECT_EQ(rightLeft.getSymbol().s.contents, "c");
		auto &rightRight = dynamic_cast<SymbolExpression &>(right.getRight());
		EXPECT_EQ(rightRight.getSymbol().s.contents, "d");
	});
}

TEST_F(TestParser, testPrecedenceBitwiseXorBitwiseOr) {
	std::vector<std::unique_ptr<Token>> tokens;
	tokens.push_back(std::make_unique<Keyword>(s, Keyword::Type::FUN));
	tokens.push_back(std::make_unique<Symbol>(Slice("foo", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::OpenParen));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::CloseParen));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::Colon));
	tokens.push_back(std::make_unique<Symbol>(Slice("i32", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::OpenBrace));
	tokens.push_back(std::make_unique<Symbol>(Slice("a", "", 0, 0)));
	tokens.push_back(std::make_unique<Operator>(dummy, Operator::Type::BitwiseXor));
	tokens.push_back(std::make_unique<Symbol>(Slice("b", "", 0, 0)));
	tokens.push_back(std::make_unique<Operator>(dummy, Operator::Type::BitwiseOr));
	tokens.push_back(std::make_unique<Symbol>(Slice("c", "", 0, 0)));
	tokens.push_back(std::make_unique<Operator>(dummy, Operator::Type::BitwiseXor));
	tokens.push_back(std::make_unique<Symbol>(Slice("d", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::CloseBrace));
	tokens.push_back(std::make_unique<EndOfFile>(s));
	Parser p = Parser("", std::move(tokens), &e);
	std::unique_ptr<Module> mod = p.parse();
	mod->forEachFunction([]([[maybe_unused]]
	                           std::string_view name,
	                           Function &f) {
		BlockExpression &body = f.getBody();
		Expression *expr = body.getFinalExpression();
		EXPECT_NE(expr, nullptr);
		auto *outer = dynamic_cast<BinaryExpression *>(expr);
		EXPECT_NE(outer, nullptr);
		EXPECT_EQ(outer->getOperator().type, Operator::Type::BitwiseOr);
		auto &left = dynamic_cast<BinaryExpression &>(outer->getLeft());
		EXPECT_EQ(left.getOperator().type, Operator::Type::BitwiseXor);
		auto &leftLeft = dynamic_cast<SymbolExpression &>(left.getLeft());
		EXPECT_EQ(leftLeft.getSymbol().s.contents, "a");
		auto &leftRight = dynamic_cast<SymbolExpression &>(left.getRight());
		EXPECT_EQ(leftRight.getSymbol().s.contents, "b");
		auto &right = dynamic_cast<BinaryExpression &>(outer->getRight());
		EXPECT_EQ(right.getOperator().type, Operator::Type::BitwiseXor);
		auto &rightLeft = dynamic_cast<SymbolExpression &>(right.getLeft());
		EXPECT_EQ(rightLeft.getSymbol().s.contents, "c");
		auto &rightRight = dynamic_cast<SymbolExpression &>(right.getRight());
		EXPECT_EQ(rightRight.getSymbol().s.contents, "d");
	});
}

TEST_F(TestParser, testPrecedenceBitwiseOrRelational) {
	std::vector<std::unique_ptr<Token>> tokens;
	tokens.push_back(std::make_unique<Keyword>(s, Keyword::Type::FUN));
	tokens.push_back(std::make_unique<Symbol>(Slice("foo", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::OpenParen));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::CloseParen));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::Colon));
	tokens.push_back(std::make_unique<Symbol>(Slice("i32", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::OpenBrace));
	tokens.push_back(std::make_unique<Symbol>(Slice("a", "", 0, 0)));
	tokens.push_back(std::make_unique<Operator>(dummy, Operator::Type::BitwiseOr));
	tokens.push_back(std::make_unique<Symbol>(Slice("b", "", 0, 0)));
	tokens.push_back(std::make_unique<Operator>(dummy, Operator::Type::LessThan));
	tokens.push_back(std::make_unique<Symbol>(Slice("c", "", 0, 0)));
	tokens.push_back(std::make_unique<Operator>(dummy, Operator::Type::BitwiseOr));
	tokens.push_back(std::make_unique<Symbol>(Slice("d", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::CloseBrace));
	tokens.push_back(std::make_unique<EndOfFile>(s));
	Parser p = Parser("", std::move(tokens), &e);
	std::unique_ptr<Module> mod = p.parse();
	mod->forEachFunction([]([[maybe_unused]]
	                           std::string_view name,
	                           Function &f) {
		BlockExpression &body = f.getBody();
		Expression *expr = body.getFinalExpression();
		EXPECT_NE(expr, nullptr);
		auto *outer = dynamic_cast<BinaryExpression *>(expr);
		EXPECT_NE(outer, nullptr);
		EXPECT_EQ(outer->getOperator().type, Operator::Type::LessThan);
		auto &left = dynamic_cast<BinaryExpression &>(outer->getLeft());
		EXPECT_EQ(left.getOperator().type, Operator::Type::BitwiseOr);
		auto &leftLeft = dynamic_cast<SymbolExpression &>(left.getLeft());
		EXPECT_EQ(leftLeft.getSymbol().s.contents, "a");
		auto &leftRight = dynamic_cast<SymbolExpression &>(left.getRight());
		EXPECT_EQ(leftRight.getSymbol().s.contents, "b");
		auto &right = dynamic_cast<BinaryExpression &>(outer->getRight());
		EXPECT_EQ(right.getOperator().type, Operator::Type::BitwiseOr);
		auto &rightLeft = dynamic_cast<SymbolExpression &>(right.getLeft());
		EXPECT_EQ(rightLeft.getSymbol().s.contents, "c");
		auto &rightRight = dynamic_cast<SymbolExpression &>(right.getRight());
		EXPECT_EQ(rightRight.getSymbol().s.contents, "d");
	});
}

TEST_F(TestParser, testPrecedenceRelationalLogicalAnd) {
	std::vector<std::unique_ptr<Token>> tokens;
	tokens.push_back(std::make_unique<Keyword>(s, Keyword::Type::FUN));
	tokens.push_back(std::make_unique<Symbol>(Slice("foo", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::OpenParen));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::CloseParen));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::Colon));
	tokens.push_back(std::make_unique<Symbol>(Slice("i32", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::OpenBrace));
	tokens.push_back(std::make_unique<Symbol>(Slice("a", "", 0, 0)));
	tokens.push_back(std::make_unique<Operator>(dummy, Operator::Type::LessThan));
	tokens.push_back(std::make_unique<Symbol>(Slice("b", "", 0, 0)));
	tokens.push_back(std::make_unique<Operator>(dummy, Operator::Type::LogicalAnd));
	tokens.push_back(std::make_unique<Symbol>(Slice("c", "", 0, 0)));
	tokens.push_back(std::make_unique<Operator>(dummy, Operator::Type::GreaterThan));
	tokens.push_back(std::make_unique<Symbol>(Slice("d", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::CloseBrace));
	tokens.push_back(std::make_unique<EndOfFile>(s));
	Parser p = Parser("", std::move(tokens), &e);
	std::unique_ptr<Module> mod = p.parse();
	mod->forEachFunction([]([[maybe_unused]]
	                           std::string_view name,
	                           Function &f) {
		BlockExpression &body = f.getBody();
		Expression *expr = body.getFinalExpression();
		EXPECT_NE(expr, nullptr);
		auto *outer = dynamic_cast<BinaryExpression *>(expr);
		EXPECT_NE(outer, nullptr);
		EXPECT_EQ(outer->getOperator().type, Operator::Type::LogicalAnd);
		auto &left = dynamic_cast<BinaryExpression &>(outer->getLeft());
		EXPECT_EQ(left.getOperator().type, Operator::Type::LessThan);
		auto &leftLeft = dynamic_cast<SymbolExpression &>(left.getLeft());
		EXPECT_EQ(leftLeft.getSymbol().s.contents, "a");
		auto &leftRight = dynamic_cast<SymbolExpression &>(left.getRight());
		EXPECT_EQ(leftRight.getSymbol().s.contents, "b");
		auto &right = dynamic_cast<BinaryExpression &>(outer->getRight());
		EXPECT_EQ(right.getOperator().type, Operator::Type::GreaterThan);
		auto &rightLeft = dynamic_cast<SymbolExpression &>(right.getLeft());
		EXPECT_EQ(rightLeft.getSymbol().s.contents, "c");
		auto &rightRight = dynamic_cast<SymbolExpression &>(right.getRight());
		EXPECT_EQ(rightRight.getSymbol().s.contents, "d");
	});
}

TEST_F(TestParser, testPrecedenceLogicalAndLogicalOr) {
	std::vector<std::unique_ptr<Token>> tokens;
	tokens.push_back(std::make_unique<Keyword>(s, Keyword::Type::FUN));
	tokens.push_back(std::make_unique<Symbol>(Slice("foo", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::OpenParen));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::CloseParen));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::Colon));
	tokens.push_back(std::make_unique<Symbol>(Slice("i32", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::OpenBrace));
	tokens.push_back(std::make_unique<Symbol>(Slice("a", "", 0, 0)));
	tokens.push_back(std::make_unique<Operator>(dummy, Operator::Type::LogicalAnd));
	tokens.push_back(std::make_unique<Symbol>(Slice("b", "", 0, 0)));
	tokens.push_back(std::make_unique<Operator>(dummy, Operator::Type::LogicalOr));
	tokens.push_back(std::make_unique<Symbol>(Slice("c", "", 0, 0)));
	tokens.push_back(std::make_unique<Operator>(dummy, Operator::Type::LogicalAnd));
	tokens.push_back(std::make_unique<Symbol>(Slice("d", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::CloseBrace));
	tokens.push_back(std::make_unique<EndOfFile>(s));
	Parser p = Parser("", std::move(tokens), &e);
	std::unique_ptr<Module> mod = p.parse();
	mod->forEachFunction([]([[maybe_unused]]
	                           std::string_view name,
	                           Function &f) {
		BlockExpression &body = f.getBody();
		Expression *expr = body.getFinalExpression();
		EXPECT_NE(expr, nullptr);
		auto *outer = dynamic_cast<BinaryExpression *>(expr);
		EXPECT_NE(outer, nullptr);
		EXPECT_EQ(outer->getOperator().type, Operator::Type::LogicalOr);
		auto &left = dynamic_cast<BinaryExpression &>(outer->getLeft());
		EXPECT_EQ(left.getOperator().type, Operator::Type::LogicalAnd);
		auto &leftLeft = dynamic_cast<SymbolExpression &>(left.getLeft());
		EXPECT_EQ(leftLeft.getSymbol().s.contents, "a");
		auto &leftRight = dynamic_cast<SymbolExpression &>(left.getRight());
		EXPECT_EQ(leftRight.getSymbol().s.contents, "b");
		auto &right = dynamic_cast<BinaryExpression &>(outer->getRight());
		EXPECT_EQ(right.getOperator().type, Operator::Type::LogicalAnd);
		auto &rightLeft = dynamic_cast<SymbolExpression &>(right.getLeft());
		EXPECT_EQ(rightLeft.getSymbol().s.contents, "c");
		auto &rightRight = dynamic_cast<SymbolExpression &>(right.getRight());
		EXPECT_EQ(rightRight.getSymbol().s.contents, "d");
	});
}

TEST_F(TestParser, testPrecedenceLogicalOrAssignment) {
	std::vector<std::unique_ptr<Token>> tokens;
	tokens.push_back(std::make_unique<Keyword>(s, Keyword::Type::FUN));
	tokens.push_back(std::make_unique<Symbol>(Slice("foo", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::OpenParen));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::CloseParen));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::Colon));
	tokens.push_back(std::make_unique<Symbol>(Slice("i32", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::OpenBrace));
	tokens.push_back(std::make_unique<Symbol>(Slice("a", "", 0, 0)));
	tokens.push_back(std::make_unique<Operator>(dummy, Operator::Type::LogicalOr));
	tokens.push_back(std::make_unique<Symbol>(Slice("b", "", 0, 0)));
	tokens.push_back(std::make_unique<Operator>(dummy, Operator::Type::Assignment));
	tokens.push_back(std::make_unique<Symbol>(Slice("c", "", 0, 0)));
	tokens.push_back(std::make_unique<Operator>(dummy, Operator::Type::LogicalOr));
	tokens.push_back(std::make_unique<Symbol>(Slice("d", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::CloseBrace));
	tokens.push_back(std::make_unique<EndOfFile>(s));
	Parser p = Parser("", std::move(tokens), &e);
	std::unique_ptr<Module> mod = p.parse();
	mod->forEachFunction([]([[maybe_unused]]
	                           std::string_view name,
	                           Function &f) {
		BlockExpression &body = f.getBody();
		Expression *expr = body.getFinalExpression();
		EXPECT_NE(expr, nullptr);
		auto *outer = dynamic_cast<BinaryExpression *>(expr);
		EXPECT_NE(outer, nullptr);
		EXPECT_EQ(outer->getOperator().type, Operator::Type::Assignment);
		auto &left = dynamic_cast<BinaryExpression &>(outer->getLeft());
		EXPECT_EQ(left.getOperator().type, Operator::Type::LogicalOr);
		auto &leftLeft = dynamic_cast<SymbolExpression &>(left.getLeft());
		EXPECT_EQ(leftLeft.getSymbol().s.contents, "a");
		auto &leftRight = dynamic_cast<SymbolExpression &>(left.getRight());
		EXPECT_EQ(leftRight.getSymbol().s.contents, "b");
		auto &right = dynamic_cast<BinaryExpression &>(outer->getRight());
		EXPECT_EQ(right.getOperator().type, Operator::Type::LogicalOr);
		auto &rightLeft = dynamic_cast<SymbolExpression &>(right.getLeft());
		EXPECT_EQ(rightLeft.getSymbol().s.contents, "c");
		auto &rightRight = dynamic_cast<SymbolExpression &>(right.getRight());
		EXPECT_EQ(rightRight.getSymbol().s.contents, "d");
	});
}

TEST_F(TestParser, testPrecedenceAssignmentReturn) {
	std::vector<std::unique_ptr<Token>> tokens;
	tokens.push_back(std::make_unique<Keyword>(s, Keyword::Type::FUN));
	tokens.push_back(std::make_unique<Symbol>(Slice("foo", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::OpenParen));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::CloseParen));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::Colon));
	tokens.push_back(std::make_unique<Symbol>(Slice("i32", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::OpenBrace));
	tokens.push_back(std::make_unique<Keyword>(s, Keyword::Type::RETURN));
	tokens.push_back(std::make_unique<Symbol>(Slice("a", "", 0, 0)));
	tokens.push_back(std::make_unique<Operator>(dummy, Operator::Type::Assignment));
	tokens.push_back(std::make_unique<Symbol>(Slice("b", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::CloseBrace));
	tokens.push_back(std::make_unique<EndOfFile>(s));
	Parser p = Parser("", std::move(tokens), &e);
	std::unique_ptr<Module> mod = p.parse();
	mod->forEachFunction([]([[maybe_unused]]
	                           std::string_view name,
	                           Function &f) {
		BlockExpression &body = f.getBody();
		Expression *expr = body.getFinalExpression();
		EXPECT_NE(expr, nullptr);
		auto *ret = dynamic_cast<ReturnExpression *>(expr);
		EXPECT_NE(ret, nullptr);
		auto *assign = dynamic_cast<BinaryExpression *>(ret->getExpression());
		EXPECT_NE(assign, nullptr);
		EXPECT_EQ(assign->getOperator().type, Operator::Type::Assignment);
		auto &left = dynamic_cast<SymbolExpression &>(assign->getLeft());
		EXPECT_EQ(left.getSymbol().s.contents, "a");
		auto &right = dynamic_cast<SymbolExpression &>(assign->getRight());
		EXPECT_EQ(right.getSymbol().s.contents, "b");
	});
}

TEST_F(TestParser, testPrecedenceParenthesizedAdditionMultiplication) {
	std::vector<std::unique_ptr<Token>> tokens;
	tokens.push_back(std::make_unique<Keyword>(s, Keyword::Type::FUN));
	tokens.push_back(std::make_unique<Symbol>(Slice("foo", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::OpenParen));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::CloseParen));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::Colon));
	tokens.push_back(std::make_unique<Symbol>(Slice("i32", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::OpenBrace));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::OpenParen));
	tokens.push_back(std::make_unique<Symbol>(Slice("a", "", 0, 0)));
	tokens.push_back(std::make_unique<Operator>(dummy, Operator::Type::Addition));
	tokens.push_back(std::make_unique<Symbol>(Slice("b", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::CloseParen));
	tokens.push_back(std::make_unique<Operator>(dummy, Operator::Type::Multiplication));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::OpenParen));
	tokens.push_back(std::make_unique<Symbol>(Slice("c", "", 0, 0)));
	tokens.push_back(std::make_unique<Operator>(dummy, Operator::Type::Addition));
	tokens.push_back(std::make_unique<Symbol>(Slice("d", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::CloseParen));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::CloseBrace));
	tokens.push_back(std::make_unique<EndOfFile>(s));
	Parser p = Parser("", std::move(tokens), &e);
	std::unique_ptr<Module> mod = p.parse();
	mod->forEachFunction([]([[maybe_unused]]
	                           std::string_view name,
	                           Function &f) {
		BlockExpression &body = f.getBody();
		Expression *expr = body.getFinalExpression();
		EXPECT_NE(expr, nullptr);
		auto *outer = dynamic_cast<BinaryExpression *>(expr);
		EXPECT_NE(outer, nullptr);
		EXPECT_EQ(outer->getOperator().type, Operator::Type::Multiplication);
		auto &left = dynamic_cast<ParenthesizedExpression &>(outer->getLeft());
		auto &leftContents = dynamic_cast<BinaryExpression &>(left.getExpression());
		EXPECT_EQ(leftContents.getOperator().type, Operator::Type::Addition);
		auto &leftLeft = dynamic_cast<SymbolExpression &>(leftContents.getLeft());
		EXPECT_EQ(leftLeft.getSymbol().s.contents, "a");
		auto &leftRight = dynamic_cast<SymbolExpression &>(leftContents.getRight());
		EXPECT_EQ(leftRight.getSymbol().s.contents, "b");
		auto &right = dynamic_cast<ParenthesizedExpression &>(outer->getRight());
		auto &rightContents = dynamic_cast<BinaryExpression &>(right.getExpression());
		EXPECT_EQ(rightContents.getOperator().type, Operator::Type::Addition);
		auto &rightLeft = dynamic_cast<SymbolExpression &>(rightContents.getLeft());
		EXPECT_EQ(rightLeft.getSymbol().s.contents, "c");
		auto &rightRight = dynamic_cast<SymbolExpression &>(rightContents.getRight());
		EXPECT_EQ(rightRight.getSymbol().s.contents, "d");
	});
}

TEST_F(TestParser, testPrecedenceParenthesizedBitwiseOrAndBitshift) {
	std::vector<std::unique_ptr<Token>> tokens;
	tokens.push_back(std::make_unique<Keyword>(s, Keyword::Type::FUN));
	tokens.push_back(std::make_unique<Symbol>(Slice("foo", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::OpenParen));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::CloseParen));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::Colon));
	tokens.push_back(std::make_unique<Symbol>(Slice("i32", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::OpenBrace));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::OpenParen));
	tokens.push_back(std::make_unique<Symbol>(Slice("a", "", 0, 0)));
	tokens.push_back(std::make_unique<Operator>(dummy, Operator::Type::BitwiseOr));
	tokens.push_back(std::make_unique<Symbol>(Slice("b", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::CloseParen));
	tokens.push_back(std::make_unique<Operator>(dummy, Operator::Type::BitwiseShiftLeft));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::OpenParen));
	tokens.push_back(std::make_unique<Symbol>(Slice("c", "", 0, 0)));
	tokens.push_back(std::make_unique<Operator>(dummy, Operator::Type::BitwiseAnd));
	tokens.push_back(std::make_unique<Symbol>(Slice("d", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::CloseParen));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::CloseBrace));
	tokens.push_back(std::make_unique<EndOfFile>(s));
	Parser p = Parser("", std::move(tokens), &e);
	std::unique_ptr<Module> mod = p.parse();
	mod->forEachFunction([]([[maybe_unused]]
	                           std::string_view name,
	                           Function &f) {
		BlockExpression &body = f.getBody();
		Expression *expr = body.getFinalExpression();
		EXPECT_NE(expr, nullptr);
		auto *outer = dynamic_cast<BinaryExpression *>(expr);
		EXPECT_NE(outer, nullptr);
		EXPECT_EQ(outer->getOperator().type, Operator::Type::BitwiseShiftLeft);
		auto &left = dynamic_cast<ParenthesizedExpression &>(outer->getLeft());
		auto &leftContents = dynamic_cast<BinaryExpression &>(left.getExpression());
		EXPECT_EQ(leftContents.getOperator().type, Operator::Type::BitwiseOr);
		auto &leftLeft = dynamic_cast<SymbolExpression &>(leftContents.getLeft());
		EXPECT_EQ(leftLeft.getSymbol().s.contents, "a");
		auto &leftRight = dynamic_cast<SymbolExpression &>(leftContents.getRight());
		EXPECT_EQ(leftRight.getSymbol().s.contents, "b");
		auto &right = dynamic_cast<ParenthesizedExpression &>(outer->getRight());
		auto &rightContents = dynamic_cast<BinaryExpression &>(right.getExpression());
		EXPECT_EQ(rightContents.getOperator().type, Operator::Type::BitwiseAnd);
		auto &rightLeft = dynamic_cast<SymbolExpression &>(rightContents.getLeft());
		EXPECT_EQ(rightLeft.getSymbol().s.contents, "c");
		auto &rightRight = dynamic_cast<SymbolExpression &>(rightContents.getRight());
		EXPECT_EQ(rightRight.getSymbol().s.contents, "d");
	});
}

TEST_P(TestParserAssociativityLeftBinary, testAssociativityLeftBinary) {
	std::array<Operator::Type, 6> operators = GetParam();
	std::vector<std::unique_ptr<Token>> tokens;
	tokens.push_back(std::make_unique<Keyword>(s, Keyword::Type::FUN));
	tokens.push_back(std::make_unique<Symbol>(Slice("foo", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::OpenParen));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::CloseParen));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::Colon));
	tokens.push_back(std::make_unique<Symbol>(Slice("i32", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::OpenBrace));
	tokens.push_back(std::make_unique<Symbol>(Slice("a", "", 0, 0)));
	tokens.push_back(std::make_unique<Operator>(dummy, operators[0]));
	tokens.push_back(std::make_unique<Symbol>(Slice("b", "", 0, 0)));
	tokens.push_back(std::make_unique<Operator>(dummy, operators[1]));
	tokens.push_back(std::make_unique<Symbol>(Slice("c", "", 0, 0)));
	tokens.push_back(std::make_unique<Operator>(dummy, operators[2]));
	tokens.push_back(std::make_unique<Symbol>(Slice("d", "", 0, 0)));
	tokens.push_back(std::make_unique<Operator>(dummy, operators[3]));
	tokens.push_back(std::make_unique<Symbol>(Slice("e", "", 0, 0)));
	tokens.push_back(std::make_unique<Operator>(dummy, operators[4]));
	tokens.push_back(std::make_unique<Symbol>(Slice("f", "", 0, 0)));
	tokens.push_back(std::make_unique<Operator>(dummy, operators[5]));
	tokens.push_back(std::make_unique<Symbol>(Slice("g", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::CloseBrace));
	tokens.push_back(std::make_unique<EndOfFile>(s));
	Parser p = Parser("", std::move(tokens), &e);
	std::unique_ptr<Module> mod = p.parse();
	mod->forEachFunction([&operators]([[maybe_unused]]
	                                  std::string_view name,
	                           Function &f) {
		BlockExpression &body = f.getBody();
		Expression *expr = body.getFinalExpression();
		EXPECT_NE(expr, nullptr);
		auto *outer = dynamic_cast<BinaryExpression *>(expr);
		EXPECT_NE(outer, nullptr);
		EXPECT_EQ(outer->getOperator().type, operators[5]);
		auto &left = dynamic_cast<BinaryExpression &>(outer->getLeft());
		EXPECT_EQ(left.getOperator().type, operators[4]);
		auto &leftLeft = dynamic_cast<BinaryExpression &>(left.getLeft());
		EXPECT_EQ(leftLeft.getOperator().type, operators[3]);
		auto &leftLeftLeft = dynamic_cast<BinaryExpression &>(leftLeft.getLeft());
		EXPECT_EQ(leftLeftLeft.getOperator().type, operators[2]);
		auto &leftLeftLeftLeft = dynamic_cast<BinaryExpression &>(leftLeftLeft.getLeft());
		EXPECT_EQ(leftLeftLeftLeft.getOperator().type, operators[1]);
		auto &leftLeftLeftLeftLeft
		      = dynamic_cast<BinaryExpression &>(leftLeftLeftLeft.getLeft());
		EXPECT_EQ(leftLeftLeftLeftLeft.getOperator().type, operators[0]);
		auto &leftLeftLeftLeftLeftLeft
		      = dynamic_cast<SymbolExpression &>(leftLeftLeftLeftLeft.getLeft());
		EXPECT_EQ(leftLeftLeftLeftLeftLeft.getSymbol().s.contents, "a");
		auto &leftLeftLeftLeftLeftRight
		      = dynamic_cast<SymbolExpression &>(leftLeftLeftLeftLeft.getRight());
		EXPECT_EQ(leftLeftLeftLeftLeftRight.getSymbol().s.contents, "b");
		auto &leftLeftLeftLeftRight
		      = dynamic_cast<SymbolExpression &>(leftLeftLeftLeft.getRight());
		EXPECT_EQ(leftLeftLeftLeftRight.getSymbol().s.contents, "c");
		auto &leftLeftLeftRight
		      = dynamic_cast<SymbolExpression &>(leftLeftLeft.getRight());
		EXPECT_EQ(leftLeftLeftRight.getSymbol().s.contents, "d");
		auto &leftLeftRight = dynamic_cast<SymbolExpression &>(leftLeft.getRight());
		EXPECT_EQ(leftLeftRight.getSymbol().s.contents, "e");
		auto &leftRight = dynamic_cast<SymbolExpression &>(left.getRight());
		EXPECT_EQ(leftRight.getSymbol().s.contents, "f");
		auto &right = dynamic_cast<SymbolExpression &>(outer->getRight());
		EXPECT_EQ(right.getSymbol().s.contents, "g");
	});
}

INSTANTIATE_TEST_SUITE_P(testAssociativityLeftBinary, TestParserAssociativityLeftBinary,
      Values(std::array<Operator::Type, 6>{Operator::Type::Addition,
                   Operator::Type::Subtraction, Operator::Type::Subtraction,
                   Operator::Type::Addition, Operator::Type::Addition,
                   Operator::Type::Subtraction},
            std::array<Operator::Type, 6>{Operator::Type::Multiplication,
                  Operator::Type::Division, Operator::Type::Modulus,
                  Operator::Type::Modulus, Operator::Type::Division,
                  Operator::Type::Multiplication},
            std::array<Operator::Type, 6>{Operator::Type::BitwiseShiftLeft,
                  Operator::Type::BitwiseShiftRight, Operator::Type::BitwiseShiftRight,
                  Operator::Type::BitwiseShiftLeft, Operator::Type::BitwiseShiftLeft,
                  Operator::Type::BitwiseShiftRight},
            std::array<Operator::Type, 6>{Operator::Type::Equality,
                  Operator::Type::GreaterThanOrEqual, Operator::Type::LessThan,
                  Operator::Type::GreaterThan, Operator::Type::Inequality,
                  Operator::Type::LessThanOrEqual}));

TEST_F(TestParser, testAssociativityRightUnary) {
	std::vector<std::unique_ptr<Token>> tokens;
	tokens.push_back(std::make_unique<Keyword>(s, Keyword::Type::FUN));
	tokens.push_back(std::make_unique<Symbol>(Slice("foo", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::OpenParen));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::CloseParen));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::Colon));
	tokens.push_back(std::make_unique<Symbol>(Slice("i32", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::OpenBrace));
	tokens.push_back(std::make_unique<Operator>(dummy, Operator::Type::Subtraction));
	tokens.push_back(std::make_unique<Operator>(dummy, Operator::Type::LogicalNot));
	tokens.push_back(std::make_unique<Operator>(dummy, Operator::Type::BitwiseNot));
	tokens.push_back(std::make_unique<Operator>(dummy, Operator::Type::Addition));

	tokens.push_back(std::make_unique<Operator>(dummy, Operator::Type::LogicalNot));
	tokens.push_back(std::make_unique<Operator>(dummy, Operator::Type::BitwiseNot));
	tokens.push_back(std::make_unique<Operator>(dummy, Operator::Type::Addition));
	tokens.push_back(std::make_unique<Operator>(dummy, Operator::Type::Subtraction));
	tokens.push_back(std::make_unique<Symbol>(Slice("a", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::CloseBrace));
	tokens.push_back(std::make_unique<EndOfFile>(s));
	Parser p = Parser("", std::move(tokens), &e);
	std::unique_ptr<Module> mod = p.parse();
	mod->forEachFunction([]([[maybe_unused]]
	                           std::string_view name,
	                           Function &f) {
		BlockExpression &body = f.getBody();
		Expression *expr = body.getFinalExpression();
		EXPECT_NE(expr, nullptr);
		auto *outer = dynamic_cast<UnaryExpression *>(expr);
		EXPECT_NE(outer, nullptr);
		EXPECT_EQ(outer->getOperator().type, Operator::Type::Subtraction);
		auto &inner = dynamic_cast<UnaryExpression &>(outer->getExpression());
		EXPECT_EQ(inner.getOperator().type, Operator::Type::LogicalNot);
		auto &innerInner = dynamic_cast<UnaryExpression &>(inner.getExpression());
		EXPECT_EQ(innerInner.getOperator().type, Operator::Type::BitwiseNot);
		auto &innerInnerInner
		      = dynamic_cast<UnaryExpression &>(innerInner.getExpression());
		EXPECT_EQ(innerInnerInner.getOperator().type, Operator::Type::Addition);
		auto &innerInnerInnerInner
		      = dynamic_cast<UnaryExpression &>(innerInnerInner.getExpression());
		EXPECT_EQ(innerInnerInnerInner.getOperator().type, Operator::Type::LogicalNot);
		auto &innerInnerInnerInnerInner
		      = dynamic_cast<UnaryExpression &>(innerInnerInnerInner.getExpression());
		EXPECT_EQ(innerInnerInnerInnerInner.getOperator().type,
		      Operator::Type::BitwiseNot);
		auto &innerInnerInnerInnerInnerInner = dynamic_cast<UnaryExpression &>(
		      innerInnerInnerInnerInner.getExpression());
		EXPECT_EQ(innerInnerInnerInnerInnerInner.getOperator().type,
		      Operator::Type::Addition);
		auto &innerInnerInnerInnerInnerInnerInner = dynamic_cast<UnaryExpression &>(
		      innerInnerInnerInnerInnerInner.getExpression());
		EXPECT_EQ(innerInnerInnerInnerInnerInnerInner.getOperator().type,
		      Operator::Type::Subtraction);
		auto &symbol = dynamic_cast<SymbolExpression &>(
		      innerInnerInnerInnerInnerInnerInner.getExpression());
		EXPECT_EQ(symbol.getSymbol().s.contents, "a");
	});
}

TEST_F(TestParserError, testMissingFunKeyword) {
	std::vector<std::unique_ptr<Token>> tokens;
	tokens.push_back(std::make_unique<Symbol>(Slice("foo", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::OpenParen));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::CloseParen));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::Colon));
	tokens.push_back(std::make_unique<Symbol>(Slice("i32", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::OpenBrace));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::CloseBrace));
	tokens.push_back(std::make_unique<EndOfFile>(s));
	Parser p = Parser("", std::move(tokens), &e);
	p.parse();
	std::queue<std::tuple<std::filesystem::path, size_t, size_t, std::string>> expected;
	expected.push(std::make_tuple("", 0, 0, "Expected keyword `fun`"));
	e.checkErrors(expected);
}

TEST_F(TestParserError, testMissingFunctionName) {
	std::vector<std::unique_ptr<Token>> tokens;
	tokens.push_back(std::make_unique<Keyword>(s, Keyword::Type::FUN));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::OpenParen));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::CloseParen));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::Colon));
	tokens.push_back(std::make_unique<Symbol>(Slice("i32", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::OpenBrace));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::CloseBrace));
	tokens.push_back(std::make_unique<EndOfFile>(s));
	Parser p = Parser("", std::move(tokens), &e);
	p.parse();
	std::queue<std::tuple<std::filesystem::path, size_t, size_t, std::string>> expected;
	expected.push(std::make_tuple("", 0, 0, "Expected symbol following `fun`"));
	e.checkErrors(expected);
}

TEST_F(TestParserError, testMissingOpenParenParamList) {
	std::vector<std::unique_ptr<Token>> tokens;
	tokens.push_back(std::make_unique<Keyword>(s, Keyword::Type::FUN));
	tokens.push_back(std::make_unique<Symbol>(Slice("foo", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::CloseParen));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::Colon));
	tokens.push_back(std::make_unique<Symbol>(Slice("i32", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::OpenBrace));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::CloseBrace));
	tokens.push_back(std::make_unique<EndOfFile>(s));
	Parser p = Parser("", std::move(tokens), &e);
	p.parse();
	std::queue<std::tuple<std::filesystem::path, size_t, size_t, std::string>> expected;
	expected.push(std::make_tuple("", 0, 0,
	      "Expected '(' following symbol in function definition"));
	e.checkErrors(expected);
}

TEST_F(TestParserError, testMissingCloseParenParamList) {
	std::vector<std::unique_ptr<Token>> tokens;
	tokens.push_back(std::make_unique<Keyword>(s, Keyword::Type::FUN));
	tokens.push_back(std::make_unique<Symbol>(Slice("foo", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::OpenParen));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::Colon));
	tokens.push_back(std::make_unique<Symbol>(Slice("i32", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::OpenBrace));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::CloseBrace));
	tokens.push_back(std::make_unique<EndOfFile>(s));
	Parser p = Parser("", std::move(tokens), &e);
	p.parse();
	std::queue<std::tuple<std::filesystem::path, size_t, size_t, std::string>> expected;
	expected.push(std::make_tuple("", 0, 0, "Expected ')' in function definition"));
	e.checkErrors(expected);
}

TEST_F(TestParserError, testMissingReturnType) {
	std::vector<std::unique_ptr<Token>> tokens;
	tokens.push_back(std::make_unique<Keyword>(s, Keyword::Type::FUN));
	tokens.push_back(std::make_unique<Symbol>(Slice("foo", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::OpenParen));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::CloseParen));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::Colon));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::OpenBrace));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::CloseBrace));
	tokens.push_back(std::make_unique<EndOfFile>(s));
	Parser p = Parser("", std::move(tokens), &e);
	p.parse();
	std::queue<std::tuple<std::filesystem::path, size_t, size_t, std::string>> expected;
	expected.push(std::make_tuple("", 0, 0,
	      "Expected type symbol following ':' in function definition"));
	e.checkErrors(expected);
}

TEST_F(TestParserError, testMissingVariableNameInLet) {
	std::vector<std::unique_ptr<Token>> tokens;
	tokens.push_back(std::make_unique<Keyword>(s, Keyword::Type::FUN));
	tokens.push_back(std::make_unique<Symbol>(Slice("foo", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::OpenParen));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::CloseParen));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::Colon));
	tokens.push_back(std::make_unique<Symbol>(Slice("i32", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::OpenBrace));
	tokens.push_back(std::make_unique<Keyword>(s, Keyword::Type::LET));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::Colon));
	tokens.push_back(std::make_unique<Symbol>(Slice("i32", "", 0, 0)));
	tokens.push_back(std::make_unique<Operator>(dummy, Operator::Type::Assignment));
	tokens.push_back(std::make_unique<Symbol>(Slice("x", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::Semicolon));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::CloseBrace));
	tokens.push_back(std::make_unique<EndOfFile>(s));
	Parser p = Parser("", std::move(tokens), &e);
	p.parse();
	std::queue<std::tuple<std::filesystem::path, size_t, size_t, std::string>> expected;
	expected.push(std::make_tuple("", 0, 0, "Expected symbol following `let`"));
	e.checkErrors(expected);
}

TEST_F(TestParserError, testMissingVariableTypeInLet) {
	std::vector<std::unique_ptr<Token>> tokens;
	tokens.push_back(std::make_unique<Keyword>(s, Keyword::Type::FUN));
	tokens.push_back(std::make_unique<Symbol>(Slice("foo", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::OpenParen));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::CloseParen));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::Colon));
	tokens.push_back(std::make_unique<Symbol>(Slice("i32", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::OpenBrace));
	tokens.push_back(std::make_unique<Keyword>(s, Keyword::Type::LET));
	tokens.push_back(std::make_unique<Symbol>(Slice("x", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::Colon));
	tokens.push_back(std::make_unique<Operator>(dummy, Operator::Type::Assignment));
	tokens.push_back(std::make_unique<Symbol>(Slice("x", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::Semicolon));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::CloseBrace));
	tokens.push_back(std::make_unique<EndOfFile>(s));
	Parser p = Parser("", std::move(tokens), &e);
	p.parse();
	std::queue<std::tuple<std::filesystem::path, size_t, size_t, std::string>> expected;
	expected.push(std::make_tuple("", 0, 0,
	      "Expected type symbol following ':' in `let` statement"));
	e.checkErrors(expected);
}

TEST_F(TestParserError, testMissingExpressionAndTypeAnnotationInLet) {
	std::vector<std::unique_ptr<Token>> tokens;
	tokens.push_back(std::make_unique<Keyword>(s, Keyword::Type::FUN));
	tokens.push_back(std::make_unique<Symbol>(Slice("foo", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::OpenParen));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::CloseParen));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::Colon));
	tokens.push_back(std::make_unique<Symbol>(Slice("i32", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::OpenBrace));
	tokens.push_back(std::make_unique<Keyword>(s, Keyword::Type::LET));
	tokens.push_back(std::make_unique<Symbol>(Slice("x", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::CloseBrace));
	tokens.push_back(std::make_unique<EndOfFile>(s));
	Parser p = Parser("", std::move(tokens), &e);
	p.parse();
	std::queue<std::tuple<std::filesystem::path, size_t, size_t, std::string>> expected;
	expected.push(std::make_tuple("", 0, 0, "Expected '=' or ';' in `let` statement"));
	e.checkErrors(expected);
}

TEST_F(TestParser, testOptionalLetAssignment) {
	std::vector<std::unique_ptr<Token>> tokens;
	tokens.push_back(std::make_unique<Keyword>(s, Keyword::Type::FUN));
	tokens.push_back(std::make_unique<Symbol>(Slice("foo", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::OpenParen));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::CloseParen));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::Colon));
	tokens.push_back(std::make_unique<Symbol>(Slice("i32", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::OpenBrace));
	tokens.push_back(std::make_unique<Keyword>(s, Keyword::Type::LET));
	tokens.push_back(std::make_unique<Symbol>(Slice("x", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::Colon));
	tokens.push_back(std::make_unique<Symbol>(Slice("i32", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::Semicolon));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::CloseBrace));
	tokens.push_back(std::make_unique<EndOfFile>(s));
	Parser p = Parser("", std::move(tokens), &e);
	std::unique_ptr<Module> mod = p.parse();
	mod->forEachFunction([]([[maybe_unused]]
	                           std::string_view name,
	                           Function &f) {
		BlockExpression &body = f.getBody();
		body.forEachStatement([](Statement &statement) {
			auto *let = dynamic_cast<LetStatement *>(&statement);
			ASSERT_NE(let, nullptr);
			EXPECT_EQ(let->getExpression(), nullptr);
			EXPECT_EQ(let->getSymbol().s.contents, "x");
			EXPECT_NE(let->getTypeAnnotation(), nullptr);
			EXPECT_EQ(let->getTypeAnnotation()->s.contents, "i32");
		});
	});
}

TEST_F(TestParserError, testMissingExpressionInLet) {
	std::vector<std::unique_ptr<Token>> tokens;
	tokens.push_back(std::make_unique<Keyword>(s, Keyword::Type::FUN));
	tokens.push_back(std::make_unique<Symbol>(Slice("foo", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::OpenParen));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::CloseParen));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::Colon));
	tokens.push_back(std::make_unique<Symbol>(Slice("i32", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::OpenBrace));
	tokens.push_back(std::make_unique<Keyword>(s, Keyword::Type::LET));
	tokens.push_back(std::make_unique<Symbol>(Slice("x", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::Colon));
	tokens.push_back(std::make_unique<Symbol>(Slice("i32", "", 0, 0)));
	tokens.push_back(std::make_unique<Operator>(dummy, Operator::Type::Assignment));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::Semicolon));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::CloseBrace));
	tokens.push_back(std::make_unique<EndOfFile>(s));
	Parser p = Parser("", std::move(tokens), &e);
	p.parse();
	std::queue<std::tuple<std::filesystem::path, size_t, size_t, std::string>> expected;
	expected.push(std::make_tuple("", 0, 0, "Expected expression"));
	e.checkErrors(expected);
}

TEST_F(TestParserError, testMissingSemicolonInLet) {
	std::vector<std::unique_ptr<Token>> tokens;
	tokens.push_back(std::make_unique<Keyword>(s, Keyword::Type::FUN));
	tokens.push_back(std::make_unique<Symbol>(Slice("foo", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::OpenParen));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::CloseParen));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::Colon));
	tokens.push_back(std::make_unique<Symbol>(Slice("i32", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::OpenBrace));
	tokens.push_back(std::make_unique<Keyword>(s, Keyword::Type::LET));
	tokens.push_back(std::make_unique<Symbol>(Slice("x", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::Colon));
	tokens.push_back(std::make_unique<Symbol>(Slice("i32", "", 0, 0)));
	tokens.push_back(std::make_unique<Operator>(dummy, Operator::Type::Assignment));
	tokens.push_back(std::make_unique<Symbol>(Slice("x", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::CloseBrace));
	tokens.push_back(std::make_unique<EndOfFile>(s));
	Parser p = Parser("", std::move(tokens), &e);
	p.parse();
	std::queue<std::tuple<std::filesystem::path, size_t, size_t, std::string>> expected;
	expected.push(std::make_tuple("", 0, 0,
	      "Expected ';' following expression in `let` statement"));
	e.checkErrors(expected);
}

TEST_F(TestParser, testOptionalSemicolonInLetBlockExpression) {
	std::vector<std::unique_ptr<Token>> tokens;
	tokens.push_back(std::make_unique<Keyword>(s, Keyword::Type::FUN));
	tokens.push_back(std::make_unique<Symbol>(Slice("foo", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::OpenParen));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::CloseParen));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::Colon));
	tokens.push_back(std::make_unique<Symbol>(Slice("i32", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::OpenBrace));
	tokens.push_back(std::make_unique<Keyword>(s, Keyword::Type::LET));
	tokens.push_back(std::make_unique<Symbol>(Slice("x", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::Colon));
	tokens.push_back(std::make_unique<Symbol>(Slice("i32", "", 0, 0)));
	tokens.push_back(std::make_unique<Operator>(dummy, Operator::Type::Assignment));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::OpenBrace));
	tokens.push_back(std::make_unique<Symbol>(Slice("x", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::CloseBrace));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::CloseBrace));
	tokens.push_back(std::make_unique<EndOfFile>(s));
	Parser p1 = Parser("", std::move(tokens), &e);
	p1.parse();

	tokens.clear();
	tokens.push_back(std::make_unique<Keyword>(s, Keyword::Type::FUN));
	tokens.push_back(std::make_unique<Symbol>(Slice("foo", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::OpenParen));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::CloseParen));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::Colon));
	tokens.push_back(std::make_unique<Symbol>(Slice("i32", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::OpenBrace));
	tokens.push_back(std::make_unique<Keyword>(s, Keyword::Type::LET));
	tokens.push_back(std::make_unique<Symbol>(Slice("x", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::Colon));
	tokens.push_back(std::make_unique<Symbol>(Slice("i32", "", 0, 0)));
	tokens.push_back(std::make_unique<Operator>(dummy, Operator::Type::Assignment));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::OpenBrace));
	tokens.push_back(std::make_unique<Symbol>(Slice("x", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::CloseBrace));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::Semicolon));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::CloseBrace));
	tokens.push_back(std::make_unique<EndOfFile>(s));
	Parser p2 = Parser("", std::move(tokens), &e);
	p2.parse();
}

TEST_F(TestParserError, testMissingFunctionOpenBraces) {
	std::vector<std::unique_ptr<Token>> tokens;
	tokens.push_back(std::make_unique<Keyword>(s, Keyword::Type::FUN));
	tokens.push_back(std::make_unique<Symbol>(Slice("foo", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::OpenParen));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::CloseParen));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::Colon));
	tokens.push_back(std::make_unique<Symbol>(Slice("i32", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::CloseBrace));
	tokens.push_back(std::make_unique<EndOfFile>(s));
	Parser p = Parser("", std::move(tokens), &e);
	p.parse();
	std::queue<std::tuple<std::filesystem::path, size_t, size_t, std::string>> expected;
	expected.push(std::make_tuple("", 0, 0, "Expected '{'"));
	e.checkErrors(expected);
}

TEST_F(TestParserError, testUnclosedFunctionBracesAtEnd) {
	std::vector<std::unique_ptr<Token>> tokens;
	tokens.push_back(std::make_unique<Keyword>(s, Keyword::Type::FUN));
	tokens.push_back(std::make_unique<Symbol>(Slice("foo", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::OpenParen));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::CloseParen));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::Colon));
	tokens.push_back(std::make_unique<Symbol>(Slice("i32", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::OpenBrace));
	tokens.push_back(std::make_unique<EndOfFile>(s));
	Parser p = Parser("", std::move(tokens), &e);
	p.parse();
	std::queue<std::tuple<std::filesystem::path, size_t, size_t, std::string>> expected;
	expected.push(std::make_tuple("", 0, 0, "Expected '}'"));
	e.checkErrors(expected);
}

TEST_F(TestParserError, testUnclosedFunctionBracesAfterFinalExpression) {
	std::vector<std::unique_ptr<Token>> tokens;
	tokens.push_back(std::make_unique<Keyword>(s, Keyword::Type::FUN));
	tokens.push_back(std::make_unique<Symbol>(Slice("foo", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::OpenParen));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::CloseParen));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::Colon));
	tokens.push_back(std::make_unique<Symbol>(Slice("i32", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::OpenBrace));
	tokens.push_back(std::make_unique<Symbol>(Slice("x", "", 0, 0)));
	tokens.push_back(std::make_unique<Keyword>(s, Keyword::Type::FUN));
	tokens.push_back(std::make_unique<Symbol>(Slice("foo", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::OpenParen));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::CloseParen));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::Colon));
	tokens.push_back(std::make_unique<Symbol>(Slice("i32", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::OpenBrace));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::CloseBrace));
	tokens.push_back(std::make_unique<EndOfFile>(s));
	Parser p = Parser("", std::move(tokens), &e);
	p.parse();
	std::queue<std::tuple<std::filesystem::path, size_t, size_t, std::string>> expected;
	expected.push(std::make_tuple("", 0, 0, "Expected '}'"));
	e.checkErrors(expected);
}

TEST_F(TestParserError, testUnclosedExpressionParentheses) {
	std::vector<std::unique_ptr<Token>> tokens;
	tokens.push_back(std::make_unique<Keyword>(s, Keyword::Type::FUN));
	tokens.push_back(std::make_unique<Symbol>(Slice("foo", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::OpenParen));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::CloseParen));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::Colon));
	tokens.push_back(std::make_unique<Symbol>(Slice("i32", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::OpenBrace));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::OpenParen));
	tokens.push_back(std::make_unique<Symbol>(Slice("x", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::CloseBrace));
	tokens.push_back(std::make_unique<EndOfFile>(s));
	Parser p = Parser("", std::move(tokens), &e);
	p.parse();
	std::queue<std::tuple<std::filesystem::path, size_t, size_t, std::string>> expected;
	expected.push(std::make_tuple("", 0, 0, "Expected ')'"));
	e.checkErrors(expected);
}

TEST_F(TestParserError, testUnclosedExpressionParenthesesAtEnd) {
	std::vector<std::unique_ptr<Token>> tokens;
	tokens.push_back(std::make_unique<Keyword>(s, Keyword::Type::FUN));
	tokens.push_back(std::make_unique<Symbol>(Slice("foo", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::OpenParen));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::CloseParen));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::Colon));
	tokens.push_back(std::make_unique<Symbol>(Slice("i32", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::OpenBrace));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::OpenParen));
	tokens.push_back(std::make_unique<Symbol>(Slice("x", "", 0, 0)));
	tokens.push_back(std::make_unique<EndOfFile>(s));
	Parser p = Parser("", std::move(tokens), &e);
	p.parse();
	std::queue<std::tuple<std::filesystem::path, size_t, size_t, std::string>> expected;
	expected.push(std::make_tuple("", 0, 0, "Expected ')'"));
	expected.push(std::make_tuple("", 0, 0, "Expected '}'"));
	e.checkErrors(expected);
}

TEST_F(TestParserError, testMissingLeftBinarySubexpression) {
	std::vector<std::unique_ptr<Token>> tokens;
	tokens.push_back(std::make_unique<Keyword>(s, Keyword::Type::FUN));
	tokens.push_back(std::make_unique<Symbol>(Slice("foo", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::OpenParen));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::CloseParen));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::Colon));
	tokens.push_back(std::make_unique<Symbol>(Slice("i32", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::OpenBrace));
	tokens.push_back(std::make_unique<Operator>(dummy, Operator::Type::Multiplication));
	tokens.push_back(std::make_unique<Symbol>(Slice("x", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::CloseBrace));
	tokens.push_back(std::make_unique<EndOfFile>(s));
	Parser p = Parser("", std::move(tokens), &e);
	p.parse();
	std::queue<std::tuple<std::filesystem::path, size_t, size_t, std::string>> expected;
	expected.push(std::make_tuple("", 0, 0, "Expected expression"));
	e.checkErrors(expected);
}

TEST_F(TestParserError, testMissingRightBinarySubexpression) {
	std::vector<std::unique_ptr<Token>> tokens;
	tokens.push_back(std::make_unique<Keyword>(s, Keyword::Type::FUN));
	tokens.push_back(std::make_unique<Symbol>(Slice("foo", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::OpenParen));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::CloseParen));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::Colon));
	tokens.push_back(std::make_unique<Symbol>(Slice("i32", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::OpenBrace));
	tokens.push_back(std::make_unique<Symbol>(Slice("x", "", 0, 0)));
	tokens.push_back(std::make_unique<Operator>(dummy, Operator::Type::Multiplication));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::CloseBrace));
	tokens.push_back(std::make_unique<EndOfFile>(s));
	Parser p = Parser("", std::move(tokens), &e);
	p.parse();
	std::queue<std::tuple<std::filesystem::path, size_t, size_t, std::string>> expected;
	expected.push(std::make_tuple("", 0, 0, "Expected expression"));
	e.checkErrors(expected);
}

TEST_F(TestParserError, testMissingBothBinarySubexpressions) {
	std::vector<std::unique_ptr<Token>> tokens;
	tokens.push_back(std::make_unique<Keyword>(s, Keyword::Type::FUN));
	tokens.push_back(std::make_unique<Symbol>(Slice("foo", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::OpenParen));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::CloseParen));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::Colon));
	tokens.push_back(std::make_unique<Symbol>(Slice("i32", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::OpenBrace));
	tokens.push_back(std::make_unique<Operator>(dummy, Operator::Type::Multiplication));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::CloseBrace));
	tokens.push_back(std::make_unique<EndOfFile>(s));
	Parser p = Parser("", std::move(tokens), &e);
	p.parse();
	std::queue<std::tuple<std::filesystem::path, size_t, size_t, std::string>> expected;
	expected.push(std::make_tuple("", 0, 0, "Expected expression"));
	e.checkErrors(expected);
}

TEST_F(TestParserError, testMissingUnarySubexpression) {
	std::vector<std::unique_ptr<Token>> tokens;
	tokens.push_back(std::make_unique<Keyword>(s, Keyword::Type::FUN));
	tokens.push_back(std::make_unique<Symbol>(Slice("foo", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::OpenParen));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::CloseParen));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::Colon));
	tokens.push_back(std::make_unique<Symbol>(Slice("i32", "", 0, 0)));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::OpenBrace));
	tokens.push_back(std::make_unique<Operator>(dummy, Operator::Type::LogicalNot));
	tokens.push_back(std::make_unique<Punctuation>(s, Punctuation::Type::CloseBrace));
	tokens.push_back(std::make_unique<EndOfFile>(s));
	Parser p = Parser("", std::move(tokens), &e);
	p.parse();
	std::queue<std::tuple<std::filesystem::path, size_t, size_t, std::string>> expected;
	expected.push(std::make_tuple("", 0, 0, "Expected expression"));
	e.checkErrors(expected);
}

// TODO missing type annotations should not be an error
