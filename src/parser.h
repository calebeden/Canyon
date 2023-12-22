#ifndef PARSE_H
#define PARSE_H

#include "ast.h"
#include "tokens.h"
#include <unordered_map>

#include <cstdint>
#include <fcntl.h>
#include <vector>

class Parser {
public:
    Parser();

    /**
     * @brief Converts tokenizzed source code into an abstract syntax tree representing
     * the current module
     *
     * @param tokens the tokens to parse
     * @return the AST of the module
     */
    AST::AST *parseModule(std::vector<Token *> *tokens);

    /**
     * @brief Parses entire rvalues, taking operator precedence into consideration
     *
     * @param it a reference to the iterator of Tokens to use. Expected to point to the
     * first token of the expression when this function is called. When this function
     * returns it will point to the token IMMEDIATELY AFTER the rvalue
     * @param context the AST::CodeBlock in which the current expression occurs
     * @return the parsed rvalue
     */
    AST::rvalue *parseRvalue(std::vector<Token *>::iterator &it, AST::CodeBlock *context);

#ifndef UNIT_TEST
private:
#endif
    /**
     * @brief Converts tokens to the AST representation of all functions in the current
     * module
     *
     * @param tokens the stream of tokens representing the current module
     * @param ast the AST to place the parsed functions inside of
     */
    void parseFunctions(std::vector<Token *> *tokens, AST::AST *ast);
    /**
     * @brief Converts tokens to the AST representation of a function
     *
     * @param it a reference to the iterator of Tokens to use. Expected to point to the
     * canyon function type when this function is called. By the end of the function it
     * will point to the token IMMEDIATELY AFTER the closing curly brace
     * @param ast the AST to place the parsed function inside of
     */
    void parseFunction(std::vector<Token *>::iterator &it, AST::AST *ast);
    /**
     * @brief Converts tokens to the AST representation of a function parameter list
     *
     * @param it a reference to the iterator of Tokens to use. Expected to point to the
     * opening parenthesis when this function is called. When this function returns, it
     * will point to the token IMMEDIATELY AFTER the closing parenthesis
     * @param function the Function to determine its parameters
     */
    void parseParameters(std::vector<Token *>::iterator &it, AST::Function *function);
    /**
     * @brief Parses tokens into a code block
     *
     * @param it a reference to the iterator of Tokens to use. Expected to point to the
     * token IMMEDIATELY AFTER the opening curly brace when this function is called. When
     * this function returns it will point to the token IMMEDIATELY AFTER the closing
     * curly brace
     * @param context the "fresh" CodeBlock to fill its contents with
     */
    void parseBlock(std::vector<Token *>::iterator &it, AST::CodeBlock *context);
    /**
     * @brief Parses tokens into a statement
     *
     * @param it a reference to the iterator of Tokens to use. Expected to point to the
     * first token of the statement when this function is called. When this function
     * returns it will point to the token IMMEDIIATELY AFTER the statement
     * @param context the most specific context that the statement is a part of
     * @return the parsed Statement
     */
    AST::Statement *parseStatement(std::vector<Token *>::iterator &it,
          AST::CodeBlock *context);
    /**
     * @brief Deeper than any operators - identifiers and literals
     *
     * @param it a reference to the iterator of Tokens to use. Expected to point to the
     * first token of the expression when this function is called. When this function
     * returns it will point to the token IMMEDIATELY AFTER the rvalue
     * @param context the AST::CodeBlock in which the current expression occurs
     * @return the parsed rvalue
     */
    AST::rvalue *e0(std::vector<Token *>::iterator &it, AST::CodeBlock *context);
    /**
     * @brief () - grouping for precedence and function calls
     *
     * @param it a reference to the iterator of Tokens to use. Expected to point to the
     * first token of the expression when this function is called. When this function
     * returns it will point to the token IMMEDIATELY AFTER the rvalue
     * @param context the AST::CodeBlock in which the current expression occurs
     * @return the parsed rvalue
     */
    AST::rvalue *e1(std::vector<Token *>::iterator &it, AST::CodeBlock *context);
    AST::rvalue *e2(std::vector<Token *>::iterator &it, AST::CodeBlock *context);
    /**
     * @brief Multiplication, division, modulo operators * / % (LTR)
     *
     * @param it a reference to the iterator of Tokens to use. Expected to point to the
     * first token of the expression when this function is called. When this function
     * returns it will point to the token IMMEDIATELY AFTER the rvalue
     * @param context the AST::CodeBlock in which the current expression occurs
     * @return the parsed rvalue
     */
    AST::rvalue *e3(std::vector<Token *>::iterator &it, AST::CodeBlock *context);
    /**
     * @brief Addition and subtraction operators + - (LTR)
     *
     * @param it a reference to the iterator of Tokens to use. Expected to point to the
     * first token of the expression when this function is called. When this function
     * returns it will point to the token IMMEDIATELY AFTER the rvalue
     * @param context the AST::CodeBlock in which the current expression occurs
     * @return the parsed rvalue
     */
    AST::rvalue *e4(std::vector<Token *>::iterator &it, AST::CodeBlock *context);
    AST::rvalue *e5(std::vector<Token *>::iterator &it, AST::CodeBlock *context);
    AST::rvalue *e6(std::vector<Token *>::iterator &it, AST::CodeBlock *context);
    AST::rvalue *e7(std::vector<Token *>::iterator &it, AST::CodeBlock *context);
    AST::rvalue *e8(std::vector<Token *>::iterator &it, AST::CodeBlock *context);
    AST::rvalue *e9(std::vector<Token *>::iterator &it, AST::CodeBlock *context);
    AST::rvalue *e10(std::vector<Token *>::iterator &it, AST::CodeBlock *context);
    AST::rvalue *e11(std::vector<Token *>::iterator &it, AST::CodeBlock *context);
    AST::rvalue *e12(std::vector<Token *>::iterator &it, AST::CodeBlock *context);
    AST::rvalue *e13(std::vector<Token *>::iterator &it, AST::CodeBlock *context);
    /**
     * @brief Assignment operators = += -= *= /= %= <<= >>= &= ^= |= (RTL)
     *
     * @param it a reference to the iterator of Tokens to use. Expected to point to the
     * first token of the expression when this function is called. When this function
     * returns it will point to the token IMMEDIATELY AFTER the rvalue
     * @param context the AST::CodeBlock in which the current expression occurs
     * @return the parsed rvalue
     */
    AST::rvalue *e14(std::vector<Token *>::iterator &it, AST::CodeBlock *context);
    AST::rvalue *e15(std::vector<Token *>::iterator &it, AST::CodeBlock *context);
};

#endif
