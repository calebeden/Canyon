#ifndef PARSER_H
#define PARSER_H

#include "ast.h"
#include "errorhandler.h"
#include "tokens.h"

#include <memory>
#include <utility>
#include <vector>

/**
 * @brief Parses a series of Tokens into an AST
 * 
 */
class Parser {
	std::vector<std::unique_ptr<Token>> tokens;
	ErrorHandler *errorHandler;
	size_t i = 0;
	bool mustSynchronize = false;
	std::filesystem::path source;
public:
	Parser(std::filesystem::path source, std::vector<std::unique_ptr<Token>> tokens,
	      ErrorHandler *errorHandler);
	std::unique_ptr<Module> parse();
	~Parser() = default;
private:
	std::pair<std::unique_ptr<Symbol>, std::unique_ptr<Function>> parseFunction();
	std::unique_ptr<Statement> parseStatement();
	std::unique_ptr<Expression> parseExpression();
	std::unique_ptr<BlockExpression> parseBlock();
	std::unique_ptr<Expression> parseReturnBreakExpression();
	std::unique_ptr<Expression> parseAssignmentExpression();
	std::unique_ptr<Expression> parseLogicalOrExpression();
	std::unique_ptr<Expression> parseLogicalAndExpression();
	std::unique_ptr<Expression> parseRelationalExpression();
	std::unique_ptr<Expression> parseBitwiseOrExpression();
	std::unique_ptr<Expression> parseBitwiseXorExpression();
	std::unique_ptr<Expression> parseBitwiseAndExpression();
	std::unique_ptr<Expression> parseBitshiftExpression();
	std::unique_ptr<Expression> parseAdditiveExpression();
	std::unique_ptr<Expression> parseMultiplicativeExpression();
	std::unique_ptr<Expression> parseUnaryExpression();
	std::unique_ptr<Expression> parseFunctionCallExpression();
	std::unique_ptr<Expression> parsePrimaryExpression();
	void synchronize();
	bool isAtEnd() const;
};

#endif
