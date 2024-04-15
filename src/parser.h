#ifndef PARSE_H
#define PARSE_H

#include "ast.h"
#include "errorhandler.h"
#include "tokens.h"
#include <unordered_map>

#include <cstdint>
#include <fcntl.h>
#include <memory>
#include <vector>

#ifdef DEBUG_TEST_MODE
#	define mockable virtual
#else
#	define mockable
#endif

class Parser {
#ifdef DEBUG_TEST_MODE
public:
#endif
	ErrorHandler errors;
public:
	Parser();

	/**
	 * @brief Converts tokenized source code into an abstract syntax tree representing
	 * the current module
	 *
	 * @param tokens the tokens to parse
	 * @return the parsed Module
	 */
	mockable AST::Module parseModule(std::vector<std::unique_ptr<Token>> &tokens);

	/**
	 * @brief Parses entire rvalues, taking operator precedence into consideration
	 *
	 * @param it a reference to the iterator of Tokens to use. Expected to point to the
	 * first token of the expression when this function is called. When this function
	 * returns it will point to the token IMMEDIATELY AFTER the rvalue
	 * @param context the AST::CodeBlock in which the current expression occurs
	 * @return the parsed rvalue
	 */
	mockable std::unique_ptr<AST::rvalue> parseRvalue(
	      std::vector<std::unique_ptr<Token>>::iterator &it, AST::CodeBlock &context);
#ifndef DEBUG_TEST_MODE
private:
#endif
	/**
	 * @brief Converts tokens to the AST representation of all functions in the
	 * current module
	 *
	 * @param tokens the stream of tokens representing the current module
	 * @param module the Module to place the parsed functions inside of
	 */
	mockable void parseFunctions(std::vector<std::unique_ptr<Token>> &tokens,
	      AST::Module &module);
	/**
	 * @brief Converts tokens to the AST representation of a function
	 *
	 * @param it a reference to the iterator of Tokens to use. Expected to point to the
	 * canyon function type when this function is called. By the end of the function it
	 * will point to the token IMMEDIATELY AFTER the closing curly brace
	 * @param module the Module to place the parsed function inside of
	 */
	mockable void parseFunction(std::vector<std::unique_ptr<Token>>::iterator &it,
	      AST::Module &module);
	/**
	 * @brief Converts tokens to the AST representation of a function parameter list
	 *
	 * @param it a reference to the iterator of Tokens to use. Expected to point to the
	 * token of the opening parenthesis when this function is called. When this function
	 * returns, it will point to the token IMMEDIATELY AFTER the closing parenthesis
	 * @param function the Function to determine its parameters
	 */
	mockable void parseParameters(std::vector<std::unique_ptr<Token>>::iterator &it,
	      AST::Function &function);
	/**
	 * @brief Parses tokens into a code block
	 *
	 * @param it a reference to the iterator of Tokens to use. Expected to point to the
	 * token of the opening curly brace when this function is called. When this function
	 * returns it will point to the token IMMEDIATELY AFTER the closing curly brace
	 * @param context the "fresh" CodeBlock to fill its contents with
	 */
	mockable void parseBlock(std::vector<std::unique_ptr<Token>>::iterator &it,
	      AST::CodeBlock &context);
	/**
	 * @brief Parses tokens into a statement
	 *
	 * @param it a reference to the iterator of Tokens to use. Expected to point to the
	 * first token of the statement when this function is called. When this function
	 * returns it will point to the token IMMEDIIATELY AFTER the statement
	 * @param context the most specific context that the statement is a part of
	 * @return the parsed Statement
	 */
	mockable std::unique_ptr<AST::Statement> parseStatement(
	      std::vector<std::unique_ptr<Token>>::iterator &it, AST::CodeBlock &context);
	/**
	 * @brief Deeper than any operators - identifiers and literals
	 *
	 * @param it a reference to the iterator of Tokens to use. Expected to point to the
	 * first token of the expression when this function is called. When this function
	 * returns it will point to the token IMMEDIATELY AFTER the rvalue
	 * @return the parsed rvalue
	 */
	mockable std::unique_ptr<AST::rvalue> e0(
	      std::vector<std::unique_ptr<Token>>::iterator &it);
	/**
	 * @brief () - grouping for precedence and function calls
	 *
	 * @param it a reference to the iterator of Tokens to use. Expected to point to the
	 * first token of the expression when this function is called. When this function
	 * returns it will point to the token IMMEDIATELY AFTER the rvalue
	 * @param context the AST::CodeBlock in which the current expression occurs
	 * @return the parsed rvalue
	 */
	mockable std::unique_ptr<AST::rvalue> e1(
	      std::vector<std::unique_ptr<Token>>::iterator &it, AST::CodeBlock &context);
	mockable std::unique_ptr<AST::rvalue> e2(
	      std::vector<std::unique_ptr<Token>>::iterator &it, AST::CodeBlock &context);
	/**
	 * @brief Multiplication, division, modulo operators * / % (LTR)
	 *
	 * @param it a reference to the iterator of Tokens to use. Expected to point to the
	 * first token of the expression when this function is called. When this function
	 * returns it will point to the token IMMEDIATELY AFTER the rvalue
	 * @param context the AST::CodeBlock in which the current expression occurs
	 * @return the parsed rvalue
	 */
	mockable std::unique_ptr<AST::rvalue> e3(
	      std::vector<std::unique_ptr<Token>>::iterator &it, AST::CodeBlock &context);
	/**
	 * @brief Addition and subtraction operators + - (LTR)
	 *
	 * @param it a reference to the iterator of Tokens to use. Expected to point to the
	 * first token of the expression when this function is called. When this function
	 * returns it will point to the token IMMEDIATELY AFTER the rvalue
	 * @param context the AST::CodeBlock in which the current expression occurs
	 * @return the parsed rvalue
	 */
	mockable std::unique_ptr<AST::rvalue> e4(
	      std::vector<std::unique_ptr<Token>>::iterator &it, AST::CodeBlock &context);
	mockable std::unique_ptr<AST::rvalue> e5(
	      std::vector<std::unique_ptr<Token>>::iterator &it, AST::CodeBlock &context);
	mockable std::unique_ptr<AST::rvalue> e6(
	      std::vector<std::unique_ptr<Token>>::iterator &it, AST::CodeBlock &context);
	mockable std::unique_ptr<AST::rvalue> e7(
	      std::vector<std::unique_ptr<Token>>::iterator &it, AST::CodeBlock &context);
	mockable std::unique_ptr<AST::rvalue> e8(
	      std::vector<std::unique_ptr<Token>>::iterator &it, AST::CodeBlock &context);
	mockable std::unique_ptr<AST::rvalue> e9(
	      std::vector<std::unique_ptr<Token>>::iterator &it, AST::CodeBlock &context);
	mockable std::unique_ptr<AST::rvalue> e10(
	      std::vector<std::unique_ptr<Token>>::iterator &it, AST::CodeBlock &context);
	mockable std::unique_ptr<AST::rvalue> e11(
	      std::vector<std::unique_ptr<Token>>::iterator &it, AST::CodeBlock &context);
	mockable std::unique_ptr<AST::rvalue> e12(
	      std::vector<std::unique_ptr<Token>>::iterator &it, AST::CodeBlock &context);
	mockable std::unique_ptr<AST::rvalue> e13(
	      std::vector<std::unique_ptr<Token>>::iterator &it, AST::CodeBlock &context);
	/**
	 * @brief Assignment operators = += -= *= /= %= <<= >>= &= ^= |= (RTL)
	 *
	 * @param it a reference to the iterator of Tokens to use. Expected to point to the
	 * first token of the expression when this function is called. When this function
	 * returns it will point to the token IMMEDIATELY AFTER the rvalue
	 * @param context the AST::CodeBlock in which the current expression occurs
	 * @return the parsed rvalue
	 */
	mockable std::unique_ptr<AST::rvalue> e14(
	      std::vector<std::unique_ptr<Token>>::iterator &it, AST::CodeBlock &context);
	mockable std::unique_ptr<AST::rvalue> e15(
	      std::vector<std::unique_ptr<Token>>::iterator &it, AST::CodeBlock &context);
public:
	~Parser() = default;
};

#endif
