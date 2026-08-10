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
		auto *keyword = dynamic_cast<Keyword *>(tokens[i].get());
		if (keyword != nullptr && keyword->type == Keyword::Type::FUN) {
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
		} else if (keyword != nullptr && keyword->type == Keyword::Type::CLASS) {
			std::pair<std::unique_ptr<Symbol>, std::unique_ptr<Class>> cls = parseClass();
			if (cls.second == nullptr) {
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
			mod->addClass(std::move(cls.first), std::move(cls.second));
		} else if (keyword != nullptr && keyword->type == Keyword::Type::IMPL) {
			std::pair<std::unique_ptr<Symbol>, std::unique_ptr<Impl>> impl = parseImpl();
			if (impl.second == nullptr) {
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
			mod->addImpl(std::move(impl.first), std::move(impl.second));
		} else {
			errorHandler->error(*tokens[i], "Expected keyword `fun` or `class`");
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
	}
	return mod;
}

std::pair<std::unique_ptr<Symbol>, std::unique_ptr<Function>> Parser::parseFunction(
      bool isConstructor) {
	auto *keyword = dynamic_cast<Keyword *>(tokens[i].get());
	if (!isConstructor) {
		if (keyword == nullptr || keyword->type != Keyword::Type::FUN) {
			errorHandler->error(*tokens[i], "Expected keyword `fun`");
			return {nullptr, nullptr};
		}
	} else {
		if (keyword == nullptr || keyword->type != Keyword::Type::CONSTRUCTOR) {
			errorHandler->error(*tokens[i], "Expected keyword `constructor`");
			return {nullptr, nullptr};
		}
	}
	i++;
	auto *symbol = dynamic_cast<Symbol *>(tokens[i].get());
	if (symbol == nullptr) {
		if (!isConstructor) {
			errorHandler->error(*tokens[i], "Expected symbol following `fun`");
		} else {
			errorHandler->error(*tokens[i], "Expected symbol following `constructor`");
		}
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

	std::vector<std::pair<std::unique_ptr<Symbol>, std::unique_ptr<Symbol>>> parameters;
	auto *argSymbol = dynamic_cast<Symbol *>(tokens[i].get());
	while (argSymbol != nullptr) {
		argSymbol = dynamic_cast<Symbol *>(tokens[i++].release());
		auto *p1 = dynamic_cast<Punctuation *>(tokens[i].get());
		if (p1 == nullptr || p1->type != Punctuation::Type::Colon) {
			errorHandler->error(*tokens[i],
			      "Expected ':' following symbol in function definition");
			return {nullptr, nullptr};
		}
		i++;
		auto *type = dynamic_cast<Symbol *>(tokens[i].get());
		if (type == nullptr) {
			errorHandler->error(*tokens[i],
			      "Expected type following ':' in function definition");
			return {nullptr, nullptr};
		}
		type = dynamic_cast<Symbol *>(tokens[i++].release());
		parameters.emplace_back(std::unique_ptr<Symbol>(argSymbol),
		      std::unique_ptr<Symbol>(type));
		auto *p2 = dynamic_cast<Punctuation *>(tokens[i].get());
		if (p2 == nullptr) {
			errorHandler->error(*tokens[i], "Expected ',' or ')' in function definition");
			return {nullptr, nullptr};
		}
		if (p2->type == Punctuation::Type::CloseParen) {
			break;
		}
		if (p2->type != Punctuation::Type::Comma) {
			errorHandler->error(*tokens[i], "Expected ',' or ')' in function definition");
			return {nullptr, nullptr};
		}
		i++;
		argSymbol = dynamic_cast<Symbol *>(tokens[i].get());
	}

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
	      std::make_unique<Function>(std::move(parameters), std::unique_ptr<Symbol>(type),
	            std::move(block),
	            isConstructor ? FunctionVariant::CONSTRUCTOR
	                          : FunctionVariant::FUNCTION)};
}

std::pair<std::unique_ptr<Symbol>, std::unique_ptr<Class>> Parser::parseClass() {
	auto *keyword = dynamic_cast<Keyword *>(tokens[i].get());
	if (keyword == nullptr || keyword->type != Keyword::Type::CLASS) {
		errorHandler->error(*tokens[i], "Expected keyword `class`");
		return {nullptr, nullptr};
	}
	i++;
	auto *symbol = dynamic_cast<Symbol *>(tokens[i].get());
	if (symbol == nullptr) {
		errorHandler->error(*tokens[i], "Expected symbol following `class`");
		return {nullptr, nullptr};
	}
	symbol = dynamic_cast<Symbol *>(tokens[i++].release());
	auto *punc = dynamic_cast<Punctuation *>(tokens[i].get());
	if (punc == nullptr || punc->type != Punctuation::Type::OpenBrace) {
		errorHandler->error(*tokens[i],
		      "Expected '{' following symbol in class definition");
		mustSynchronize = true;
		return {nullptr, nullptr};
	}
	i++;

	std::vector<std::unique_ptr<LetStatement>> fields;
	while (true) {
		auto *p1 = dynamic_cast<Punctuation *>(tokens[i].get());
		if (p1 != nullptr && p1->type == Punctuation::Type::CloseBrace) {
			break;
		}
		auto *field = dynamic_cast<Symbol *>(tokens[i].get());
		if (field == nullptr) {
			errorHandler->error(*tokens[i],
			      "Expected symbol for class field declaration");
			mustSynchronize = true;
			return {nullptr, nullptr};
		}
		field = dynamic_cast<Symbol *>(tokens[i++].release());
		auto *p2 = dynamic_cast<Punctuation *>(tokens[i].get());
		if (p2 == nullptr || p2->type != Punctuation::Type::Colon) {
			errorHandler->error(*tokens[i],
			      "Expected ':' following symbol in class field declaration");
			mustSynchronize = true;
			return {nullptr, nullptr};
		}
		i++;
		Symbol *type = dynamic_cast<Symbol *>(tokens[i].get());
		if (type == nullptr) {
			errorHandler->error(*tokens[i],
			      "Expected type following ':' in `let` statement");
			mustSynchronize = true;
			return {nullptr, nullptr};
		}
		type = dynamic_cast<Symbol *>(tokens[i++].release());
		auto *p3 = dynamic_cast<Punctuation *>(tokens[i].get());
		if (p3 == nullptr || p3->type != Punctuation::Type::Semicolon) {
			errorHandler->error(*tokens[i],
			      "Expected ';' following type in class field declaration");
			mustSynchronize = true;
			return {nullptr, nullptr};
		}
		i++;

		fields.emplace_back(std::make_unique<LetStatement>(std::unique_ptr<Symbol>(field),
		      std::unique_ptr<Symbol>(type), punc));
	}

	auto *p2 = dynamic_cast<Punctuation *>(tokens[i].get());
	if (p2 == nullptr || p2->type != Punctuation::Type::CloseBrace) {
		errorHandler->error(*tokens[i], "Expected '}' after class definition");
		return {nullptr, nullptr};
	}
	i++;

	return {std::unique_ptr<Symbol>(symbol), std::make_unique<Class>(std::move(fields))};
}

std::pair<std::unique_ptr<Symbol>, std::unique_ptr<Impl>> Parser::parseImpl() {
	auto *keyword = dynamic_cast<Keyword *>(tokens[i].get());
	if (keyword == nullptr || keyword->type != Keyword::Type::IMPL) {
		errorHandler->error(*tokens[i], "Expected keyword `impl`");
		return {nullptr, nullptr};
	}
	i++;
	auto *symbol = dynamic_cast<Symbol *>(tokens[i].get());
	if (symbol == nullptr) {
		errorHandler->error(*tokens[i], "Expected symbol following `impl`");
		return {nullptr, nullptr};
	}
	symbol = dynamic_cast<Symbol *>(tokens[i++].release());
	auto *punc = dynamic_cast<Punctuation *>(tokens[i].get());
	if (punc == nullptr || punc->type != Punctuation::Type::OpenBrace) {
		errorHandler->error(*tokens[i], "Expected '{' following symbol in impl block");
		mustSynchronize = true;
		return {nullptr, nullptr};
	}
	i++;

	std::unordered_map<std::string_view, std::unique_ptr<Function>> methods;
	while (true) {
		auto *keyword2 = dynamic_cast<Keyword *>(tokens[i].get());
		if (keyword2 != nullptr && keyword2->type == Keyword::Type::FUN) {
			std::pair<std::unique_ptr<Symbol>, std::unique_ptr<Function>> method
			      = parseFunction();
			if (method.second == nullptr) {
				return {nullptr, nullptr};
			}
			methods.emplace(method.first->s.contents, std::move(method.second));
		} else if (keyword2 != nullptr && keyword2->type == Keyword::Type::CONSTRUCTOR) {
			std::pair<std::unique_ptr<Symbol>, std::unique_ptr<Function>> method
			      = parseFunction(true);
			if (method.second == nullptr) {
				return {nullptr, nullptr};
			}
			methods.emplace(method.first->s.contents, std::move(method.second));
		} else if (keyword2 != nullptr) {
			errorHandler->error(*tokens[i], "Unexpected keyword in impl block");
			mustSynchronize = true;
			return {nullptr, nullptr};
		} else {
			break;
		}
	}

	auto *p2 = dynamic_cast<Punctuation *>(tokens[i].get());
	if (p2 == nullptr || p2->type != Punctuation::Type::CloseBrace) {
		errorHandler->error(*tokens[i], "Expected '}' after impl block");
		return {nullptr, nullptr};
	}
	i++;

	return {std::unique_ptr<Symbol>(symbol), std::make_unique<Impl>(std::move(methods))};
}

std::unique_ptr<Statement> Parser::parseStatement() {
	auto *keyword = dynamic_cast<Keyword *>(tokens[i].get());
	if (keyword != nullptr && keyword->type == Keyword::Type::LET) {
		return parseLet();
	}
	return nullptr;
}

std::unique_ptr<LetStatement> Parser::parseLet() {
	auto *keyword = dynamic_cast<Keyword *>(tokens[i].get());
	if (keyword == nullptr || keyword->type != Keyword::Type::LET) {
		errorHandler->error(*tokens[i], "Expected keyword `let`");
		mustSynchronize = true;
		return nullptr;
	}
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
		return std::make_unique<LetStatement>(*keyword, std::unique_ptr<Symbol>(symbol),
		      std::unique_ptr<Symbol>(type), nullptr, nullptr, punc);
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
	      std::unique_ptr<Symbol>(type), std::unique_ptr<Operator>(op), std::move(expr),
	      punc);
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
		            || punc->type == Punctuation::Type::CloseBrace
		            || punc->type == Punctuation::Type::CloseParen
		            || punc->type == Punctuation::Type::Comma)) {
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
	return parseFieldAccessExpression();
}

std::unique_ptr<Expression> Parser::parseFieldAccessExpression() {
	std::unique_ptr<Expression> expr = parseFunctionCallExpression();
	if (expr == nullptr) {
		return nullptr;
	}
	while (true) {
		auto *punc = dynamic_cast<Punctuation *>(tokens[i].get());
		if (punc != nullptr && punc->type == Punctuation::Type::Period) {
			i++;
			std::unique_ptr<Expression> expr2 = parseFunctionCallExpression();
			if (expr2 == nullptr) {
				return nullptr;
			}
			if (!dynamic_cast<SymbolExpression *>(expr2.get())
			      && !dynamic_cast<FunctionCallExpression *>(expr2.get())) {
				errorHandler->error(*tokens[i], "Expected symbol or method call");
				return nullptr;
			}
			expr = std::make_unique<FieldAccessExpression>(std::move(expr),
			      std::move(expr2));
		} else {
			return expr;
		}
	}
}

std::unique_ptr<Expression> Parser::parseFunctionCallExpression() {
	std::unique_ptr<Expression> expr = parsePathExpression();
	if (expr == nullptr) {
		return nullptr;
	}
	auto *p1 = dynamic_cast<Punctuation *>(tokens[i].get());
	if (p1 != nullptr && (p1->type == Punctuation::Type::OpenParen)) {
		i++;
		std::vector<std::unique_ptr<Expression>> arguments;
		auto *p2 = dynamic_cast<Punctuation *>(tokens[i].get());
		while (p2 == nullptr || p2->type != Punctuation::Type::CloseParen) {
			std::unique_ptr<Expression> arg = parseExpression();
			if (arg == nullptr) {
				return nullptr;
			}
			arguments.push_back(std::move(arg));
			p2 = dynamic_cast<Punctuation *>(tokens[i].get());
			if (p2 == nullptr) {
				errorHandler->error(*tokens[i], "Expected ',' or ')'");
				return nullptr;
			}
			if (p2->type == Punctuation::Type::CloseParen) {
				break;
			}
			if (p2->type != Punctuation::Type::Comma) {
				errorHandler->error(*tokens[i], "Expected ',' or ')'");
				return nullptr;
			}
			i++;
		}
		i++;
		return std::make_unique<FunctionCallExpression>(std::move(expr), *p1,
		      std::move(arguments), *p2);
	}
	return expr;
}

std::unique_ptr<Expression> Parser::parsePathExpression() {
	std::unique_ptr<Expression> expr = parsePrimaryExpression();
	if (expr == nullptr) {
		return nullptr;
	}
	if (!dynamic_cast<SymbolExpression *>(expr.get())) {
		return expr;
	}
	std::unique_ptr<SymbolExpression> symExpr(
	      dynamic_cast<SymbolExpression *>(expr.release()));
	std::vector<std::unique_ptr<SymbolExpression>> symbols;
	symbols.push_back(std::move(symExpr));
	while (true) {
		auto *op = dynamic_cast<Operator *>(tokens[i].get());
		if (op != nullptr && op->type == Operator::Type::Scope) {
			op = dynamic_cast<Operator *>(tokens[i++].release());
			std::unique_ptr<Expression> expr2 = parsePrimaryExpression();
			if (expr2 == nullptr) {
				return nullptr;
			}
			if (!dynamic_cast<SymbolExpression *>(expr2.get())) {
				errorHandler->error(*tokens[i], "Expected symbol");
				return nullptr;
			}
			symExpr = std::unique_ptr<SymbolExpression>(
			      dynamic_cast<SymbolExpression *>(expr2.release()));
			symbols.push_back(std::move(symExpr));
		} else {
			if (symbols.size() == 1) {
				return std::move(symbols[0]);
			} else {
				return std::make_unique<PathExpression>(std::move(symbols));
			}
		}
	}
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

	auto *charLiteral = dynamic_cast<CharacterLiteral *>(tokens[i].get());
	if (charLiteral != nullptr) {
		charLiteral = dynamic_cast<CharacterLiteral *>(tokens[i++].release());
		return std::make_unique<CharacterLiteralExpression>(
		      std::unique_ptr<CharacterLiteral>(charLiteral));
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
