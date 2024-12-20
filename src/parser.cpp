#include "parser.h"

#include "ast.h"
#include "errorhandler.h"

#include <memory>
#include <vector>

Parser::Parser(std::filesystem::path source, std::vector<std::unique_ptr<Token>> tokens,
      ErrorHandler *errorHandler)
    : tokens(std::move(tokens)), errorHandler(errorHandler), source(std::move(source)) {
}

std::unique_ptr<Module> Parser::parse() {
	auto mod = std::make_unique<Module>(source);
	while (!isAtEnd()) {
		std::pair<std::unique_ptr<Symbol>, std::unique_ptr<Function>> func
		      = parseFunction();
		if (func.second == nullptr) {
			synchronize();
			mustSynchronize = false;
			auto *punc = dynamic_cast<Punctuation *>(tokens[i].get());
			while (punc != nullptr) {
				if (punc->type == Punctuation::Type::Semicolon) {
					i++;
					synchronize();
					mustSynchronize = false;
					punc = dynamic_cast<Punctuation *>(tokens[i].get());
				} else if (punc->type == Punctuation::Type::CloseBrace) {
					i++;
					break;
				} else {
					std::cerr << "Unexpected token in parse" << std::endl;
					exit(EXIT_FAILURE);
				}
			}
			continue;
		}
		mod->addFunction(std::move(func.first), std::move(func.second));
	}
	return mod;
}

std::pair<std::unique_ptr<Symbol>, std::unique_ptr<Function>> Parser::parseFunction() {
	auto *keyword = dynamic_cast<Keyword *>(tokens[i].get());
	if (keyword == nullptr || keyword->type != Keyword::Type::FUN) {
		errorHandler->error(*tokens[i], "Expected keyword `fun`");
		return {nullptr, nullptr};
	}
	i++;
	auto *symbol = dynamic_cast<Symbol *>(tokens[i].get());
	if (symbol == nullptr) {
		errorHandler->error(*tokens[i], "Expected symbol following `fun`");
		return {nullptr, nullptr};
	}
	symbol = dynamic_cast<Symbol *>(tokens[i++].release());
	auto *punc = dynamic_cast<Punctuation *>(tokens[i].get());
	if (punc == nullptr || punc->type != Punctuation::Type::OpenParen) {
		errorHandler->error(*tokens[i],
		      "Expected '(' following symbol in function definition");
		mustSynchronize = true;
		return {nullptr, nullptr};
	}
	i++;
	punc = dynamic_cast<Punctuation *>(tokens[i].get());
	if (punc == nullptr || punc->type != Punctuation::Type::CloseParen) {
		errorHandler->error(*tokens[i], "Expected ')' in function definition");
		return {nullptr, nullptr};
	}
	i++;
	Symbol *type = nullptr;
	punc = dynamic_cast<Punctuation *>(tokens[i].get());
	if (punc != nullptr && punc->type == Punctuation::Type::Colon) {
		i++;
		type = dynamic_cast<Symbol *>(tokens[i].get());
		if (type == nullptr) {
			errorHandler->error(*tokens[i],
			      "Expected function return type following ':' in function definition");
			return {nullptr, nullptr};
		}
		type = dynamic_cast<Symbol *>(tokens[i++].release());
	} else {
		type = nullptr;
	}
	std::unique_ptr<BlockExpression> block = parseBlock();
	if (block == nullptr) {
		return {nullptr, nullptr};
	}

	return {std::unique_ptr<Symbol>(symbol),
	      std::make_unique<Function>(std::unique_ptr<Symbol>(type), std::move(block))};
}

std::unique_ptr<Statement> Parser::parseStatement() {
	auto *keyword = dynamic_cast<Keyword *>(tokens[i].get());
	if (keyword != nullptr && keyword->type == Keyword::Type::LET) {
		i++;
		auto *symbol = dynamic_cast<Symbol *>(tokens[i].get());
		if (symbol == nullptr) {
			errorHandler->error(*tokens[i], "Expected symbol following `let`");
			mustSynchronize = true;
			return nullptr;
		}
		symbol = dynamic_cast<Symbol *>(tokens[i++].release());
		Symbol *type = nullptr;
		auto *punc = dynamic_cast<Punctuation *>(tokens[i].get());
		if (punc != nullptr && punc->type == Punctuation::Type::Colon) {
			i++;
			type = dynamic_cast<Symbol *>(tokens[i].get());
			if (type == nullptr) {
				errorHandler->error(*tokens[i],
				      "Expected type following ':' in `let` statement");
				mustSynchronize = true;
				return nullptr;
			}
			type = dynamic_cast<Symbol *>(tokens[i++].release());
		} else {
			type = nullptr;
		}
		punc = dynamic_cast<Punctuation *>(tokens[i].get());
		if (punc != nullptr && punc->type == Punctuation::Type::Semicolon) {
			i++;
			return std::make_unique<LetStatement>(*keyword,
			      std::unique_ptr<Symbol>(symbol), std::unique_ptr<Symbol>(type), nullptr,
			      nullptr, punc);
		}
		auto *op = dynamic_cast<Operator *>(tokens[i].get());
		if (op == nullptr || op->type != Operator::Type::Assignment) {
			errorHandler->error(*tokens[i],
			      "Expected assignment expression in `let` statement");
			mustSynchronize = true;
			return nullptr;
		}
		op = dynamic_cast<Operator *>(tokens[i].release());
		i++;
		std::unique_ptr<Expression> expr = parseExpression();
		if (expr == nullptr) {
			synchronize();
			mustSynchronize = false;
			auto *punc = dynamic_cast<Punctuation *>(tokens[i].get());
			if (punc != nullptr) {
				if (punc->type == Punctuation::Type::Semicolon) {
					i++;
					return nullptr;
				}
				if (punc->type == Punctuation::Type::CloseBrace) {
					return nullptr;
				}
				std::cerr << "Unexpected token in parseExpression" << std::endl;
				exit(EXIT_FAILURE);
			}
		}
		punc = dynamic_cast<Punctuation *>(tokens[i].get());
		if (punc == nullptr || punc->type != Punctuation::Type::Semicolon) {
			errorHandler->error(*tokens[i],
			      "Expected ';' following expression in `let` statement");
			mustSynchronize = true;
			return nullptr;
		}
		i++;
		return std::make_unique<LetStatement>(*keyword, std::unique_ptr<Symbol>(symbol),
		      std::unique_ptr<Symbol>(type), std::unique_ptr<Operator>(op),
		      std::move(expr), punc);
	}

	return nullptr;
}

std::unique_ptr<Expression> Parser::parseExpression() {
	auto *p1 = dynamic_cast<Punctuation *>(tokens[i].get());
	if (p1 != nullptr && (p1->type == Punctuation::Type::OpenBrace)) {
		return parseBlock();
	}

	return parseReturnBreakExpression();
}

std::unique_ptr<BlockExpression> Parser::parseBlock() {
	auto *p1 = dynamic_cast<Punctuation *>(tokens[i].get());
	if (p1 == nullptr || p1->type != Punctuation::Type::OpenBrace) {
		errorHandler->error(*tokens[i], "Expected '{'");
		return nullptr;
	}
	std::vector<std::unique_ptr<Statement>> statements;
	i++;
	while (true) {
		std::unique_ptr<Statement> statement = parseStatement();
		if (statement != nullptr) {
			statements.push_back(std::move(statement));
			continue;
		}
		if (mustSynchronize) {
			synchronize();
			mustSynchronize = false;
			auto *punc = dynamic_cast<Punctuation *>(tokens[i].get());
			if (punc != nullptr) {
				i++;
				if (punc->type == Punctuation::Type::CloseBrace) {
					return std::make_unique<BlockExpression>(*p1, std::move(statements),
					      nullptr, *punc);
				}
			}
			if (isAtEnd()) {
				errorHandler->error(*tokens[i], "Expected '}'");
				return nullptr;
			}
			continue;
		}
		auto *p2 = dynamic_cast<Punctuation *>(tokens[i].get());
		if (p2 != nullptr && p2->type == Punctuation::Type::CloseBrace) {
			i++;
			return std::make_unique<BlockExpression>(*p1, std::move(statements), nullptr,
			      *p2);
		}
		if (isAtEnd()) {
			errorHandler->error(*tokens[i], "Expected '}'");
			return nullptr;
		}
		std::unique_ptr<Expression> expr = parseExpression();
		if (expr == nullptr) {
			synchronize();
			auto *punc = dynamic_cast<Punctuation *>(tokens[i].get());
			if (punc != nullptr) {
				i++;
				if (punc->type == Punctuation::Type::CloseBrace) {
					return std::make_unique<BlockExpression>(*p1, std::move(statements),
					      nullptr, *punc);
				}
			}
			if (isAtEnd()) {
				errorHandler->error(*tokens[i], "Expected '}'");
				return nullptr;
			}
			continue;
		}
		auto *p3 = dynamic_cast<Punctuation *>(tokens[i].get());
		if (p3 != nullptr && p3->type == Punctuation::Type::Semicolon) {
			i++;
			statements.push_back(
			      std::make_unique<ExpressionStatement>(std::move(expr), *p3));
		} else if (p3 != nullptr && p3->type == Punctuation::Type::CloseBrace) {
			i++;
			if (expr != nullptr) {
				return std::make_unique<BlockExpression>(*p1, std::move(statements),
				      std::move(expr), *p3);
			}
			return std::make_unique<BlockExpression>(*p1, std::move(statements), nullptr,
			      *p3);
		} else if (dynamic_cast<BlockExpression *>(expr.get()) != nullptr) {
			// A block expression can be a statement without semicolon if not at the end
			// of the enclosing scope
			statements.push_back(
			      std::make_unique<ExpressionStatement>(std::unique_ptr<BlockExpression>(
			            dynamic_cast<BlockExpression *>(expr.release()))));
		} else if (dynamic_cast<IfElseExpression *>(expr.get()) != nullptr) {
			// An if/else expression can be a statement without semicolon if not at the
			// end of the enclosing scope
			statements.push_back(
			      std::make_unique<ExpressionStatement>(std::unique_ptr<IfElseExpression>(
			            dynamic_cast<IfElseExpression *>(expr.release()))));
		} else if (dynamic_cast<WhileExpression *>(expr.get()) != nullptr) {
			// A while expression can be a statement without semicolon if not at the end
			// of the enclosing scope
			statements.push_back(
			      std::make_unique<ExpressionStatement>(std::unique_ptr<WhileExpression>(
			            dynamic_cast<WhileExpression *>(expr.release()))));
		} else {
			errorHandler->error(*tokens[i], "Expected '}'");
			return nullptr;
		}
	}
}

std::unique_ptr<IfElseExpression> Parser::parseIfElse() {
	auto *keyword = dynamic_cast<Keyword *>(tokens[i].get());
	if (keyword == nullptr || keyword->type != Keyword::Type::IF) {
		errorHandler->error(*tokens[i], "Expected keyword `if`");
		return nullptr;
	}
	i++;
	auto condition = parseExpression();
	auto *p1 = dynamic_cast<Punctuation *>(tokens[i].get());
	if (p1 == nullptr || p1->type != Punctuation::Type::OpenBrace) {
		errorHandler->error(*tokens[i], "Expected '{'");
		return nullptr;
	}
	auto thenBlock = parseBlock();
	if (thenBlock == nullptr) {
		return nullptr;
	}
	auto *keyword2 = dynamic_cast<Keyword *>(tokens[i].get());
	if (keyword2 == nullptr || keyword2->type != Keyword::Type::ELSE) {
		return std::make_unique<IfElseExpression>(*keyword, std::move(condition),
		      std::move(thenBlock));
	}
	i++;
	auto *keyword3 = dynamic_cast<Keyword *>(tokens[i].get());
	if (keyword3 != nullptr && keyword3->type == Keyword::Type::IF) {
		auto ifelse = parseIfElse();
		if (ifelse == nullptr) {
			return nullptr;
		}
		return std::make_unique<IfElseExpression>(*keyword, std::move(condition),
		      std::move(thenBlock), *keyword2, std::move(ifelse));
	}
	auto *p2 = dynamic_cast<Punctuation *>(tokens[i].get());
	if (p2 == nullptr || p2->type != Punctuation::Type::OpenBrace) {
		errorHandler->error(*tokens[i], "Expected '{'");
		return nullptr;
	}
	auto elseExpression = parseBlock();
	if (elseExpression == nullptr) {
		return nullptr;
	}
	return std::make_unique<IfElseExpression>(*keyword, std::move(condition),
	      std::move(thenBlock), *keyword2, std::move(elseExpression));
}

std::unique_ptr<WhileExpression> Parser::parseWhile() {
	auto *keyword = dynamic_cast<Keyword *>(tokens[i].get());
	if (keyword == nullptr || keyword->type != Keyword::Type::WHILE) {
		errorHandler->error(*tokens[i], "Expected keyword `while`");
		return nullptr;
	}
	i++;
	auto condition = parseExpression();
	auto *p1 = dynamic_cast<Punctuation *>(tokens[i].get());
	if (p1 == nullptr || p1->type != Punctuation::Type::OpenBrace) {
		errorHandler->error(*tokens[i], "Expected '{'");
		return nullptr;
	}
	auto block = parseBlock();
	if (block == nullptr) {
		return nullptr;
	}
	return std::make_unique<WhileExpression>(*keyword, std::move(condition),
	      std::move(block));
}

std::unique_ptr<Expression> Parser::parseReturnBreakExpression() {
	auto *keyword = dynamic_cast<Keyword *>(tokens[i].get());
	if (keyword != nullptr && keyword->type == Keyword::Type::RETURN) {
		i++;
		auto *punc = dynamic_cast<Punctuation *>(tokens[i].get());
		if (punc != nullptr
		      && (punc->type == Punctuation::Type::Semicolon
		            || punc->type == Punctuation::Type::CloseBrace)) {
			return std::make_unique<ReturnExpression>(*keyword, nullptr);
		}
		std::unique_ptr<Expression> expr = parseReturnBreakExpression();
		return std::make_unique<ReturnExpression>(*keyword, std::move(expr));
	}
	return parseAssignmentExpression();
}

std::unique_ptr<Expression> Parser::parseAssignmentExpression() {
	std::unique_ptr<Expression> expr = parseLogicalOrExpression();
	if (expr == nullptr) {
		return nullptr;
	}
	auto *op = dynamic_cast<Operator *>(tokens[i].get());
	if (op != nullptr && (op->type == Operator::Type::Assignment)) {
		op = dynamic_cast<Operator *>(tokens[i++].release());
		std::unique_ptr<Expression> expr2 = parseAssignmentExpression();
		if (expr2 == nullptr) {
			return nullptr;
		}
		return std::make_unique<BinaryExpression>(std::unique_ptr<Operator>(op),
		      std::move(expr), std::move(expr2));
	}
	return expr;
}

std::unique_ptr<Expression> Parser::parseLogicalOrExpression() {
	std::unique_ptr<Expression> expr = parseLogicalAndExpression();
	if (expr == nullptr) {
		return nullptr;
	}
	while (true) {
		auto *op = dynamic_cast<Operator *>(tokens[i].get());
		if (op != nullptr && op->type == Operator::Type::LogicalOr) {
			op = dynamic_cast<Operator *>(tokens[i++].release());
			std::unique_ptr<Expression> expr2 = parseLogicalAndExpression();
			if (expr2 == nullptr) {
				return nullptr;
			}
			expr = std::make_unique<BinaryExpression>(std::unique_ptr<Operator>(op),
			      std::move(expr), std::move(expr2));
		} else {
			return expr;
		}
	}
}

std::unique_ptr<Expression> Parser::parseLogicalAndExpression() {
	std::unique_ptr<Expression> expr = parseRelationalExpression();
	if (expr == nullptr) {
		return nullptr;
	}
	while (true) {
		auto *op = dynamic_cast<Operator *>(tokens[i].get());
		if (op != nullptr && op->type == Operator::Type::LogicalAnd) {
			op = dynamic_cast<Operator *>(tokens[i++].release());
			std::unique_ptr<Expression> expr2 = parseRelationalExpression();
			if (expr2 == nullptr) {
				return nullptr;
			}
			expr = std::make_unique<BinaryExpression>(std::unique_ptr<Operator>(op),
			      std::move(expr), std::move(expr2));
		} else {
			return expr;
		}
	}
}

std::unique_ptr<Expression> Parser::parseRelationalExpression() {
	std::unique_ptr<Expression> expr = parseBitwiseOrExpression();
	if (expr == nullptr) {
		return nullptr;
	}
	while (true) {
		auto *op = dynamic_cast<Operator *>(tokens[i].get());
		if (op != nullptr
		      && (op->type == Operator::Type::Equality
		            || op->type == Operator::Type::Inequality
		            || op->type == Operator::Type::LessThan
		            || op->type == Operator::Type::LessThanOrEqual
		            || op->type == Operator::Type::GreaterThan
		            || op->type == Operator::Type::GreaterThanOrEqual)) {
			op = dynamic_cast<Operator *>(tokens[i++].release());
			std::unique_ptr<Expression> expr2 = parseBitwiseOrExpression();
			if (expr2 == nullptr) {
				return nullptr;
			}
			expr = std::make_unique<BinaryExpression>(std::unique_ptr<Operator>(op),
			      std::move(expr), std::move(expr2));
		} else {
			return expr;
		}
	}
}

std::unique_ptr<Expression> Parser::parseBitwiseOrExpression() {
	std::unique_ptr<Expression> expr = parseBitwiseXorExpression();
	if (expr == nullptr) {
		return nullptr;
	}
	while (true) {
		auto *op = dynamic_cast<Operator *>(tokens[i].get());
		if (op != nullptr && op->type == Operator::Type::BitwiseOr) {
			op = dynamic_cast<Operator *>(tokens[i++].release());
			std::unique_ptr<Expression> expr2 = parseBitwiseXorExpression();
			if (expr2 == nullptr) {
				return nullptr;
			}
			expr = std::make_unique<BinaryExpression>(std::unique_ptr<Operator>(op),
			      std::move(expr), std::move(expr2));
		} else {
			return expr;
		}
	}
}

std::unique_ptr<Expression> Parser::parseBitwiseXorExpression() {
	std::unique_ptr<Expression> expr = parseBitwiseAndExpression();
	if (expr == nullptr) {
		return nullptr;
	}
	while (true) {
		auto *op = dynamic_cast<Operator *>(tokens[i].get());
		if (op != nullptr && op->type == Operator::Type::BitwiseXor) {
			op = dynamic_cast<Operator *>(tokens[i++].release());
			std::unique_ptr<Expression> expr2 = parseBitwiseAndExpression();
			if (expr2 == nullptr) {
				return nullptr;
			}
			expr = std::make_unique<BinaryExpression>(std::unique_ptr<Operator>(op),
			      std::move(expr), std::move(expr2));
		} else {
			return expr;
		}
	}
}

std::unique_ptr<Expression> Parser::parseBitwiseAndExpression() {
	std::unique_ptr<Expression> expr = parseBitshiftExpression();
	if (expr == nullptr) {
		return nullptr;
	}
	while (true) {
		auto *op = dynamic_cast<Operator *>(tokens[i].get());
		if (op != nullptr && op->type == Operator::Type::BitwiseAnd) {
			op = dynamic_cast<Operator *>(tokens[i++].release());
			std::unique_ptr<Expression> expr2 = parseBitshiftExpression();
			if (expr2 == nullptr) {
				return nullptr;
			}
			expr = std::make_unique<BinaryExpression>(std::unique_ptr<Operator>(op),
			      std::move(expr), std::move(expr2));
		} else {
			return expr;
		}
	}
}

std::unique_ptr<Expression> Parser::parseBitshiftExpression() {
	std::unique_ptr<Expression> expr = parseAdditiveExpression();
	if (expr == nullptr) {
		return nullptr;
	}
	while (true) {
		auto *op = dynamic_cast<Operator *>(tokens[i].get());
		if (op != nullptr
		      && (op->type == Operator::Type::BitwiseShiftLeft
		            || op->type == Operator::Type::BitwiseShiftRight)) {
			op = dynamic_cast<Operator *>(tokens[i++].release());
			std::unique_ptr<Expression> expr2 = parseAdditiveExpression();
			if (expr2 == nullptr) {
				return nullptr;
			}
			expr = std::make_unique<BinaryExpression>(std::unique_ptr<Operator>(op),
			      std::move(expr), std::move(expr2));
		} else {
			return expr;
		}
	}
}

std::unique_ptr<Expression> Parser::parseAdditiveExpression() {
	std::unique_ptr<Expression> expr = parseMultiplicativeExpression();
	if (expr == nullptr) {
		return nullptr;
	}
	while (true) {
		auto *op = dynamic_cast<Operator *>(tokens[i].get());
		if (op != nullptr
		      && (op->type == Operator::Type::Addition
		            || op->type == Operator::Type::Subtraction)) {
			op = dynamic_cast<Operator *>(tokens[i++].release());
			std::unique_ptr<Expression> expr2 = parseMultiplicativeExpression();
			if (expr2 == nullptr) {
				return nullptr;
			}
			expr = std::make_unique<BinaryExpression>(std::unique_ptr<Operator>(op),
			      std::move(expr), std::move(expr2));
		} else {
			return expr;
		}
	}
}

std::unique_ptr<Expression> Parser::parseMultiplicativeExpression() {
	std::unique_ptr<Expression> expr = parseUnaryExpression();
	if (expr == nullptr) {
		return nullptr;
	}
	while (true) {
		auto *op = dynamic_cast<Operator *>(tokens[i].get());
		if (op != nullptr
		      && (op->type == Operator::Type::Multiplication
		            || op->type == Operator::Type::Division
		            || op->type == Operator::Type::Modulus)) {
			op = dynamic_cast<Operator *>(tokens[i++].release());
			std::unique_ptr<Expression> expr2 = parseUnaryExpression();
			if (expr2 == nullptr) {
				return nullptr;
			}
			expr = std::make_unique<BinaryExpression>(std::unique_ptr<Operator>(op),
			      std::move(expr), std::move(expr2));
		} else {
			return expr;
		}
	}
}

std::unique_ptr<Expression> Parser::parseUnaryExpression() {
	auto *op = dynamic_cast<Operator *>(tokens[i].get());
	if (op != nullptr
	      && (op->type == Operator::Type::LogicalNot
	            || op->type == Operator::Type::BitwiseNot
	            || op->type == Operator::Type::Subtraction
	            || op->type == Operator::Type::Addition)) {
		op = dynamic_cast<Operator *>(tokens[i++].release());
		std::unique_ptr<Expression> expr = parseUnaryExpression();
		if (expr == nullptr) {
			return nullptr;
		}
		return std::make_unique<UnaryExpression>(std::unique_ptr<Operator>(op),
		      std::move(expr));
	}
	return parseFunctionCallExpression();
}

std::unique_ptr<Expression> Parser::parseFunctionCallExpression() {
	std::unique_ptr<Expression> expr = parsePrimaryExpression();
	if (expr == nullptr) {
		return nullptr;
	}
	auto *p1 = dynamic_cast<Punctuation *>(tokens[i].get());
	if (p1 != nullptr && (p1->type == Punctuation::Type::OpenParen)) {
		auto *p2 = dynamic_cast<Punctuation *>(tokens[i + 1].get());
		if (p2 != nullptr && (p2->type == Punctuation::Type::CloseParen)) {
			i += 2;
			return std::make_unique<FunctionCallExpression>(std::move(expr));
		}
	}
	return expr;
}

std::unique_ptr<Expression> Parser::parsePrimaryExpression() {
	auto *integerLiteral = dynamic_cast<IntegerLiteral *>(tokens[i].get());
	if (integerLiteral != nullptr) {
		integerLiteral = dynamic_cast<IntegerLiteral *>(tokens[i++].release());
		return std::make_unique<IntegerLiteralExpression>(
		      std::unique_ptr<IntegerLiteral>(integerLiteral));
	}

	auto *boolLiteral = dynamic_cast<BoolLiteral *>(tokens[i].get());
	if (boolLiteral != nullptr) {
		boolLiteral = dynamic_cast<BoolLiteral *>(tokens[i++].release());
		return std::make_unique<BoolLiteralExpression>(
		      std::unique_ptr<BoolLiteral>(boolLiteral));
	}

	auto *symbol = dynamic_cast<Symbol *>(tokens[i].get());
	if (symbol != nullptr) {
		symbol = dynamic_cast<Symbol *>(tokens[i++].release());
		return std::make_unique<SymbolExpression>(std::unique_ptr<Symbol>(symbol));
	}

	auto *p1 = dynamic_cast<Punctuation *>(tokens[i].get());
	if (p1 != nullptr && (p1->type == Punctuation::Type::OpenParen)) {
		i++;
		auto expr = parseExpression();
		auto *p2 = dynamic_cast<Punctuation *>(tokens[i].get());
		if (p2 == nullptr || p2->type != Punctuation::Type::CloseParen) {
			errorHandler->error(*tokens[i], "Expected ')'");
			return nullptr;
		}
		i++;
		return std::make_unique<ParenthesizedExpression>(*p1, std::move(expr), *p2);
	}
	if (p1 != nullptr && (p1->type == Punctuation::Type::OpenBrace)) {
		return parseBlock();
	}

	auto *keyword = dynamic_cast<Keyword *>(tokens[i].get());
	if (keyword != nullptr && keyword->type == Keyword::Type::IF) {
		return parseIfElse();
	}
	if (keyword != nullptr && keyword->type == Keyword::Type::WHILE) {
		return parseWhile();
	}

	errorHandler->error(*tokens[i], "Expected expression");
	return nullptr;
}

void Parser::synchronize() {
	while (!isAtEnd()) {
		auto *p = dynamic_cast<Punctuation *>(tokens[i].get());
		if (p != nullptr
		      && (p->type == Punctuation::Type::Semicolon
		            || p->type == Punctuation::Type::CloseBrace)) {
			return;
		}
		i++;
	}
}

bool Parser::isAtEnd() const {
	return dynamic_cast<EndOfFile *>(tokens[i].get()) != nullptr;
}
