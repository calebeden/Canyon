// #ifndef DEBUG_TEST_MODE
// #	error "DEBUG_TEST_MODE not defined"
// #endif

// #include "ast.h"
// #include "parser.h"
// #include "tokens.h"

// #include "gmock/gmock.h"
// #include "gtest/gtest.h"

// #include <cstdio>
// #include <cstdlib>
// #include <cstring>
// #include <iostream>
// #include <vector>

// using namespace AST;

// struct EmptyRvalue : public rvalue {
// 	EmptyRvalue() : rvalue("", 0, 0) {
// 	}

// 	virtual void print([[maybe_unused]] std::ostream &os) const {
// 	}

// 	virtual void compile([[maybe_unused]] std::ostream &outfile) const {
// 	}

// 	virtual Type typeCheck([[maybe_unused]] const CodeBlock &context,
// 	      [[maybe_unused]] ErrorHandler &errors) const {
// 		return Type::UNKNOWN;
// 	}
// };

// struct EmptyStatement : public Statement {
// 	virtual void print([[maybe_unused]] std::ostream &os) const {
// 	}

// 	virtual void compile([[maybe_unused]] std::ostream &outfile) const {
// 	}

// 	virtual Type typeCheck([[maybe_unused]] const CodeBlock &context,
// 	      [[maybe_unused]] Type returnType, [[maybe_unused]] ErrorHandler &errors) const {
// 		return Type::UNKNOWN;
// 	}
// };

// struct EmptyFunction : public Function {
// 	explicit EmptyFunction(Module *module) : Function(module) {
// 	}
// };

// TEST(test_parser, test_parseModule) {
// 	class MockParser : public Parser {
// 	public:
// 		MOCK_METHOD(void, parseFunctions,
// 		      (std::vector<Token *> & tokens, Module &module));
// 	};

// 	std::vector<Token *> tokens;

// 	MockParser p;
// 	Function *function;
// 	EXPECT_CALL(p, parseFunctions)
// 	      .WillOnce(::testing::Invoke([&function]([[maybe_unused]]
// 	                                              std::vector<Token *> &tokens,
// 	                                        Module &module) {
// 		      function = new EmptyFunction(&module);
// 		      module.functions["canyonMain"] = function;
// 	      }));

// 	Module module = p.parseModule(tokens);
// 	// main + print
// 	EXPECT_EQ(module.functions.size(), 2);
// 	Function *main = module.functions["canyonMain"];
// 	EXPECT_EQ(main, function);
// }

// TEST(test_parser, test_parseModule_error) {
// 	std::vector<Token *> tokens;
// 	Parser p;

// 	// Test 1: no main function defined
// 	EXPECT_EXIT(p.parseModule(tokens), ::testing::ExitedWithCode(EXIT_FAILURE),
// 	      "Parse error: no main function");
// }

// TEST(test_parser, test_parseFunctions) {
// 	class MockParser : public Parser {
// 	public:
// 		MOCK_METHOD(void, parseFunction,
// 		      (std::vector<Token *>::iterator & it, Module &module));
// 	};

// 	std::vector<Token *> tokens;
// 	Module module;

// 	MockParser p;
// 	tokens = {};
// 	tokens.push_back(new Identifier(Slice("x", "", 0, 0)));
// 	EXPECT_CALL(p, parseFunction)
// 	      .WillOnce(::testing::DoDefault())
// 	      .WillOnce(::testing::DoDefault())
// 	      .WillOnce(::testing::DoDefault())
// 	      .WillOnce(::testing::DoDefault())
// 	      .WillOnce(::testing::DoDefault())
// 	      .WillOnce(::testing::DoDefault())
// 	      .WillOnce(
// 	            ::testing::Invoke([](std::vector<Token *>::iterator &it, [[maybe_unused]]
// 	                                                                     Module &module) {
// 		            it++;
// 	            }));

// 	p.parseFunctions(tokens, module);
// }

// TEST(test_parser, test_parseFunction) {
// 	class MockParser : public Parser {
// 	public:
// 		MOCK_METHOD(void, parseParameters,
// 		      (std::vector<Token *>::iterator & it, Function *function));
// 		MOCK_METHOD(void, parseBlock,
// 		      (std::vector<Token *>::iterator & it, CodeBlock &context));
// 	};

// 	std::vector<Token *> tokens;
// 	std::vector<Token *>::iterator it;
// 	Module module;
// 	Identifier *param;
// 	Function *function;

// 	// Test 1: Normal function parsing
// 	// 1a: Primitive return type
// 	MockParser p1a;
// 	tokens = {};
// 	module = {};
// 	tokens.push_back(new Primitive(Slice("int", "", 0, 0), Type::INT));
// 	tokens.push_back(new Identifier(Slice("foo", "", 0, 0)));
// 	param = new Identifier(Slice("x", "", 0, 0));
// 	EXPECT_CALL(p1a, parseParameters)
// 	      .WillOnce(::testing::Invoke([param]([[maybe_unused]]
// 	                                          std::vector<Token *>::iterator &it,
// 	                                        Function *function) {
// 		      function->parameters.emplace_back(param, Type::INT);
// 	      }));
// 	Statement *statement = new EmptyStatement();
// 	EXPECT_CALL(p1a, parseBlock)
// 	      .WillOnce(::testing::Invoke(
// 	            [statement]([[maybe_unused]]
// 	                                               std::vector<Token *>::iterator &it,
// 	                  CodeBlock &context) {
// 		            context.statements.push_back(std::unique_ptr<Statement>(statement));
// 	            }));
// 	it = tokens.begin();

// 	p1a.parseFunction(it, module);

// 	EXPECT_EQ(it, tokens.end());
// 	// foo + print
// 	EXPECT_EQ(module.functions.size(), 2);
// 	if (module.functions.size() >= 2) {
// 		function = module.functions["foo"];
// 		EXPECT_EQ(function->type, Type::INT);
// 		EXPECT_EQ(function->parameters.size(), 1);
// 		if (function->parameters.size() >= 1) {
// 			EXPECT_EQ(function->parameters[0].first->s, "x");
// 			EXPECT_EQ(function->parameters[0].second, Type::INT);
// 		}
// 		EXPECT_EQ(function->body->statements.size(), 1);
// 		if (function->body->statements.size() >= 1) {
// 			EXPECT_EQ(function->body->statements[0].get(), statement);
// 		}
// 	}

// 	// 1b: void return type
// 	MockParser p1b;
// 	tokens = {};
// 	module = {};
// 	tokens.push_back(new Keyword(Slice("void", "", 0, 0), Keyword::Type::VOID));
// 	tokens.push_back(new Identifier(Slice("foo", "", 0, 0)));
// 	param = new Identifier(Slice("x", "", 0, 0));
// 	EXPECT_CALL(p1b, parseParameters)
// 	      .WillOnce(::testing::Invoke([param]([[maybe_unused]]
// 	                                          std::vector<Token *>::iterator &it,
// 	                                        Function *function) {
// 		      function->parameters.emplace_back(param, Type::INT);
// 	      }));
// 	statement = new EmptyStatement();
// 	EXPECT_CALL(p1b, parseBlock)
// 	      .WillOnce(::testing::Invoke(
// 	            [statement ]([[maybe_unused]]
// 	                                               std::vector<Token *>::iterator &it,
// 	                  CodeBlock &context) {
// 		            context.statements.push_back(std::unique_ptr<Statement>(statement));
// 	            }));
// 	it = tokens.begin();

// 	p1b.parseFunction(it, module);

// 	EXPECT_EQ(it, tokens.end());
// 	// foo + print
// 	EXPECT_EQ(module.functions.size(), 2);
// 	if (module.functions.size() >= 2) {
// 		function = module.functions["foo"];
// 		EXPECT_EQ(function->type, Type::VOID);
// 		EXPECT_EQ(function->parameters.size(), 1);
// 		if (function->parameters.size() >= 1) {
// 			EXPECT_EQ(function->parameters[0].first->s, "x");
// 			EXPECT_EQ(function->parameters[0].second, Type::INT);
// 		}
// 		EXPECT_EQ(function->body->statements.size(), 1);
// 		if (function->body->statements.size() >= 1) {
// 			EXPECT_EQ(function->body->statements[0].get(), statement);
// 		}
// 	}

// 	// Test 2: main gets mangled to canyonMain
// 	MockParser p2;
// 	tokens = {};
// 	module = {};
// 	tokens.push_back(new Keyword(Slice("void", "", 0, 0), Keyword::Type::VOID));
// 	tokens.push_back(new Identifier(Slice("main", "", 0, 0)));
// 	EXPECT_CALL(p2, parseParameters).Times(1);
// 	EXPECT_CALL(p2, parseBlock).Times(1);
// 	it = tokens.begin();

// 	p2.parseFunction(it, module);

// 	EXPECT_EQ(it, tokens.end());
// 	// main + print
// 	EXPECT_EQ(module.functions.size(), 2);
// 	if (module.functions.size() >= 2) {
// 		function = module.functions["canyonMain"];
// 		EXPECT_EQ(function->type, Type::VOID);
// 		EXPECT_EQ(function->parameters.size(), 0);
// 		EXPECT_EQ(function->body->statements.size(), 0);
// 	}
// }

// TEST(test_parser, test_parseFunction_error) {
// 	std::vector<Token *> tokens;
// 	std::vector<Token *>::iterator it;
// 	Module module;
// 	Parser p;

// 	// Test 1: Missing function type
// 	tokens = {};
// 	tokens.push_back(new Identifier(Slice("x", "", 0, 0)));
// 	it = tokens.begin();

// 	EXPECT_EXIT(p.parseFunction(it, module), ::testing::ExitedWithCode(EXIT_FAILURE),
// 	      "Expected function type");

// 	// Test 2: Missing function name
// 	// 2a: primitive type
// 	tokens = {};
// 	tokens.push_back(new Primitive(Slice("int", "", 0, 0), Type::INT));
// 	tokens.push_back(new Punctuation(Slice("(", "", 0, 0), Punctuation::Type::OpenParen));
// 	it = tokens.begin();

// 	EXPECT_EXIT(p.parseFunction(it, module), ::testing::ExitedWithCode(EXIT_FAILURE),
// 	      "Expected identifier");

// 	// 2a: void type
// 	tokens = {};
// 	tokens.push_back(new Keyword(Slice("void", "", 0, 0), Keyword::Type::VOID));
// 	tokens.push_back(new Punctuation(Slice("(", "", 0, 0), Punctuation::Type::OpenParen));
// 	it = tokens.begin();

// 	EXPECT_EXIT(p.parseFunction(it, module), ::testing::ExitedWithCode(EXIT_FAILURE),
// 	      "Expected identifier");
// }

// TEST(test_parser, test_parseParameters) {
// 	std::vector<Token *> tokens;
// 	std::vector<Token *>::iterator it;
// 	Module module;
// 	Function *function = new Function(&module);
// 	Parser p;

// 	tokens = {};
// 	tokens.push_back(new Punctuation(Slice("(", "", 0, 0), Punctuation::Type::OpenParen));
// 	tokens.push_back(new Primitive(Slice("int", "", 0, 0), Type::INT));
// 	tokens.push_back(new Identifier(Slice("x", "", 0, 0)));
// 	tokens.push_back(new Punctuation(Slice(",", "", 0, 0), Punctuation::Type::Comma));
// 	tokens.push_back(new Primitive(Slice("float", "", 0, 0), Type::FLOAT));
// 	tokens.push_back(new Identifier(Slice("y", "", 0, 0)));
// 	tokens.push_back(new Punctuation(Slice(",", "", 0, 0), Punctuation::Type::Comma));
// 	tokens.push_back(new Primitive(Slice("char", "", 0, 0), Type::CHAR));
// 	tokens.push_back(new Identifier(Slice("z", "", 0, 0)));
// 	tokens.push_back(new Punctuation(Slice(",", "", 0, 0), Punctuation::Type::Comma));
// 	tokens.push_back(new Primitive(Slice("double", "", 0, 0), Type::DOUBLE));
// 	tokens.push_back(new Identifier(Slice("a", "", 0, 0)));
// 	tokens.push_back(new Punctuation(Slice(",", "", 0, 0), Punctuation::Type::Comma));
// 	tokens.push_back(new Primitive(Slice("long", "", 0, 0), Type::LONG));
// 	tokens.push_back(new Identifier(Slice("b", "", 0, 0)));
// 	tokens.push_back(new Punctuation(Slice(",", "", 0, 0), Punctuation::Type::Comma));
// 	tokens.push_back(new Primitive(Slice("bool", "", 0, 0), Type::BOOL));
// 	tokens.push_back(new Identifier(Slice("c", "", 0, 0)));
// 	tokens.push_back(
// 	      new Punctuation(Slice(")", "", 0, 0), Punctuation::Type::CloseParen));
// 	it = tokens.begin();

// 	p.parseParameters(it, function);
// 	EXPECT_EQ(it, tokens.end());
// 	EXPECT_EQ(function->parameters.size(), 6);
// 	if (function->parameters.size() >= 6) {
// 		EXPECT_EQ(function->parameters[0].first->s, "x");
// 		EXPECT_EQ(function->parameters[0].second, Type::INT);
// 		EXPECT_EQ(function->parameters[1].first->s, "y");
// 		EXPECT_EQ(function->parameters[1].second, Type::FLOAT);
// 		EXPECT_EQ(function->parameters[2].first->s, "z");
// 		EXPECT_EQ(function->parameters[2].second, Type::CHAR);
// 		EXPECT_EQ(function->parameters[3].first->s, "a");
// 		EXPECT_EQ(function->parameters[3].second, Type::DOUBLE);
// 		EXPECT_EQ(function->parameters[4].first->s, "b");
// 		EXPECT_EQ(function->parameters[4].second, Type::LONG);
// 		EXPECT_EQ(function->parameters[5].first->s, "c");
// 		EXPECT_EQ(function->parameters[5].second, Type::BOOL);
// 	}
// }

// TEST(test_parser, test_parseParameters_error) {
// 	std::vector<Token *> tokens;
// 	std::vector<Token *>::iterator it;
// 	Module module;
// 	Function *function = new Function(&module);
// 	Parser p;

// 	// Test 1: Missing open parenthesis
// 	tokens = {};
// 	tokens.push_back(new Identifier(Slice("x", "", 0, 0)));
// 	it = tokens.begin();

// 	EXPECT_EXIT(p.parseParameters(it, function), ::testing::ExitedWithCode(EXIT_FAILURE),
// 	      "Expected '\\('");

// 	// Test 2: Redeclaration of parameter of same type
// 	tokens = {};
// 	module = {};
// 	function = new Function(&module);
// 	tokens.push_back(new Punctuation(Slice("(", "", 0, 0), Punctuation::Type::OpenParen));
// 	tokens.push_back(new Primitive(Slice("int", "", 0, 0), Type::INT));
// 	tokens.push_back(new Identifier(Slice("x", "", 0, 0)));
// 	tokens.push_back(new Punctuation(Slice(",", "", 0, 0), Punctuation::Type::Comma));
// 	tokens.push_back(new Primitive(Slice("int", "", 0, 0), Type::INT));
// 	tokens.push_back(new Identifier(Slice("x", "", 0, 0)));
// 	it = tokens.begin();

// 	EXPECT_EXIT(p.parseParameters(it, function), ::testing::ExitedWithCode(EXIT_FAILURE),
// 	      "Re-declaration of parameter x");

// 	// Test 3: Redeclaration of parameter of different type
// 	tokens = {};
// 	module = {};
// 	function = new Function(&module);
// 	tokens.push_back(new Punctuation(Slice("(", "", 0, 0), Punctuation::Type::OpenParen));
// 	tokens.push_back(new Primitive(Slice("int", "", 0, 0), Type::INT));
// 	tokens.push_back(new Identifier(Slice("x", "", 0, 0)));
// 	tokens.push_back(new Punctuation(Slice(",", "", 0, 0), Punctuation::Type::Comma));
// 	tokens.push_back(new Primitive(Slice("float", "", 0, 0), Type::FLOAT));
// 	tokens.push_back(new Identifier(Slice("x", "", 0, 0)));
// 	it = tokens.begin();

// 	EXPECT_EXIT(p.parseParameters(it, function), ::testing::ExitedWithCode(EXIT_FAILURE),
// 	      "Re-declaration of parameter x");

// 	// Test 4: Missing parameter type
// 	tokens = {};
// 	tokens.push_back(new Punctuation(Slice("(", "", 0, 0), Punctuation::Type::OpenParen));
// 	tokens.push_back(new Identifier(Slice("x", "", 0, 0)));
// 	it = tokens.begin();

// 	EXPECT_EXIT(p.parseParameters(it, function), ::testing::ExitedWithCode(EXIT_FAILURE),
// 	      "Expected primitive");

// 	// Test 5: Missing parameter name
// 	tokens = {};
// 	module = {};
// 	function = new Function(&module);
// 	tokens.push_back(new Punctuation(Slice("(", "", 0, 0), Punctuation::Type::OpenParen));
// 	tokens.push_back(new Primitive(Slice("int", "", 0, 0), Type::INT));
// 	tokens.push_back(new Punctuation(Slice(",", "", 0, 0), Punctuation::Type::Comma));
// 	it = tokens.begin();

// 	EXPECT_EXIT(p.parseParameters(it, function), ::testing::ExitedWithCode(EXIT_FAILURE),
// 	      "Unexpected token following primitive");

// 	// Test 6: Unexpected token following parameter declaration
// 	// 6a: other punctuation
// 	tokens = {};
// 	module = {};
// 	function = new Function(&module);
// 	tokens.push_back(new Punctuation(Slice("(", "", 0, 0), Punctuation::Type::OpenParen));
// 	tokens.push_back(new Primitive(Slice("int", "", 0, 0), Type::INT));
// 	tokens.push_back(new Identifier(Slice("x", "", 0, 0)));
// 	tokens.push_back(new Punctuation(Slice("+", "", 0, 0), Punctuation::Type::Plus));
// 	it = tokens.begin();

// 	EXPECT_EXIT(p.parseParameters(it, function), ::testing::ExitedWithCode(EXIT_FAILURE),
// 	      "Unexpected token; expected '\\)' or ','");

// 	// 6b: Identifier
// 	tokens = {};
// 	module = {};
// 	function = new Function(&module);
// 	tokens.push_back(new Punctuation(Slice("(", "", 0, 0), Punctuation::Type::OpenParen));
// 	tokens.push_back(new Primitive(Slice("int", "", 0, 0), Type::INT));
// 	tokens.push_back(new Identifier(Slice("x", "", 0, 0)));
// 	tokens.push_back(new Identifier(Slice("y", "", 0, 0)));
// 	it = tokens.begin();

// 	EXPECT_EXIT(p.parseParameters(it, function), ::testing::ExitedWithCode(EXIT_FAILURE),
// 	      "Unexpected token; expected '\\)' or ','");

// 	// 6c: Primitive
// 	tokens = {};
// 	module = {};
// 	function = new Function(&module);
// 	tokens.push_back(new Punctuation(Slice("(", "", 0, 0), Punctuation::Type::OpenParen));
// 	tokens.push_back(new Primitive(Slice("int", "", 0, 0), Type::INT));
// 	tokens.push_back(new Identifier(Slice("x", "", 0, 0)));
// 	tokens.push_back(new Primitive(Slice("int", "", 0, 0), Type::INT));
// 	it = tokens.begin();

// 	EXPECT_EXIT(p.parseParameters(it, function), ::testing::ExitedWithCode(EXIT_FAILURE),
// 	      "Unexpected token; expected '\\)' or ','");

// 	// 6d: Keyword
// 	tokens = {};
// 	module = {};
// 	function = new Function(&module);
// 	tokens.push_back(new Punctuation(Slice("(", "", 0, 0), Punctuation::Type::OpenParen));
// 	tokens.push_back(new Primitive(Slice("int", "", 0, 0), Type::INT));
// 	tokens.push_back(new Identifier(Slice("x", "", 0, 0)));
// 	tokens.push_back(new Keyword(Slice("return", "", 0, 0), Keyword::Type::RETURN));
// 	it = tokens.begin();

// 	EXPECT_EXIT(p.parseParameters(it, function), ::testing::ExitedWithCode(EXIT_FAILURE),
// 	      "Unexpected token; expected '\\)' or ','");
// }

// TEST(test_parser, test_parseBlock) {
// 	class MockParser : public Parser {
// 	public:
// 		MOCK_METHOD(std::unique_ptr<Statement>, parseStatement,
// 		      (std::vector<Token *>::iterator & it, CodeBlock &context));
// 	};

// 	std::vector<Token *> tokens;
// 	std::vector<Token *>::iterator it;
// 	Module module;
// 	CodeBlock context = CodeBlock(&module);
// 	std::unique_ptr<Statement> statement1 = std::make_unique<EmptyStatement>();
// 	Statement *copy1 = statement1.get();
// 	std::unique_ptr<Statement> statement2 = std::make_unique<EmptyStatement>();
// 	Statement *copy2 = statement2.get();
// 	std::unique_ptr<Statement> statement3 = std::make_unique<EmptyStatement>();
// 	Statement *copy3 = statement3.get();
// 	std::unique_ptr<Statement> statement4 = std::make_unique<EmptyStatement>();
// 	Statement *copy4 = statement4.get();
// 	std::unique_ptr<Statement> statement5 = std::make_unique<EmptyStatement>();
// 	Statement *copy5 = statement5.get();
// 	std::unique_ptr<Statement> statement6 = std::make_unique<EmptyStatement>();
// 	Statement *copy6 = statement6.get();
// 	std::unique_ptr<Statement> statement7 = std::make_unique<EmptyStatement>();
// 	Statement *copy7 = statement7.get();
// 	std::unique_ptr<Statement> statement8 = std::make_unique<EmptyStatement>();
// 	Statement *copy8 = statement8.get();

// 	tokens = {};
// 	MockParser p;
// 	tokens.push_back(new Punctuation(Slice("{", "", 0, 0), Punctuation::Type::OpenBrace));
// 	tokens.push_back(new Identifier(Slice("x", "", 0, 0)));
// 	tokens.push_back(
// 	      new Punctuation(Slice("}", "", 0, 0), Punctuation::Type::CloseBrace));
// 	EXPECT_CALL(p, parseStatement)
// 	      .WillOnce(::testing::Return(std::move(statement1)))
// 	      .WillOnce(::testing::Return(nullptr))
// 	      .WillOnce(::testing::Return(std::move(statement2)))
// 	      .WillOnce(::testing::Return(std::move(statement3)))
// 	      .WillOnce(::testing::Return(std::move(statement4)))
// 	      .WillOnce(::testing::Return(std::move(statement5)))
// 	      .WillOnce(::testing::Return(nullptr))
// 	      .WillOnce(::testing::Return(nullptr))
// 	      .WillOnce(::testing::Return(std::move(statement6)))
// 	      .WillOnce(::testing::Return(std::move(statement7)))
// 	      .WillOnce(::testing::DoAll(
// 	            ::testing::Invoke(
// 	                  [statement8 = std::move(statement8)](
// 	                        std::vector<Token *>::iterator &it, [[maybe_unused]]
// 	                                                            CodeBlock &context) {
// 		                  it++;
// 	                  }),
// 	            ::testing::Return(std::move(statement8))));
// 	it = tokens.begin();

// 	p.parseBlock(it, context);
// 	EXPECT_EQ(it, tokens.end());
// 	EXPECT_EQ(context.statements.size(), 8);
// 	if (context.statements.size() >= 8) {
// 		EXPECT_EQ(context.statements[0].get(), copy1);
// 		EXPECT_EQ(context.statements[1].get(), copy2);
// 		EXPECT_EQ(context.statements[2].get(), copy3);
// 		EXPECT_EQ(context.statements[3].get(), copy4);
// 		EXPECT_EQ(context.statements[4].get(), copy5);
// 		EXPECT_EQ(context.statements[5].get(), copy6);
// 		EXPECT_EQ(context.statements[6].get(), copy7);
// 		EXPECT_EQ(context.statements[7].get(), copy8);
// 	}
// }

// TEST(test_parser, test_parseBlock_error) {
// 	std::vector<Token *> tokens;
// 	std::vector<Token *>::iterator it;
// 	Module module;
// 	CodeBlock context = CodeBlock(&module);
// 	Parser p;

// 	// Test 1: Missing open brace
// 	tokens = {};
// 	tokens.push_back(new Identifier(Slice("x", "", 0, 0)));
// 	it = tokens.begin();

// 	EXPECT_EXIT(p.parseBlock(it, context), ::testing::ExitedWithCode(EXIT_FAILURE),
// 	      "Expected '\\{'");
// }

// TEST(test_parser, test_parseStatement) {
// 	class MockParser : public Parser {
// 	public:
// 		MOCK_METHOD(std::unique_ptr<rvalue>, parseRvalue,
// 		      (std::vector<Token *>::iterator & it, CodeBlock &context));
// 	};

// 	std::vector<Token *> tokens;
// 	std::vector<Token *>::iterator it;
// 	Module module;
// 	CodeBlock context = CodeBlock(&module);
// 	Expression *expression;
// 	Return *returnStatement;

// 	// Test 1: Regular expression statement
// 	MockParser p1;
// 	tokens = {};
// 	tokens.push_back(new Identifier(Slice("1", "", 0, 0)));
// 	tokens.push_back(new Punctuation(Slice(";", "", 0, 0), Punctuation::Type::Semicolon));
// 	std::unique_ptr<rvalue> toReturn = std::make_unique<EmptyRvalue>();
// 	rvalue *copy = toReturn.get();
// 	EXPECT_CALL(p1, parseRvalue)
// 	      .WillOnce(
// 	            ::testing::DoAll(::testing::Invoke([](std::vector<Token *>::iterator &it,
// 	                                                     [[maybe_unused]]
// 	                                                     CodeBlock &context) {
// 		            it++;
// 	            }),
// 	                  ::testing::Return(std::move(toReturn))));
// 	it = tokens.begin();

// 	std::unique_ptr<Statement> statement = p1.parseStatement(it, context);

// 	EXPECT_EQ(it, tokens.end());
// 	expression = dynamic_cast<Expression *>(statement.get());
// 	EXPECT_NE(expression, nullptr);
// 	if (expression != nullptr) {
// 		EXPECT_EQ(expression->rval.get(), copy);
// 	}

// 	// Test 2: Return (void) statement
// 	MockParser p2;
// 	tokens = {};
// 	tokens.push_back(new Keyword(Slice("return", "", 0, 0), Keyword::Type::RETURN));
// 	tokens.push_back(new Punctuation(Slice(";", "", 0, 0), Punctuation::Type::Semicolon));
// 	EXPECT_CALL(p2, parseRvalue).WillOnce(::testing::Return(nullptr));
// 	it = tokens.begin();

// 	statement = p2.parseStatement(it, context);

// 	EXPECT_EQ(it, tokens.end());
// 	returnStatement = dynamic_cast<Return *>(statement.get());
// 	EXPECT_NE(returnStatement, nullptr);
// 	if (returnStatement != nullptr) {
// 		EXPECT_EQ(returnStatement->rval, nullptr);
// 	}

// 	// Test 3: Return expression statement
// 	MockParser p3;
// 	tokens = {};
// 	tokens.push_back(new Keyword(Slice("return", "", 0, 0), Keyword::Type::RETURN));
// 	tokens.push_back(new Identifier(Slice("1", "", 0, 0)));
// 	tokens.push_back(new Punctuation(Slice(";", "", 0, 0), Punctuation::Type::Semicolon));
// 	toReturn = std::make_unique<EmptyRvalue>();
// 	copy = toReturn.get();
// 	EXPECT_CALL(p3, parseRvalue)
// 	      .WillOnce(
// 	            ::testing::DoAll(::testing::Invoke([](std::vector<Token *>::iterator &it,
// 	                                                     [[maybe_unused]]
// 	                                                     CodeBlock &context) {
// 		            it++;
// 	            }),
// 	                  ::testing::Return(std::move(toReturn))));
// 	it = tokens.begin();

// 	statement = p3.parseStatement(it, context);

// 	EXPECT_EQ(it, tokens.end());
// 	returnStatement = dynamic_cast<Return *>(statement.get());
// 	EXPECT_NE(returnStatement, nullptr);
// 	if (returnStatement != nullptr) {
// 		EXPECT_EQ(returnStatement->rval.get(), copy);
// 	}

// 	// Test 4: Skips extra semicolons
// 	MockParser p4;
// 	tokens = {};
// 	tokens.push_back(new Punctuation(Slice(";", "", 0, 0), Punctuation::Type::Semicolon));
// 	tokens.push_back(new Punctuation(Slice(";", "", 0, 0), Punctuation::Type::Semicolon));
// 	tokens.push_back(new Punctuation(Slice(";", "", 0, 0), Punctuation::Type::Semicolon));
// 	tokens.push_back(new Identifier(Slice("1", "", 0, 0)));
// 	tokens.push_back(new Punctuation(Slice(";", "", 0, 0), Punctuation::Type::Semicolon));
// 	toReturn = std::make_unique<EmptyRvalue>();
// 	copy = toReturn.get();
// 	EXPECT_CALL(p4, parseRvalue)
// 	      .WillOnce(
// 	            ::testing::DoAll(::testing::Invoke([](std::vector<Token *>::iterator &it,
// 	                                                     [[maybe_unused]]
// 	                                                     CodeBlock &context) {
// 		            it++;
// 	            }),
// 	                  ::testing::Return(std::move(toReturn))));
// 	it = tokens.begin();

// 	statement = p4.parseStatement(it, context);

// 	EXPECT_EQ(it, tokens.end());
// 	expression = dynamic_cast<Expression *>(statement.get());
// 	EXPECT_NE(expression, nullptr);
// 	if (expression != nullptr) {
// 		EXPECT_EQ(dynamic_cast<Expression *>(statement.get())->rval, toReturn);
// 	}

// 	// Test 5: Returns nullptr at close brace
// 	MockParser p5;
// 	tokens = {};
// 	tokens.push_back(
// 	      new Punctuation(Slice("}", "", 0, 0), Punctuation::Type::CloseBrace));
// 	EXPECT_CALL(p5, parseRvalue).Times(0);
// 	it = tokens.begin();

// 	statement = p5.parseStatement(it, context);

// 	EXPECT_EQ(it, tokens.begin());
// 	EXPECT_EQ(statement, nullptr);

// 	// Test 6: Variable declaration
// 	MockParser p6;
// 	tokens = {};
// 	module = {};
// 	context = CodeBlock(&module);
// 	tokens.push_back(new Primitive(Slice("int", "", 0, 0), Type::INT));
// 	tokens.push_back(new Identifier(Slice("x", "", 0, 0)));
// 	tokens.push_back(new Punctuation(Slice(";", "", 0, 0), Punctuation::Type::Semicolon));
// 	EXPECT_CALL(p6, parseRvalue)
// 	      .WillOnce(
// 	            ::testing::Invoke([](std::vector<Token *>::iterator &it,
// 	                                    [[maybe_unused]]
// 	                                    CodeBlock &context) -> std::unique_ptr<rvalue> {
// 		            it++;
// 		            return std::make_unique<Variable>(
// 		                  new Identifier(Slice("x", "", 0, 0)));
// 	            }));
// 	it = tokens.begin();

// 	statement = p6.parseStatement(it, context);

// 	EXPECT_EQ(it, tokens.end());
// 	EXPECT_EQ(statement, nullptr);
// 	EXPECT_NE(context.locals.find(new Identifier(Slice("x", "", 0, 0))),
// 	      context.locals.end());

// 	// Test 7: Variable declaration assignment
// 	MockParser p7;
// 	tokens = {};
// 	module = {};
// 	context = CodeBlock(&module);
// 	tokens.push_back(new Primitive(Slice("int", "", 0, 0), Type::INT));
// 	tokens.push_back(new Identifier(Slice("x", "", 0, 0)));
// 	tokens.push_back(new Punctuation(Slice(";", "", 0, 0), Punctuation::Type::Semicolon));
// 	toReturn = std::make_unique<Assignment>(new Identifier(Slice("x", "", 0, 0)),
// 	      std::make_unique<Literal>(new Identifier(Slice("1", "", 0, 0))));
// 	copy = toReturn.get();
// 	EXPECT_CALL(p7, parseRvalue)
// 	      .WillOnce(
// 	            ::testing::DoAll(::testing::Invoke([](std::vector<Token *>::iterator &it,
// 	                                                     [[maybe_unused]]
// 	                                                     CodeBlock &context) {
// 		            it++;
// 	            }),
// 	                  ::testing::Return(std::move(toReturn))));
// 	it = tokens.begin();

// 	statement = p7.parseStatement(it, context);

// 	EXPECT_EQ(it, tokens.end());
// 	expression = dynamic_cast<Expression *>(statement.get());
// 	EXPECT_NE(expression, nullptr);
// 	if (expression != nullptr) {
// 		EXPECT_EQ(expression->rval, toReturn);
// 	}
// 	EXPECT_NE(context.locals.find(new Identifier(Slice("x", "", 0, 0))),
// 	      context.locals.end());
// }

// TEST(test_parser, test_parseStatement_error) {
// 	std::vector<Token *> tokens;
// 	std::vector<Token *>::iterator it;
// 	Module module;
// 	CodeBlock context = CodeBlock(&module);
// 	Parser p;

// 	// Test 1: Redeclaration of variable of same type
// 	tokens = {};
// 	module = {};
// 	context = CodeBlock(&module);
// 	context.locals.insert({
// 	      new Identifier(Slice("x", "", 0, 0)), {Type::INT, false}
//     });
// 	tokens.push_back(new Primitive(Slice("int", "", 0, 0), Type::INT));
// 	tokens.push_back(new Identifier(Slice("x", "", 0, 0)));
// 	tokens.push_back(new Punctuation(Slice(";", "", 0, 0), Punctuation::Type::Semicolon));
// 	it = tokens.begin();

// 	EXPECT_EXIT(p.parseStatement(it, context), ::testing::ExitedWithCode(EXIT_FAILURE),
// 	      "Re-declaration of variable x");

// 	// Test 2: Redeclaration of variable with different type
// 	tokens = {};
// 	module = {};
// 	context = CodeBlock(&module);
// 	context.locals.insert({
// 	      new Identifier(Slice("x", "", 0, 0)), {Type::INT, false}
//     });
// 	tokens.push_back(new Primitive(Slice("float", "", 0, 0), Type::FLOAT));
// 	tokens.push_back(new Identifier(Slice("x", "", 0, 0)));
// 	tokens.push_back(new Punctuation(Slice(";", "", 0, 0), Punctuation::Type::Semicolon));
// 	it = tokens.begin();

// 	EXPECT_EXIT(p.parseStatement(it, context), ::testing::ExitedWithCode(EXIT_FAILURE),
// 	      "Re-declaration of variable x");

// 	// Test 3: Statement is not declaration or declaration assignment
// 	module = {};
// 	context = CodeBlock(&module);
// 	// 3a: Literal
// 	tokens = {};
// 	tokens.push_back(new Primitive(Slice("int", "", 0, 0), Type::INT));
// 	tokens.push_back(new Identifier(Slice("1", "", 0, 0)));
// 	tokens.push_back(new Punctuation(Slice(";", "", 0, 0), Punctuation::Type::Semicolon));
// 	it = tokens.begin();

// 	EXPECT_EXIT(p.parseStatement(it, context), ::testing::ExitedWithCode(EXIT_FAILURE),
// 	      "Unexpected expression following declaration");

// 	// 3b: Operator
// 	tokens = {};
// 	tokens.push_back(new Primitive(Slice("int", "", 0, 0), Type::INT));
// 	tokens.push_back(new Punctuation(Slice("+", "", 0, 0), Punctuation::Type::Plus));
// 	tokens.push_back(new Punctuation(Slice(";", "", 0, 0), Punctuation::Type::Semicolon));
// 	it = tokens.begin();

// 	EXPECT_EXIT(p.parseStatement(it, context), ::testing::ExitedWithCode(EXIT_FAILURE),
// 	      "Unexpected token following primitive");

// 	// 3c: Other punctuation
// 	tokens = {};
// 	tokens.push_back(new Primitive(Slice("int", "", 0, 0), Type::INT));
// 	tokens.push_back(new Punctuation(Slice(";", "", 0, 0), Punctuation::Type::Semicolon));
// 	it = tokens.begin();

// 	EXPECT_EXIT(p.parseStatement(it, context), ::testing::ExitedWithCode(EXIT_FAILURE),
// 	      "Unexpected token following primitive");

// 	// 3d: Non-assignment expression
// 	tokens = {};
// 	tokens.push_back(new Primitive(Slice("int", "", 0, 0), Type::INT));
// 	tokens.push_back(new Identifier(Slice("x", "", 0, 0)));
// 	tokens.push_back(new Punctuation(Slice("+", "", 0, 0), Punctuation::Type::Plus));
// 	tokens.push_back(new Identifier(Slice("1", "", 0, 0)));
// 	tokens.push_back(new Punctuation(Slice(";", "", 0, 0), Punctuation::Type::Semicolon));
// 	it = tokens.begin();

// 	EXPECT_EXIT(p.parseStatement(it, context), ::testing::ExitedWithCode(EXIT_FAILURE),
// 	      "Unexpected expression following declaration");

// 	// Test 4: Assignment LHS not variable
// 	tokens = {};
// 	tokens.push_back(new Keyword(Slice("return", "", 0, 0), Keyword::Type::RETURN));
// 	tokens.push_back(new Punctuation(Slice("=", "", 0, 0), Punctuation::Type::Equals));
// 	it = tokens.begin();

// 	EXPECT_EXIT(p.parseStatement(it, context), ::testing::ExitedWithCode(EXIT_FAILURE),
// 	      "LHS of assignment is not a variable");

// 	tokens = {};
// 	tokens.push_back(new Identifier(Slice("1", "", 0, 0)));
// 	tokens.push_back(new Punctuation(Slice("=", "", 0, 0), Punctuation::Type::Equals));
// 	tokens.push_back(new Identifier(Slice("1", "", 0, 0)));
// 	tokens.push_back(new Punctuation(Slice(";", "", 0, 0), Punctuation::Type::Semicolon));
// 	it = tokens.begin();

// 	// TODO change this to check if the LHS is not a literal - this is currently an e14
// 	// test here
// 	EXPECT_EXIT(p.parseStatement(it, context), ::testing::ExitedWithCode(EXIT_FAILURE),
// 	      "Undeclared variable 1");

// 	// Test 5: Missing ; after declaration
// 	tokens = {};
// 	tokens.push_back(new Primitive(Slice("int", "", 0, 0), Type::INT));
// 	tokens.push_back(new Identifier(Slice("x", "", 0, 0)));
// 	tokens.push_back(
// 	      new Punctuation(Slice("}", "", 0, 0), Punctuation::Type::CloseBrace));
// 	it = tokens.begin();

// 	EXPECT_EXIT(p.parseStatement(it, context), ::testing::ExitedWithCode(EXIT_FAILURE),
// 	      "Expected ';' after statement");

// 	// Test 6: Missing ; after declaration assignment
// 	tokens = {};
// 	tokens.push_back(new Primitive(Slice("int", "", 0, 0), Type::INT));
// 	tokens.push_back(new Identifier(Slice("x", "", 0, 0)));
// 	tokens.push_back(new Punctuation(Slice("=", "", 0, 0), Punctuation::Type::Equals));
// 	tokens.push_back(new Identifier(Slice("1", "", 0, 0)));
// 	tokens.push_back(
// 	      new Punctuation(Slice("}", "", 0, 0), Punctuation::Type::CloseBrace));
// 	it = tokens.begin();

// 	EXPECT_EXIT(p.parseStatement(it, context), ::testing::ExitedWithCode(EXIT_FAILURE),
// 	      "Expected ';' after statement");

// 	// Test 7: Missing ; after expression statement
// 	tokens = {};
// 	context.locals.insert({
// 	      new Identifier(Slice("x", "", 0, 0)), {Type::INT, false}
//     });
// 	tokens.push_back(new Identifier(Slice("x", "", 0, 0)));
// 	tokens.push_back(new Punctuation(Slice("=", "", 0, 0), Punctuation::Type::Equals));
// 	tokens.push_back(new Identifier(Slice("1", "", 0, 0)));
// 	tokens.push_back(
// 	      new Punctuation(Slice("}", "", 0, 0), Punctuation::Type::CloseBrace));
// 	it = tokens.begin();

// 	EXPECT_EXIT(p.parseStatement(it, context), ::testing::ExitedWithCode(EXIT_FAILURE),
// 	      "Expected ';' after statement");

// 	// Test 8: Missing ; after return statement
// 	tokens = {};
// 	tokens.push_back(new Keyword(Slice("return", "", 0, 0), Keyword::Type::RETURN));
// 	tokens.push_back(
// 	      new Punctuation(Slice("}", "", 0, 0), Punctuation::Type::CloseBrace));
// 	it = tokens.begin();

// 	EXPECT_EXIT(p.parseStatement(it, context), ::testing::ExitedWithCode(EXIT_FAILURE),
// 	      "Expected ';' after statement");
// }

// TEST(test_parser, test_e0) {
// 	Parser p;
// 	std::vector<Token *> tokens;
// 	const char *identifier;
// 	Slice s = {"", "", 0, 0};
// 	std::vector<Token *>::iterator it;
// 	Variable *var;
// 	Literal *val;

// 	// Test 1: All alphabetical identifier
// 	tokens = {};
// 	identifier = "xyz";
// 	s = Slice(identifier, "", 0, 0);
// 	tokens.push_back(new Identifier(s));
// 	it = tokens.begin();
// 	std::unique_ptr<rvalue> rval = p.e0(it);
// 	EXPECT_EQ(it, tokens.end());
// 	var = dynamic_cast<Variable *>(rval.get());
// 	EXPECT_NE(var, nullptr);
// 	if (var != nullptr) {
// 		EXPECT_EQ(var->variable->s, identifier);
// 	}

// 	// Test 2: All numerical identifier
// 	tokens = {};
// 	identifier = "123";
// 	s = Slice(identifier, "", 0, 0);
// 	tokens.push_back(new Identifier(s));
// 	it = tokens.begin();
// 	rval = p.e0(it);
// 	EXPECT_EQ(it, tokens.end());
// 	val = dynamic_cast<Literal *>(rval.get());
// 	EXPECT_NE(val, nullptr);
// 	if (val != nullptr) {
// 		EXPECT_EQ(val->value, 123);
// 	}

// 	// Test 3: Alphanumeric identifier
// 	tokens = {};
// 	identifier = "abc123";
// 	s = Slice(identifier, "", 0, 0);
// 	tokens.push_back(new Identifier(s));
// 	it = tokens.begin();
// 	rval = p.e0(it);
// 	EXPECT_EQ(it, tokens.end());
// 	var = dynamic_cast<Variable *>(rval.get());
// 	EXPECT_NE(var, nullptr);
// 	if (var != nullptr) {
// 		EXPECT_EQ(var->variable->s, identifier);
// 	}

// 	// Test 4: Not an identifier
// 	tokens = {};
// 	identifier = "int";
// 	s = Slice(identifier, "", 0, 0);
// 	tokens.push_back(new Primitive(s, Type::INT));
// 	it = tokens.begin();
// 	rval = p.e0(it);
// 	// Should not have advanced the iterator if it was not an identifier
// 	EXPECT_EQ(it, tokens.begin());
// 	EXPECT_EQ(rval, nullptr);
// }

// TEST(test_parser, test_e1) {
// 	class MockParser : public Parser {
// 	public:
// 		MOCK_METHOD(std::unique_ptr<rvalue>, e0, (std::vector<Token *>::iterator & it));
// 		MOCK_METHOD(std::unique_ptr<rvalue>, parseRvalue,
// 		      (std::vector<Token *>::iterator & it, CodeBlock &context));
// 	};

// 	std::vector<Token *> tokens;
// 	std::vector<Token *>::iterator it;
// 	Module module;
// 	CodeBlock context = CodeBlock(&module);
// 	FunctionCall *call;

// 	// Test 1: Parenthesized expressions call parseRvalue and return what they receive
// 	// from it
// 	MockParser p1;
// 	tokens = {};
// 	EXPECT_CALL(p1, e0).WillOnce(::testing::Return(nullptr));
// 	tokens.push_back(new Punctuation(Slice("(", "", 0, 0), Punctuation::Type::OpenParen));
// 	std::unique_ptr<rvalue> toReturn = std::make_unique<EmptyRvalue>();
// 	rvalue *copy = toReturn.get();
// 	EXPECT_CALL(p1, parseRvalue).WillOnce(::testing::Return(std::move(toReturn)));
// 	tokens.push_back(
// 	      new Punctuation(Slice(")", "", 0, 0), Punctuation::Type::CloseParen));

// 	it = tokens.begin();

// 	std::unique_ptr<rvalue> rval = p1.e1(it, context);

// 	EXPECT_EQ(it, tokens.end());
// 	EXPECT_EQ(rval.get(), copy);

// 	// Test 2: Function calls do not need arguments
// 	MockParser p2;
// 	tokens = {};
// 	std::unique_ptr<Variable> functionName
// 	      = std::make_unique<Variable>(new Identifier(Slice("function", "", 0, 0)));
// 	Variable *nameCopy = functionName.get();
// 	EXPECT_CALL(p2, e0).WillOnce(::testing::Return(std::move(functionName)));
// 	tokens.push_back(new Punctuation(Slice("(", "", 0, 0), Punctuation::Type::OpenParen));
// 	EXPECT_CALL(p2, parseRvalue).WillOnce(::testing::Return(nullptr));
// 	tokens.push_back(
// 	      new Punctuation(Slice(")", "", 0, 0), Punctuation::Type::CloseParen));
// 	it = tokens.begin();

// 	rval = p2.e1(it, context);

// 	EXPECT_EQ(it, tokens.end());
// 	call = dynamic_cast<FunctionCall *>(rval.get());
// 	EXPECT_NE(call, nullptr);
// 	if (call != nullptr) {
// 		EXPECT_EQ(call->name.get(), nameCopy);
// 		EXPECT_EQ(call->arguments.size(), 0);
// 	}

// 	// Test 3: Function calls may take an argument
// 	MockParser p3;
// 	tokens = {};
// 	functionName
// 	      = std::make_unique<Variable>(new Identifier(Slice("function", "", 0, 0)));
// 	EXPECT_CALL(p3, e0).WillOnce(::testing::Return(std::move(functionName)));
// 	tokens.push_back(new Punctuation(Slice("(", "", 0, 0), Punctuation::Type::OpenParen));
// 	std::unique_ptr<rvalue> argument = std::make_unique<EmptyRvalue>();
// 	rvalue *argCopy = argument.get();
// 	EXPECT_CALL(p3, parseRvalue).WillOnce(::testing::Return(std::move(argument)));
// 	tokens.push_back(
// 	      new Punctuation(Slice(")", "", 0, 0), Punctuation::Type::CloseParen));
// 	it = tokens.begin();

// 	rval = p3.e1(it, context);

// 	EXPECT_EQ(it, tokens.end());
// 	call = dynamic_cast<FunctionCall *>(rval.get());
// 	EXPECT_NE(call, nullptr);
// 	if (call != nullptr) {
// 		EXPECT_EQ(call->name, functionName);
// 		EXPECT_EQ(call->arguments.size(), 1);
// 		if (call->arguments.size() >= 1) {
// 			EXPECT_EQ(call->arguments[0].get(), argCopy);
// 		}
// 	}

// 	// Test 4: Function calls may take multiple arguments
// 	MockParser p4_1;
// 	tokens = {};
// 	functionName
// 	      = std::make_unique<Variable>(new Identifier(Slice("function", "", 0, 0)));
// 	copy = functionName.get();
// 	EXPECT_CALL(p4_1, e0).WillOnce(::testing::Return(std::move(functionName)));
// 	tokens.push_back(new Punctuation(Slice("(", "", 0, 0), Punctuation::Type::OpenParen));
// 	std::unique_ptr<rvalue> argument1 = std::make_unique<EmptyRvalue>();
// 	rvalue *argCopy1 = argument1.get();
// 	std::unique_ptr<rvalue> argument2 = std::make_unique<EmptyRvalue>();
// 	rvalue *argCopy2 = argument2.get();
// 	EXPECT_CALL(p4_1, parseRvalue)
// 	      .WillOnce(::testing::Return(std::move(argument1)))
// 	      .WillOnce(::testing::Return(std::move(argument2)));
// 	tokens.push_back(new Punctuation(Slice(",", "", 0, 0), Punctuation::Type::Comma));
// 	tokens.push_back(
// 	      new Punctuation(Slice(")", "", 0, 0), Punctuation::Type::CloseParen));
// 	it = tokens.begin();

// 	rval = p4_1.e1(it, context);

// 	EXPECT_EQ(it, tokens.end());
// 	call = dynamic_cast<FunctionCall *>(rval.get());
// 	EXPECT_NE(call, nullptr);
// 	if (call != nullptr) {
// 		EXPECT_EQ(call->name, functionName);
// 		EXPECT_EQ(call->arguments.size(), 2);
// 		if (call->arguments.size() >= 2) {
// 			EXPECT_EQ(call->arguments[0].get(), argCopy1);
// 			EXPECT_EQ(call->arguments[1].get(), argCopy2);
// 		}
// 	}

// 	MockParser p4_2;
// 	tokens = {};
// 	functionName
// 	      = std::make_unique<Variable>(new Identifier(Slice("function", "", 0, 0)));
// 	EXPECT_CALL(p4_2, e0).WillOnce(::testing::Return(std::move(functionName)));
// 	tokens.push_back(new Punctuation(Slice("(", "", 0, 0), Punctuation::Type::OpenParen));
// 	argument1 = std::make_unique<EmptyRvalue>();
// 	argument2 = std::make_unique<EmptyRvalue>();
// 	std::unique_ptr<rvalue> argument3 = std::make_unique<EmptyRvalue>();
// 	EXPECT_CALL(p4_2, parseRvalue)
// 	      .WillOnce(::testing::Return(std::move(argument1)))
// 	      .WillOnce(::testing::Return(std::move(argument2)))
// 	      .WillOnce(::testing::Return(std::move(argument3)));
// 	tokens.push_back(new Punctuation(Slice(",", "", 0, 0), Punctuation::Type::Comma));
// 	tokens.push_back(new Punctuation(Slice(",", "", 0, 0), Punctuation::Type::Comma));
// 	tokens.push_back(
// 	      new Punctuation(Slice(")", "", 0, 0), Punctuation::Type::CloseParen));
// 	it = tokens.begin();

// 	rval = p4_2.e1(it, context);

// 	EXPECT_EQ(it, tokens.end());
// 	call = dynamic_cast<FunctionCall *>(rval.get());
// 	if (call != nullptr) {
// 		EXPECT_EQ(call->name, functionName);
// 		EXPECT_EQ(call->arguments.size(), 3);
// 		if (call->arguments.size() >= 3) {
// 			EXPECT_EQ(call->arguments[0], argument1);
// 			EXPECT_EQ(call->arguments[1], argument2);
// 			EXPECT_EQ(call->arguments[2], argument3);
// 		}
// 	}
// }

// TEST(test_parser, test_e1_error) {
// 	std::vector<Token *> tokens;
// 	std::vector<Token *>::iterator it;
// 	Parser p;

// 	// Test 1: Using variable as function call
// 	tokens = {};
// 	Module module;
// 	CodeBlock context = CodeBlock(&module);
// 	context.locals.insert({
// 	      new Identifier(Slice("var", "", 0, 0)), {Type::INT, true}
//     });
// 	tokens.push_back(new Identifier(Slice("var", "", 0, 0)));
// 	tokens.push_back(new Punctuation(Slice("(", "", 0, 0), Punctuation::Type::OpenParen));
// 	tokens.push_back(
// 	      new Punctuation(Slice(")", "", 0, 0), Punctuation::Type::CloseParen));
// 	it = tokens.begin();

// 	EXPECT_EXIT(p.e1(it, context), ::testing::ExitedWithCode(EXIT_FAILURE),
// 	      "var is not callable");

// 	// Test 2: Using function name as variable
// 	module = {};
// 	context = CodeBlock(&module);
// 	Function *foo = new Function(&module);
// 	module.functions.insert({"foo", foo});
// 	tokens = {};
// 	tokens.push_back(new Identifier(Slice("foo", "", 0, 0)));
// 	tokens.push_back(new Punctuation(Slice(";", "", 0, 0), Punctuation::Type::Semicolon));
// 	it = tokens.begin();

// 	EXPECT_EXIT(p.e1(it, context), ::testing::ExitedWithCode(EXIT_FAILURE),
// 	      "foo is not a variable");

// 	// Test 3: Empty parenthesis for non-function call
// 	tokens = {};
// 	module = {};
// 	context = CodeBlock(&module);
// 	tokens.push_back(new Punctuation(Slice("(", "", 0, 0), Punctuation::Type::OpenParen));
// 	tokens.push_back(
// 	      new Punctuation(Slice(")", "", 0, 0), Punctuation::Type::CloseParen));
// 	it = tokens.begin();

// 	EXPECT_EXIT(p.e1(it, context), ::testing::ExitedWithCode(EXIT_FAILURE),
// 	      "Expected expression");

// 	// Test 4: Unmatched parenthesis in grouping
// 	tokens = {};
// 	module = {};
// 	context = CodeBlock(&module);
// 	tokens.push_back(new Punctuation(Slice("(", "", 0, 0), Punctuation::Type::OpenParen));
// 	tokens.push_back(new Identifier(Slice("var", "", 0, 0)));
// 	tokens.push_back(new Punctuation(Slice(";", "", 0, 0), Punctuation::Type::Semicolon));
// 	it = tokens.begin();

// 	EXPECT_EXIT(p.e1(it, context), ::testing::ExitedWithCode(EXIT_FAILURE),
// 	      "Expected ')'");

// 	// Test 5: Unmatched parenthesis in function call
// 	tokens = {};
// 	module = {};
// 	context = CodeBlock(&module);
// 	tokens.push_back(new Identifier(Slice("foo", "", 0, 0)));
// 	tokens.push_back(new Punctuation(Slice("(", "", 0, 0), Punctuation::Type::OpenParen));
// 	tokens.push_back(new Punctuation(Slice(";", "", 0, 0), Punctuation::Type::Semicolon));
// 	it = tokens.begin();

// 	EXPECT_EXIT(p.e1(it, context), ::testing::ExitedWithCode(EXIT_FAILURE),
// 	      "Expected ')'");
// }

// TEST(test_parser, test_e2) {
// 	std::vector<Token *> tokens;
// 	std::vector<Token *>::iterator it;
// 	Module module;
// 	CodeBlock context = CodeBlock(&module);

// 	class MockParser : public Parser {
// 	public:
// 		MOCK_METHOD(std::unique_ptr<rvalue>, e1,
// 		      (std::vector<Token *>::iterator & it, CodeBlock &context));
// 	};

// 	MockParser p;
// 	std::unique_ptr<rvalue> toReturn = std::make_unique<EmptyRvalue>();
// 	rvalue *copy = toReturn.get();
// 	EXPECT_CALL(p, e1).WillOnce(::testing::Return(std::move(toReturn)));
// 	it = tokens.begin();
// 	std::unique_ptr<rvalue> rval = p.e2(it, context);
// 	EXPECT_EQ(it, tokens.begin());
// 	EXPECT_EQ(rval.get(), copy);
// }

// TEST(test_parser, test_e3) {
// 	std::vector<Token *> tokens;
// 	std::vector<Token *>::iterator it;
// 	Module module;
// 	CodeBlock context = CodeBlock(&module);
// 	Multiplication *multiplication;
// 	Division *division;
// 	Modulo *modulo;

// 	class MockParser : public Parser {
// 	public:
// 		MOCK_METHOD(std::unique_ptr<rvalue>, e2,
// 		      (std::vector<Token *>::iterator & it, CodeBlock &context));
// 	};

// 	// Test 1: Multiplication
// 	MockParser p1;
// 	tokens = {};
// 	tokens.push_back(new Punctuation(Slice("*", "", 0, 0), Punctuation::Type::Times));
// 	std::unique_ptr<rvalue> operand1 = std::make_unique<EmptyRvalue>();
// 	rvalue *copy1 = operand1.get();
// 	std::unique_ptr<rvalue> operand2 = std::make_unique<EmptyRvalue>();
// 	rvalue *copy2 = operand2.get();
// 	EXPECT_CALL(p1, e2)
// 	      .WillOnce(::testing::Return(std::move(operand1)))
// 	      .WillOnce(::testing::Return(std::move(operand2)));
// 	it = tokens.begin();

// 	std::unique_ptr<rvalue> rval = p1.e3(it, context);

// 	EXPECT_EQ(it, tokens.end());
// 	multiplication = dynamic_cast<Multiplication *>(rval.get());
// 	EXPECT_NE(multiplication, nullptr);
// 	if (multiplication != nullptr) {
// 		EXPECT_EQ(multiplication->operand1.get(), copy1);
// 		EXPECT_EQ(multiplication->operand2.get(), copy2);
// 	}

// 	// Test 2: Subtraction
// 	MockParser p2;
// 	tokens = {};
// 	tokens.push_back(new Punctuation(Slice("/", "", 0, 0), Punctuation::Type::Divide));
// 	operand1 = std::make_unique<EmptyRvalue>();
// 	copy1 = operand1.get();
// 	operand2 = std::make_unique<EmptyRvalue>();
// 	copy2 = operand2.get();
// 	EXPECT_CALL(p2, e2)
// 	      .WillOnce(::testing::Return(std::move(operand1)))
// 	      .WillOnce(::testing::Return(std::move(operand2)));
// 	it = tokens.begin();

// 	rval = p2.e3(it, context);

// 	EXPECT_EQ(it, tokens.end());
// 	division = dynamic_cast<Division *>(rval.get());
// 	EXPECT_NE(division, nullptr);
// 	if (division != nullptr) {
// 		EXPECT_EQ(division->operand1.get(), copy1);
// 		EXPECT_EQ(division->operand2.get(), copy2);
// 	}

// 	// Test 3: Modulo
// 	MockParser p3;
// 	tokens = {};
// 	tokens.push_back(new Punctuation(Slice("%", "", 0, 0), Punctuation::Type::Mod));
// 	operand1 = std::make_unique<EmptyRvalue>();
// 	copy1 = operand1.get();
// 	operand2 = std::make_unique<EmptyRvalue>();
// 	copy2 = operand2.get();
// 	EXPECT_CALL(p3, e2)
// 	      .WillOnce(::testing::Return(std::move(operand1)))
// 	      .WillOnce(::testing::Return(std::move(operand2)));
// 	it = tokens.begin();

// 	rval = p3.e3(it, context);

// 	EXPECT_EQ(it, tokens.end());
// 	modulo = dynamic_cast<Modulo *>(rval.get());
// 	EXPECT_NE(modulo, nullptr);
// 	if (modulo != nullptr) {
// 		EXPECT_EQ(modulo->operand1.get(), copy1);
// 		EXPECT_EQ(modulo->operand2.get(), copy2);
// 	}

// 	// Test 4: Multiple Operators
// 	MockParser p4;
// 	tokens = {};
// 	tokens.push_back(new Punctuation(Slice("*", "", 0, 0), Punctuation::Type::Times));
// 	tokens.push_back(new Punctuation(Slice("/", "", 0, 0), Punctuation::Type::Divide));
// 	tokens.push_back(new Punctuation(Slice("/", "", 0, 0), Punctuation::Type::Divide));
// 	tokens.push_back(new Punctuation(Slice("%", "", 0, 0), Punctuation::Type::Mod));
// 	tokens.push_back(new Punctuation(Slice("*", "", 0, 0), Punctuation::Type::Times));
// 	tokens.push_back(new Punctuation(Slice("%", "", 0, 0), Punctuation::Type::Mod));
// 	tokens.push_back(new Punctuation(Slice("/", "", 0, 0), Punctuation::Type::Divide));
// 	tokens.push_back(new Punctuation(Slice(";", "", 0, 0), Punctuation::Type::Semicolon));
// 	operand1 = std::make_unique<EmptyRvalue>();
// 	copy1 = operand1.get();
// 	operand2 = std::make_unique<EmptyRvalue>();
// 	copy2 = operand2.get();
// 	std::unique_ptr<rvalue> operand3 = std::make_unique<EmptyRvalue>();
// 	rvalue *copy3 = operand3.get();
// 	std::unique_ptr<rvalue> operand4 = std::make_unique<EmptyRvalue>();
// 	rvalue *copy4 = operand4.get();
// 	std::unique_ptr<rvalue> operand5 = std::make_unique<EmptyRvalue>();
// 	rvalue *copy5 = operand5.get();
// 	std::unique_ptr<rvalue> operand6 = std::make_unique<EmptyRvalue>();
// 	rvalue *copy6 = operand6.get();
// 	std::unique_ptr<rvalue> operand7 = std::make_unique<EmptyRvalue>();
// 	rvalue *copy7 = operand7.get();
// 	std::unique_ptr<rvalue> operand8 = std::make_unique<EmptyRvalue>();
// 	rvalue *copy8 = operand8.get();
// 	EXPECT_CALL(p4, e2)
// 	      .WillOnce(::testing::Return(std::move(operand1)))
// 	      .WillOnce(::testing::Return(std::move(operand2)))
// 	      .WillOnce(::testing::Return(std::move(operand3)))
// 	      .WillOnce(::testing::Return(std::move(operand4)))
// 	      .WillOnce(::testing::Return(std::move(operand5)))
// 	      .WillOnce(::testing::Return(std::move(operand6)))
// 	      .WillOnce(::testing::Return(std::move(operand7)))
// 	      .WillOnce(::testing::Return(std::move(operand8)));
// 	it = tokens.begin();

// 	rval = p4.e4(it, context);

// 	EXPECT_EQ(it + 1, tokens.end());
// 	division = dynamic_cast<Division *>(rval.get());
// 	EXPECT_NE(division, nullptr);
// 	if (division != nullptr) {
// 		EXPECT_EQ(division->operand2.get(), copy8);
// 		modulo = dynamic_cast<Modulo *>(division->operand1.get());
// 		EXPECT_NE(modulo, nullptr);
// 		if (modulo != nullptr) {
// 			EXPECT_EQ(modulo->operand2.get(), copy7);
// 			multiplication = dynamic_cast<Multiplication *>(modulo->operand1.get());
// 			EXPECT_NE(multiplication, nullptr);
// 			if (multiplication != nullptr) {
// 				EXPECT_EQ(multiplication->operand2.get(), copy6);
// 				modulo = dynamic_cast<Modulo *>(multiplication->operand1.get());
// 				EXPECT_NE(modulo, nullptr);
// 				if (modulo != nullptr) {
// 					EXPECT_EQ(modulo->operand2.get(), copy5);
// 					division = dynamic_cast<Division *>(modulo->operand1.get());
// 					EXPECT_NE(division, nullptr);
// 					if (division != nullptr) {
// 						EXPECT_EQ(division->operand2.get(), copy4);
// 						division = dynamic_cast<Division *>(division->operand1.get());
// 						EXPECT_NE(division, nullptr);
// 						if (division != nullptr) {
// 							EXPECT_EQ(division->operand2.get(), copy3);
// 							multiplication = dynamic_cast<Multiplication *>(
// 							      division->operand1.get());
// 							EXPECT_NE(multiplication, nullptr);
// 							if (multiplication != nullptr) {
// 								EXPECT_EQ(multiplication->operand2.get(), copy2);
// 								EXPECT_EQ(multiplication->operand1.get(), copy1);
// 							}
// 						}
// 					}
// 				}
// 			}
// 		}
// 	}

// 	// Test 5: Other punctuation
// 	MockParser p5;
// 	tokens = {};
// 	tokens.push_back(new Punctuation(Slice(";", "", 0, 0), Punctuation::Type::Semicolon));
// 	std::unique_ptr<rvalue> toReturn = std::make_unique<EmptyRvalue>();
// 	EXPECT_CALL(p5, e2).WillOnce(::testing::Return(std::move(toReturn)));
// 	it = tokens.begin();

// 	rval = p5.e3(it, context);

// 	EXPECT_EQ(it, tokens.begin());
// 	EXPECT_EQ(rval, toReturn);

// 	// Test 6: Non-punctuation
// 	MockParser p6;
// 	tokens = {};
// 	tokens.push_back(new Identifier(Slice("x", "", 0, 0)));
// 	toReturn = std::make_unique<EmptyRvalue>();
// 	rvalue *copy = toReturn.get();
// 	EXPECT_CALL(p6, e2).WillOnce(::testing::Return(std::move(toReturn)));
// 	it = tokens.begin();

// 	rval = p6.e3(it, context);

// 	EXPECT_EQ(it, tokens.begin());
// 	EXPECT_EQ(rval.get(), copy);
// }

// TEST(test_parser, test_e4) {
// 	std::vector<Token *> tokens;
// 	std::vector<Token *>::iterator it;
// 	Module module;
// 	CodeBlock context = CodeBlock(&module);
// 	Addition *addition;
// 	Subtraction *subtraction;

// 	class MockParser : public Parser {
// 	public:
// 		MOCK_METHOD(std::unique_ptr<rvalue>, e3,
// 		      (std::vector<Token *>::iterator & it, CodeBlock &context));
// 	};

// 	// Test 1: Addition
// 	MockParser p1;
// 	tokens = {};
// 	tokens.push_back(new Punctuation(Slice("+", "", 0, 0), Punctuation::Type::Plus));
// 	std::unique_ptr<rvalue> operand1 = std::make_unique<EmptyRvalue>();
// 	std::unique_ptr<rvalue> operand2 = std::make_unique<EmptyRvalue>();
// 	EXPECT_CALL(p1, e3)
// 	      .WillOnce(::testing::Return(std::move(operand1)))
// 	      .WillOnce(::testing::Return(std::move(operand2)));
// 	it = tokens.begin();

// 	std::unique_ptr<rvalue> rval = p1.e4(it, context);

// 	EXPECT_EQ(it, tokens.end());
// 	addition = dynamic_cast<Addition *>(rval.get());
// 	EXPECT_NE(addition, nullptr);
// 	if (addition != nullptr) {
// 		EXPECT_EQ(addition->operand1, operand1);
// 		EXPECT_EQ(addition->operand2, operand2);
// 	}

// 	// Test 2: Subtraction
// 	MockParser p2;
// 	tokens = {};
// 	tokens.push_back(new Punctuation(Slice("-", "", 0, 0), Punctuation::Type::Minus));
// 	operand1 = std::make_unique<EmptyRvalue>();
// 	operand2 = std::make_unique<EmptyRvalue>();
// 	EXPECT_CALL(p2, e3)
// 	      .WillOnce(::testing::Return(std::move(operand1)))
// 	      .WillOnce(::testing::Return(std::move(operand2)));
// 	it = tokens.begin();

// 	rval = p2.e4(it, context);

// 	EXPECT_EQ(it, tokens.end());
// 	subtraction = dynamic_cast<Subtraction *>(rval.get());
// 	EXPECT_NE(subtraction, nullptr);
// 	if (subtraction != nullptr) {
// 		EXPECT_EQ(subtraction->operand1, operand1);
// 		EXPECT_EQ(subtraction->operand2, operand2);
// 	}

// 	// Test 3: Multiple Operators
// 	MockParser p3;
// 	tokens = {};
// 	tokens.push_back(new Punctuation(Slice("+", "", 0, 0), Punctuation::Type::Plus));
// 	tokens.push_back(new Punctuation(Slice("-", "", 0, 0), Punctuation::Type::Minus));
// 	tokens.push_back(new Punctuation(Slice("-", "", 0, 0), Punctuation::Type::Minus));
// 	tokens.push_back(new Punctuation(Slice("+", "", 0, 0), Punctuation::Type::Plus));
// 	tokens.push_back(new Punctuation(Slice("+", "", 0, 0), Punctuation::Type::Plus));
// 	tokens.push_back(new Punctuation(Slice("-", "", 0, 0), Punctuation::Type::Minus));
// 	tokens.push_back(new Punctuation(Slice("+", "", 0, 0), Punctuation::Type::Plus));
// 	tokens.push_back(new Punctuation(Slice(";", "", 0, 0), Punctuation::Type::Semicolon));
// 	operand1 = std::make_unique<EmptyRvalue>();
// 	rvalue *copy1 = operand1.get();
// 	operand2 = std::make_unique<EmptyRvalue>();
// 	rvalue *copy2 = operand2.get();
// 	std::unique_ptr<rvalue> operand3 = std::make_unique<EmptyRvalue>();
// 	rvalue *copy3 = operand3.get();
// 	std::unique_ptr<rvalue> operand4 = std::make_unique<EmptyRvalue>();
// 	rvalue *copy4 = operand4.get();
// 	std::unique_ptr<rvalue> operand5 = std::make_unique<EmptyRvalue>();
// 	rvalue *copy5 = operand5.get();
// 	std::unique_ptr<rvalue> operand6 = std::make_unique<EmptyRvalue>();
// 	rvalue *copy6 = operand6.get();
// 	std::unique_ptr<rvalue> operand7 = std::make_unique<EmptyRvalue>();
// 	rvalue *copy7 = operand7.get();
// 	std::unique_ptr<rvalue> operand8 = std::make_unique<EmptyRvalue>();
// 	rvalue *copy8 = operand8.get();
// 	EXPECT_CALL(p3, e3)
// 	      .WillOnce(::testing::Return(std::move(std::move(operand1))))
// 	      .WillOnce(::testing::Return(std::move(std::move(operand2))))
// 	      .WillOnce(::testing::Return(std::move(std::move(operand3))))
// 	      .WillOnce(::testing::Return(std::move(std::move(operand4))))
// 	      .WillOnce(::testing::Return(std::move(std::move(operand5))))
// 	      .WillOnce(::testing::Return(std::move(std::move(operand6))))
// 	      .WillOnce(::testing::Return(std::move(std::move(operand7))))
// 	      .WillOnce(::testing::Return(std::move(std::move(operand8))));
// 	it = tokens.begin();

// 	rval = p3.e4(it, context);

// 	EXPECT_EQ(it + 1, tokens.end());
// 	addition = dynamic_cast<Addition *>(rval.get());
// 	EXPECT_NE(addition, nullptr);
// 	if (addition != nullptr) {
// 		EXPECT_EQ(addition->operand2.get(), copy8);
// 		subtraction = dynamic_cast<Subtraction *>(addition->operand1.get());
// 		EXPECT_NE(subtraction, nullptr);
// 		if (subtraction != nullptr) {
// 			EXPECT_EQ(subtraction->operand2.get(), copy7);
// 			addition = dynamic_cast<Addition *>(subtraction->operand1.get());
// 			EXPECT_NE(addition, nullptr);
// 			if (addition != nullptr) {
// 				EXPECT_EQ(addition->operand2.get(), copy6);
// 				addition = dynamic_cast<Addition *>(addition->operand1.get());
// 				EXPECT_NE(addition, nullptr);
// 				if (addition != nullptr) {
// 					EXPECT_EQ(addition->operand2.get(), copy5);
// 					subtraction = dynamic_cast<Subtraction *>(addition->operand1.get());
// 					EXPECT_NE(subtraction, nullptr);
// 					if (subtraction != nullptr) {
// 						EXPECT_EQ(subtraction->operand2.get(), copy4);
// 						subtraction
// 						      = dynamic_cast<Subtraction *>(subtraction->operand1.get());
// 						EXPECT_NE(subtraction, nullptr);
// 						if (subtraction != nullptr) {
// 							EXPECT_EQ(subtraction->operand2.get(), copy3);
// 							addition
// 							      = dynamic_cast<Addition *>(subtraction->operand1.get());
// 							EXPECT_NE(addition, nullptr);
// 							if (addition != nullptr) {
// 								EXPECT_EQ(addition->operand2.get(), copy2);
// 								EXPECT_EQ(addition->operand1.get(), copy1);
// 							}
// 						}
// 					}
// 				}
// 			}
// 		}
// 	}

// 	// Test 4: Other punctuation
// 	MockParser p4;
// 	tokens = {};
// 	tokens.push_back(new Punctuation(Slice(";", "", 0, 0), Punctuation::Type::Semicolon));
// 	std::unique_ptr<rvalue> toReturn = std::make_unique<EmptyRvalue>();
// 	rvalue *copy = toReturn.get();
// 	EXPECT_CALL(p4, e3).WillOnce(::testing::Return(std::move(toReturn)));
// 	it = tokens.begin();

// 	rval = p4.e4(it, context);

// 	EXPECT_EQ(it, tokens.begin());
// 	EXPECT_EQ(rval.get(), copy);

// 	// Test 5: Non-punctuation
// 	MockParser p5;
// 	tokens = {};
// 	tokens.push_back(new Identifier(Slice("x", "", 0, 0)));
// 	toReturn = std::make_unique<EmptyRvalue>();
// 	copy = toReturn.get();
// 	EXPECT_CALL(p5, e3).WillOnce(::testing::Return(std::move(toReturn)));
// 	it = tokens.begin();

// 	rval = p5.e4(it, context);

// 	EXPECT_EQ(it, tokens.begin());
// 	EXPECT_EQ(rval.get(), copy);
// }

// TEST(test_parser, test_e5) {
// 	std::vector<Token *> tokens;
// 	std::vector<Token *>::iterator it;
// 	Module module;
// 	CodeBlock context = CodeBlock(&module);

// 	class MockParser : public Parser {
// 	public:
// 		MOCK_METHOD(std::unique_ptr<rvalue>, e4,
// 		      (std::vector<Token *>::iterator & it, CodeBlock &context));
// 	};

// 	MockParser p;
// 	std::unique_ptr<rvalue> toReturn = std::make_unique<EmptyRvalue>();
// 	rvalue *copy = toReturn.get();
// 	EXPECT_CALL(p, e4).WillOnce(::testing::Return(std::move(toReturn)));
// 	it = tokens.begin();
// 	std::unique_ptr<rvalue> rval = p.e5(it, context);
// 	EXPECT_EQ(it, tokens.begin());
// 	EXPECT_EQ(rval.get(), copy);
// }

// TEST(test_parser, test_e6) {
// 	std::vector<Token *> tokens;
// 	std::vector<Token *>::iterator it;
// 	Module module;
// 	CodeBlock context = CodeBlock(&module);

// 	class MockParser : public Parser {
// 	public:
// 		MOCK_METHOD(std::unique_ptr<rvalue>, e5,
// 		      (std::vector<Token *>::iterator & it, CodeBlock &context));
// 	};

// 	MockParser p;
// 	std::unique_ptr<rvalue> toReturn = std::make_unique<EmptyRvalue>();
// 	rvalue *copy = toReturn.get();
// 	EXPECT_CALL(p, e5).WillOnce(::testing::Return(std::move(toReturn)));
// 	it = tokens.begin();
// 	std::unique_ptr<rvalue> rval = p.e6(it, context);
// 	EXPECT_EQ(it, tokens.begin());
// 	EXPECT_EQ(rval.get(), copy);
// }

// TEST(test_parser, test_e7) {
// 	std::vector<Token *> tokens;
// 	std::vector<Token *>::iterator it;
// 	Module module;
// 	CodeBlock context = CodeBlock(&module);

// 	class MockParser : public Parser {
// 	public:
// 		MOCK_METHOD(std::unique_ptr<rvalue>, e6,
// 		      (std::vector<Token *>::iterator & it, CodeBlock &context));
// 	};

// 	MockParser p;
// 	std::unique_ptr<rvalue> toReturn = std::make_unique<EmptyRvalue>();
// 	rvalue *copy = toReturn.get();
// 	EXPECT_CALL(p, e6).WillOnce(::testing::Return(std::move(toReturn)));
// 	it = tokens.begin();
// 	std::unique_ptr<rvalue> rval = p.e7(it, context);
// 	EXPECT_EQ(it, tokens.begin());
// 	EXPECT_EQ(rval.get(), copy);
// }

// TEST(test_parser, test_e8) {
// 	std::vector<Token *> tokens;
// 	std::vector<Token *>::iterator it;
// 	Module module;
// 	CodeBlock context = CodeBlock(&module);

// 	class MockParser : public Parser {
// 	public:
// 		MOCK_METHOD(std::unique_ptr<rvalue>, e7,
// 		      (std::vector<Token *>::iterator & it, CodeBlock &context));
// 	};

// 	MockParser p;
// 	std::unique_ptr<rvalue> toReturn = std::make_unique<EmptyRvalue>();
// 	rvalue *copy = toReturn.get();
// 	EXPECT_CALL(p, e7).WillOnce(::testing::Return(std::move(toReturn)));
// 	it = tokens.begin();
// 	std::unique_ptr<rvalue> rval = p.e8(it, context);
// 	EXPECT_EQ(it, tokens.begin());
// 	EXPECT_EQ(rval.get(), copy);
// }

// TEST(test_parser, test_e9) {
// 	std::vector<Token *> tokens;
// 	std::vector<Token *>::iterator it;
// 	Module module;
// 	CodeBlock context = CodeBlock(&module);

// 	class MockParser : public Parser {
// 	public:
// 		MOCK_METHOD(std::unique_ptr<rvalue>, e8,
// 		      (std::vector<Token *>::iterator & it, CodeBlock &context));
// 	};

// 	MockParser p;
// 	std::unique_ptr<rvalue> toReturn = std::make_unique<EmptyRvalue>();
// 	rvalue *copy = toReturn.get();
// 	EXPECT_CALL(p, e8).WillOnce(::testing::Return(std::move(toReturn)));
// 	it = tokens.begin();
// 	std::unique_ptr<rvalue> rval = p.e9(it, context);
// 	EXPECT_EQ(it, tokens.begin());
// 	EXPECT_EQ(rval.get(), copy);
// }

// TEST(test_parser, test_e10) {
// 	std::vector<Token *> tokens;
// 	std::vector<Token *>::iterator it;
// 	Module module;
// 	CodeBlock context = CodeBlock(&module);

// 	class MockParser : public Parser {
// 	public:
// 		MOCK_METHOD(std::unique_ptr<rvalue>, e9,
// 		      (std::vector<Token *>::iterator & it, CodeBlock &context));
// 	};

// 	MockParser p;
// 	std::unique_ptr<rvalue> toReturn = std::make_unique<EmptyRvalue>();
// 	rvalue *copy = toReturn.get();
// 	EXPECT_CALL(p, e9).WillOnce(::testing::Return(std::move(toReturn)));
// 	it = tokens.begin();
// 	std::unique_ptr<rvalue> rval = p.e10(it, context);
// 	EXPECT_EQ(it, tokens.begin());
// 	EXPECT_EQ(rval.get(), copy);
// }

// TEST(test_parser, test_e11) {
// 	std::vector<Token *> tokens;
// 	std::vector<Token *>::iterator it;
// 	Module module;
// 	CodeBlock context = CodeBlock(&module);

// 	class MockParser : public Parser {
// 	public:
// 		MOCK_METHOD(std::unique_ptr<rvalue>, e10,
// 		      (std::vector<Token *>::iterator & it, CodeBlock &context));
// 	};

// 	MockParser p;
// 	std::unique_ptr<rvalue> toReturn = std::make_unique<EmptyRvalue>();
// 	rvalue *copy = toReturn.get();
// 	EXPECT_CALL(p, e10).WillOnce(::testing::Return(std::move(toReturn)));
// 	it = tokens.begin();
// 	std::unique_ptr<rvalue> rval = p.e11(it, context);
// 	EXPECT_EQ(it, tokens.begin());
// 	EXPECT_EQ(rval.get(), copy);
// }

// TEST(test_parser, test_e12) {
// 	std::vector<Token *> tokens;
// 	std::vector<Token *>::iterator it;
// 	Module module;
// 	CodeBlock context = CodeBlock(&module);

// 	class MockParser : public Parser {
// 	public:
// 		MOCK_METHOD(std::unique_ptr<rvalue>, e11,
// 		      (std::vector<Token *>::iterator & it, CodeBlock &context));
// 	};

// 	MockParser p;
// 	std::unique_ptr<rvalue> toReturn = std::make_unique<EmptyRvalue>();
// 	rvalue *copy = toReturn.get();
// 	EXPECT_CALL(p, e11).WillOnce(::testing::Return(std::move(toReturn)));
// 	it = tokens.begin();
// 	std::unique_ptr<rvalue> rval = p.e12(it, context);
// 	EXPECT_EQ(it, tokens.begin());
// 	EXPECT_EQ(rval.get(), copy);
// }

// TEST(test_parser, test_e13) {
// 	std::vector<Token *> tokens;
// 	std::vector<Token *>::iterator it;
// 	Module module;
// 	CodeBlock context = CodeBlock(&module);

// 	class MockParser : public Parser {
// 	public:
// 		MOCK_METHOD(std::unique_ptr<rvalue>, e12,
// 		      (std::vector<Token *>::iterator & it, CodeBlock &context));
// 	};

// 	MockParser p;
// 	std::unique_ptr<rvalue> toReturn = std::make_unique<EmptyRvalue>();
// 	rvalue *copy = toReturn.get();
// 	EXPECT_CALL(p, e12).WillOnce(::testing::Return(std::move(toReturn)));
// 	it = tokens.begin();
// 	std::unique_ptr<rvalue> rval = p.e13(it, context);
// 	EXPECT_EQ(it, tokens.begin());
// 	EXPECT_EQ(rval.get(), copy);
// }

// TEST(test_parser, test_e14) {
// 	class MockParser : public Parser {
// 	public:
// 		MOCK_METHOD(std::unique_ptr<rvalue>, e13,
// 		      (std::vector<Token *>::iterator & it, CodeBlock &context));
// 		MOCK_METHOD(std::unique_ptr<rvalue>, e14,
// 		      (std::vector<Token *>::iterator & it, CodeBlock &context));
// 	};

// 	std::vector<Token *> tokens;
// 	std::vector<Token *>::iterator it;
// 	Module module;
// 	CodeBlock context = CodeBlock(&module);
// 	Assignment *assignment;

// 	// Test 1: Assignment
// 	MockParser p1;
// 	tokens = {};
// 	tokens.push_back(new Identifier(Slice("x", "", 0, 0)));
// 	tokens.push_back(new Punctuation(Slice("=", "", 0, 0), Punctuation::Type::Equals));
// 	std::unique_ptr<rvalue> toReturn = std::make_unique<EmptyRvalue>();
// 	rvalue *copy = toReturn.get();
// 	context.locals.insert({
// 	      new Identifier(Slice("x", "", 0, 0)), {Type::INT, true}
//     });
// 	EXPECT_CALL(p1, e14)
// 	      // Test multiple assignment for free thanks to recursion here
// 	      .WillOnce(
// 	            ::testing::Invoke([&p1](std::vector<Token *>::iterator &it,
// 	                                    CodeBlock &context) -> std::unique_ptr<rvalue> {
// 		            return p1.Parser::e14(it, context);
// 	            }))
// 	      .WillOnce(::testing::Return(std::move(toReturn)));
// 	it = tokens.begin();

// 	std::unique_ptr<rvalue> rval = p1.e14(it, context);

// 	EXPECT_EQ(it, tokens.end());
// 	assignment = dynamic_cast<Assignment *>(rval.get());
// 	EXPECT_NE(assignment, nullptr);
// 	if (assignment != nullptr) {
// 		EXPECT_EQ(assignment->variable->s, "x");
// 		EXPECT_EQ(assignment->expression.get(), copy);
// 	}

// 	// Test 2: No Assignment with leading identifier
// 	MockParser p2;
// 	tokens = {};
// 	tokens.push_back(new Identifier(Slice("x", "", 0, 0)));
// 	tokens.push_back(new Punctuation(Slice("+", "", 0, 0), Punctuation::Type::Plus));
// 	toReturn = std::make_unique<EmptyRvalue>();
// 	copy = toReturn.get();
// 	EXPECT_CALL(p2, e14).WillOnce(
// 	      ::testing::Invoke([&p2](std::vector<Token *>::iterator &it,
// 	                              CodeBlock &context) -> std::unique_ptr<rvalue> {
// 		      return p2.Parser::e14(it, context);
// 	      }));
// 	EXPECT_CALL(p2, e13).WillOnce(::testing::Return(std::move(toReturn)));
// 	it = tokens.begin();

// 	rval = p2.e14(it, context);

// 	EXPECT_EQ(it, tokens.begin());
// 	EXPECT_EQ(rval.get(), copy);

// 	// Test 3: No Assignment without leading identifier
// 	MockParser p3;
// 	tokens = {};
// 	tokens.push_back(new Punctuation(Slice("+", "", 0, 0), Punctuation::Type::Plus));
// 	toReturn = std::make_unique<EmptyRvalue>();
// 	EXPECT_CALL(p3, e14).WillOnce(
// 	      ::testing::Invoke([&p3](std::vector<Token *>::iterator &it,
// 	                              CodeBlock &context) -> std::unique_ptr<rvalue> {
// 		      return p3.Parser::e14(it, context);
// 	      }));
// 	EXPECT_CALL(p3, e13).WillOnce(::testing::Return(std::move(toReturn)));
// 	it = tokens.begin();

// 	rval = p3.e14(it, context);

// 	EXPECT_EQ(it, tokens.begin());
// 	EXPECT_EQ(rval, toReturn);

// 	// Test 4: Assignment to undeclared variable
// 	Parser p4;
// 	tokens = {};
// 	module = {};
// 	context = CodeBlock(&module);
// 	tokens.push_back(new Identifier(Slice("x", "", 0, 0)));
// 	tokens.push_back(new Punctuation(Slice("=", "", 0, 0), Punctuation::Type::Equals));
// 	it = tokens.begin();

// 	EXPECT_EXIT(p4.e14(it, context), ::testing::ExitedWithCode(EXIT_FAILURE),
// 	      "Undeclared variable x");
// }

// TEST(test_parser, test_e15) {
// 	std::vector<Token *> tokens;
// 	std::vector<Token *>::iterator it;
// 	Module module;
// 	CodeBlock context = CodeBlock(&module);

// 	class MockParser : public Parser {
// 	public:
// 		MOCK_METHOD(std::unique_ptr<rvalue>, e14,
// 		      (std::vector<Token *>::iterator & it, CodeBlock &context));
// 	};

// 	MockParser p;
// 	std::unique_ptr<rvalue> toReturn = std::make_unique<EmptyRvalue>();
// 	rvalue *copy = toReturn.get();
// 	EXPECT_CALL(p, e14).WillOnce(::testing::Return(std::move(toReturn)));
// 	it = tokens.begin();
// 	std::unique_ptr<rvalue> rval = p.e15(it, context);
// 	EXPECT_EQ(it, tokens.begin());
// 	EXPECT_EQ(rval.get(), copy);
// }

// TEST(test_parser, test_parseRvalue) {
// 	std::vector<Token *> tokens;
// 	std::vector<Token *>::iterator it;
// 	Module module;
// 	CodeBlock context = CodeBlock(&module);

// 	class MockParser : public Parser {
// 	public:
// 		MOCK_METHOD(std::unique_ptr<rvalue>, e15,
// 		      (std::vector<Token *>::iterator & it, CodeBlock &context));
// 	};

// 	MockParser p;
// 	std::unique_ptr<rvalue> toReturn = std::make_unique<EmptyRvalue>();
// 	rvalue *copy = toReturn.get();
// 	EXPECT_CALL(p, e15).WillOnce(::testing::Return(std::move(toReturn)));
// 	it = tokens.begin();
// 	std::unique_ptr<rvalue> rval = p.parseRvalue(it, context);
// 	EXPECT_EQ(it, tokens.begin());
// 	EXPECT_EQ(rval.get(), copy);
// }
