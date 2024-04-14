#include "parser.h"

#include "ast.h"
#include "tokens.h"
#include <string_view>
#include <unordered_map>

#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <memory>
#include <stdexcept>
#include <string>
#include <typeinfo>
#include <vector>

using namespace AST;

Parser::Parser() = default;

Module Parser::parseModule(std::vector<Token *> &tokens) {
	Module module = Module();
	parseFunctions(tokens, module);
	if (module.functions.find("canyonMain") == module.functions.end()) {
		std::cerr << "Parse error: no main function\n";
		exit(EXIT_FAILURE);
	}
	module.resolve(errors);
	return module;
}

void Parser::parseFunctions(std::vector<Token *> &tokens, Module &module) {
	auto it = tokens.begin();
	while (it < tokens.end()) {
		parseFunction(it, module);
	}
}

void Parser::parseFunction(std::vector<Token *>::iterator &it, Module &module) {
	Type type;
	if (auto *primitive = dynamic_cast<Primitive *>(*it)) {
		type = primitive->type;
	} else if (auto *keyword = dynamic_cast<Keyword *>(*it);
	           keyword && keyword->type == Keyword::Type::VOID) {
		type = Type::VOID;
	} else {
		errors.error(*it, "Expected function type");
	}
	it++;
	if (dynamic_cast<Identifier *>(*it) == nullptr) {
		errors.error(*it, "Expected identifier");
	}
	std::string_view name = dynamic_cast<Identifier *>(*it)->s;
	if (name == "main") {
		name = "canyonMain";
	}
	it++;
	Function *function = new Function(&module);

	parseParameters(it, function);

	parseBlock(it, *function->body);

	for (const std::unique_ptr<Statement> &s : function->body->statements) {
		s->print(std::cerr);
	}
	function->type = type;
	module.functions[name] = function;
}

void Parser::parseParameters(std::vector<Token *>::iterator &it, Function *function) {
	if (auto *punc = dynamic_cast<Punctuation *>(*it);
	      !punc || punc->type != Punctuation::Type::OpenParen) {
		errors.error(*it, "Expected '('");
	}
	it++;
	if (auto *punc = dynamic_cast<Punctuation *>(*it)) {
		if (punc->type == Punctuation::Type::CloseParen) {
			it++;
			return;
		}
	}

	CodeBlock *context = function->body;
	while (true) {
		if (auto *type = dynamic_cast<Primitive *>(*it)) {
			it++;
			if (auto *id = dynamic_cast<Identifier *>(*it)) {
				it++;
				if (context->locals.find(id) != context->locals.end()) {
					errors.error(id,
					      std::string("Re-declaration of parameter ").append(id->s));
				}
				context->locals.insert({
				      id, {type->type, true}
                });
				function->parameters.emplace_back(id, type->type);
			} else {
				errors.error(*it, "Unexpected token following primitive");
			}
		} else {
			errors.error(*it, "Expected primitive");
		}
		if (auto *punc = dynamic_cast<Punctuation *>(*it)) {
			if (punc->type == Punctuation::Type::CloseParen) {
				it++;
				return;
			}
			if (punc->type == Punctuation::Type::Comma) {
				it++;
				continue;
			}
		}
		errors.error(*it, "Unexpected token; expected ')' or ','");
	}
}

void Parser::parseBlock(std::vector<Token *>::iterator &it, CodeBlock &context) {
	if (auto *punc = dynamic_cast<Punctuation *>(*it);
	      !punc || punc->type != Punctuation::Type::OpenBrace) {
		errors.error(*it, "Expected '{'");
	}
	it++;
	auto *punc = dynamic_cast<Punctuation *>(*it);
	while (!punc || punc->type != Punctuation::Type::CloseBrace) {
		std::unique_ptr<Statement> s = parseStatement(it, context);
		if (s != nullptr) {
			context.statements.push_back(std::move(s));
		}
		punc = dynamic_cast<Punctuation *>(*it);
	}
	it++;
}

std::unique_ptr<Statement> Parser::parseStatement(std::vector<Token *>::iterator &it,
      CodeBlock &context) {
	auto *punc = dynamic_cast<Punctuation *>(*it);
	while (punc && punc->type == Punctuation::Type::Semicolon) {
		it++;
		punc = dynamic_cast<Punctuation *>(*it);
	}
	if (punc && punc->type == Punctuation::Type::CloseBrace) {
		return nullptr;
	}

	std::unique_ptr<rvalue> rval;
	if (auto *type = dynamic_cast<Primitive *>(*it)) {
		it++;
		if (auto *id = dynamic_cast<Identifier *>(*it)) {
			if (context.locals.find(id) != context.locals.end()) {
				errors.error(*it,
				      std::string("Re-declaration of variable ").append(id->s));
			}
			context.locals.insert({
			      id, {type->type, false}
            });
			rval = parseRvalue(it, context);
			if (!rval) {
				errors.error(*it, "parseRvalue should have at least returned the "
				                  "Variable we declared...");
			}
			if (dynamic_cast<Variable *>(rval.get())) {
				if (auto *punc = dynamic_cast<Punctuation *>(*it);
				      !punc || punc->type != Punctuation::Type::Semicolon) {
					errors.error(*it, "Expected ';' after statement");
				}
				it++;
				return nullptr;
			}
			if (!dynamic_cast<Assignment *>(rval.get())) {
				errors.error(*it, "Unexpected expression following declaration");
			}
		} else {
			errors.error(*it, "Unexpected token following primitive");
		}
	} else if (auto *keyword = dynamic_cast<Keyword *>(*it);
	           keyword && keyword->type == Keyword::Type::RETURN) {
		Token *return_token = *it;
		it++;
		rval = parseRvalue(it, context);
		if (auto *punc = dynamic_cast<Punctuation *>(*it);
		      !punc || punc->type != Punctuation::Type::Semicolon) {
			if (punc && punc->type == Punctuation::Type::Equals) {
				errors.error(*it, "LHS of assignment is not a variable");
			}
			errors.error(*it, "Expected ';' after statement");
		}
		it++;
		return std::make_unique<Return>(std::move(rval), return_token);
	} else {
		rval = parseRvalue(it, context);
	}

	if (auto *punc = dynamic_cast<Punctuation *>(*it);
	      !punc || punc->type != Punctuation::Type::Semicolon) {
		if (punc && punc->type == Punctuation::Type::Equals) {
			errors.error(*it, "LHS of assignment is not a variable");
		}
		errors.error(*it, "Expected ';' after statement");
	}
	it++;
	if (rval != nullptr) {
		return std::make_unique<Expression>(std::move(rval));
	}
	return nullptr;
}

/*
https://en.cppreference.com/w/c/language/operator_precedence
Precedence          Operator            Associativity
1                   ++ Postfix          LTR
                    -- Postfix
                    () Function call
                    [] Array index
                    . Struct and union member
                    -> struct and union member
                    (type){list} Compound literal (C99)
2                   ++ Prefix           RTL
                    -- Prefix
                    + Unary Plus
                    - Unary Subtract
                    ! Not
                    ~ B/W Not
                    (type) Cast
                    * Dereference
                    & Address of
                    sizeof
                    alignof
3                   * / %               LTR
4                   + -                 LTR
5                   << >>               LTR
6                   < <= > >=           LTR
7                   == !=               LTR
8                   & B/W AND           LTR
9                   ^ B/W XOR           LTR
10                  | B/W OR            LTR
11                  && Log AND          LTR
12                  || Log OR           LTR
13                  ?: Ternary          RTL
14                  = += -= *= /= %=    RTL
                    <<= >>= &= ^= |=
15                  , Comma             LTR
*/

/*
    RTL template:
    eN() {
        if (is this operator) {
            rval = new rvalue;
            rval.operand = eN();
            return rval;
        }
        return eN-1();
    }

    LTR template:
    eN() {
        operand = eN-1();
        while (true) {
            if (is this operator) {
                rval = new rvalue;
                rval.operands = operand0, eN();
                operand = rvalue;
            } else {
                return operand;
            }
        }
    }
*/

std::unique_ptr<AST::rvalue> Parser::e0(std::vector<Token *>::iterator &it) {
	if (auto *id = dynamic_cast<Identifier *>(*it)) {
		it++;
		bool isInt = true;
		for (char c : id->s) {
			if (std::isdigit(c) == 0) {
				isInt = false;
				break;
			}
		}
		if (isInt) {
			return std::make_unique<Literal>(id);
		}
		return std::make_unique<Variable>(id);
	}
	return nullptr;
}

std::unique_ptr<AST::rvalue> Parser::e1(std::vector<Token *>::iterator &it,
      CodeBlock &context) {
	std::unique_ptr<rvalue> temp = e0(it);

	if (auto *punc = dynamic_cast<Punctuation *>(*it);
	      punc && punc->type == Punctuation::Type::OpenParen) {
		it++;
		std::unique_ptr<rvalue> rval = parseRvalue(it, context);
		Variable *tmp = dynamic_cast<Variable *>(temp.get());
		if (tmp != nullptr) {
			std::unique_ptr<Variable> id;
			temp.release();
			id.reset(tmp);
			CodeBlock::IdentifierStatus status = context.find(id.get()); // TODO
			switch (status) {
				case CodeBlock::IdentifierStatus::VARIABLE: {
					errors.error(*it, std::string(id->variable->s) + " is not callable");
					break;
				}
				case CodeBlock::IdentifierStatus::UNKNOWN: {
					context.defer(id.get()); // TODO
				}
				case CodeBlock::IdentifierStatus::FUNCTION: {
					break;
				}
			}
			std::unique_ptr<FunctionCall> call
			      = std::make_unique<FunctionCall>(std::move(id));
			while (rval) {
				call->arguments.push_back(std::move(rval));
				if (auto *punc = dynamic_cast<Punctuation *>(*it);
				      punc && punc->type == Punctuation::Type::Comma) {
					it++;
					rval = parseRvalue(it, context);
					if (rval == nullptr) {
						// Not tested since I'm not sure how this would even happen
						errors.error(*it, "How did parseRvalue return null?");
					}
				} else {
					rval = nullptr;
				}
			}
			// context.global->functionCalls.push_back(call);
			// TODO replace the global functionCalls with recursion through Statements and
			// rvalues
			rval = std::move(call);
		}
		if (rval == nullptr) {
			errors.error(*it, "Expected expression");
		}
		if (auto *punc = dynamic_cast<Punctuation *>(*it);
		      !punc || punc->type != Punctuation::Type::CloseParen) {
			errors.error(*it, "Expected ')'");
		}
		it++;
		return rval;
	}

	Variable *tmp = dynamic_cast<Variable *>(temp.get());
	std::unique_ptr<Variable> id;
	if (tmp != nullptr) {
		temp.release();
		id.reset(tmp);
		CodeBlock::IdentifierStatus status = context.find(id.get()); // TODO
		switch (status) {
			case CodeBlock::IdentifierStatus::FUNCTION: {
				errors.error(*it, std::string(id->variable->s) + " is not a variable");
				break;
			}
			case CodeBlock::IdentifierStatus::UNKNOWN: {
				context.defer(id.get()); // TODO
			}
			case CodeBlock::IdentifierStatus::VARIABLE: {
				break;
			}
		}
		return id;
	}
	return temp;
}

std::unique_ptr<AST::rvalue> Parser::e2(std::vector<Token *>::iterator &it,
      CodeBlock &context) {
	return e1(it, context);
}

std::unique_ptr<AST::rvalue> Parser::e3(std::vector<Token *>::iterator &it,
      CodeBlock &context) {
	std::unique_ptr<rvalue> operand1 = e2(it, context);
	while (true) {
		if (auto *punc = dynamic_cast<Punctuation *>(*it)) {
			if (punc->type == Punctuation::Type::Times) {
				it++;
				operand1 = std::make_unique<Multiplication>(std::move(operand1),
				      e2(it, context));
			} else if (punc->type == Punctuation::Type::Divide) {
				it++;
				operand1
				      = std::make_unique<Division>(std::move(operand1), e2(it, context));
			} else if (punc->type == Punctuation::Type::Mod) {
				it++;
				operand1 = std::make_unique<Modulo>(std::move(operand1), e2(it, context));
			} else {
				return operand1;
			}
		} else {
			return operand1;
		}
	}
}

std::unique_ptr<AST::rvalue> Parser::e4(std::vector<Token *>::iterator &it,
      CodeBlock &context) {
	std::unique_ptr<rvalue> operand1 = e3(it, context);
	while (true) {
		if (auto *punc = dynamic_cast<Punctuation *>(*it)) {
			if (punc->type == Punctuation::Type::Plus) {
				it++;
				operand1
				      = std::make_unique<Addition>(std::move(operand1), e3(it, context));
			} else if (punc->type == Punctuation::Type::Minus) {
				it++;
				operand1 = std::make_unique<Subtraction>(std::move(operand1),
				      e3(it, context));
			} else {
				return operand1;
			}
		} else {
			return operand1;
		}
	}
}

std::unique_ptr<AST::rvalue> Parser::e5(std::vector<Token *>::iterator &it,
      CodeBlock &context) {
	return e4(it, context);
}

std::unique_ptr<AST::rvalue> Parser::e6(std::vector<Token *>::iterator &it,
      CodeBlock &context) {
	return e5(it, context);
}

std::unique_ptr<AST::rvalue> Parser::e7(std::vector<Token *>::iterator &it,
      CodeBlock &context) {
	return e6(it, context);
}

std::unique_ptr<AST::rvalue> Parser::e8(std::vector<Token *>::iterator &it,
      CodeBlock &context) {
	return e7(it, context);
}

std::unique_ptr<AST::rvalue> Parser::e9(std::vector<Token *>::iterator &it,
      CodeBlock &context) {
	return e8(it, context);
}

std::unique_ptr<AST::rvalue> Parser::e10(std::vector<Token *>::iterator &it,
      CodeBlock &context) {
	return e9(it, context);
}

std::unique_ptr<AST::rvalue> Parser::e11(std::vector<Token *>::iterator &it,
      CodeBlock &context) {
	return e10(it, context);
}

std::unique_ptr<AST::rvalue> Parser::e12(std::vector<Token *>::iterator &it,
      CodeBlock &context) {
	return e11(it, context);
}

std::unique_ptr<AST::rvalue> Parser::e13(std::vector<Token *>::iterator &it,
      CodeBlock &context) {
	return e12(it, context);
}

std::unique_ptr<AST::rvalue> Parser::e14(std::vector<Token *>::iterator &it,
      CodeBlock &context) {
	if (auto *id = dynamic_cast<Identifier *>(*it)) {
		if (auto *punc = dynamic_cast<Punctuation *>(*(it + 1));
		      punc && punc->type == Punctuation::Type::Equals) {
			if (context.locals.find(id) == context.locals.end()) {
				errors.error(id, std::string("Undeclared variable ").append(id->s));
			}
			it += 2;
			return std::make_unique<Assignment>(id, e14(it, context));
		}
		return e13(it, context);
	}
	return e13(it, context);
}

std::unique_ptr<AST::rvalue> Parser::e15(std::vector<Token *>::iterator &it,
      CodeBlock &context) {
	return e14(it, context);
}

std::unique_ptr<AST::rvalue> Parser::parseRvalue(std::vector<Token *>::iterator &it,
      CodeBlock &context) {
	return e15(it, context);
}
