#include "parser.h"

#include "ast.h"
#include "tokens.h"
#include <unordered_map>

#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <stddef.h>
#include <stdexcept>
#include <stdlib.h>
#include <typeinfo>
#include <vector>

using namespace AST;

Parser::Parser() {
}

AST::AST *Parser::parseModule(std::vector<Token *> *tokens) {
    AST::AST *ast = new AST::AST();
    parseFunctions(tokens, ast);
    if (ast->functions.find("canyonMain") == ast->functions.end()) {
        fprintf(stderr, "Parse error: no main function\n");
        exit(EXIT_FAILURE);
    }
    ast->resolve();
    return ast;
}

void Parser::parseFunctions(std::vector<Token *> *tokens, AST::AST *ast) {
    std::vector<Token *>::iterator it = tokens->begin();
    while (it < tokens->end()) {
        parseFunction(it, ast);
    }
}

void Parser::parseFunction(std::vector<Token *>::iterator &it, AST::AST *ast) {
    Type type;
    if (typeid(**it) == typeid(Primitive)) {
        type = static_cast<Primitive *>(*it)->type;
    } else if (typeid(**it) == typeid(Keyword)
               && static_cast<Keyword *>(*it)->type == Keyword::Type::VOID) {
        type = Type::VOID;
    } else {
        (*it)->error("Expected function type");
    }
    it++;
    if (typeid(**it) != typeid(Identifier)) {
        (*it)->error("Expected identifier");
    }
    std::string name = static_cast<Identifier *>(*it)->s;
    if (name == "main") {
        name = "canyonMain";
    }
    it++;
    Function *function = new Function(ast);
    parseParameters(it, function);
    if (typeid(**it) != typeid(Punctuation)
          && static_cast<Punctuation *>(*it)->type != Punctuation::Type::OpenBrace) {
        (*it)->error("Expected '{'");
    }
    it++;

    parseBlock(it, function->body);

    if (typeid(**it) != typeid(Punctuation)
          && static_cast<Punctuation *>(*it)->type != Punctuation::Type::CloseBrace) {
        (*it)->error("Expected '}'");
    }
    it++;

    for (Statement *s : function->body->statements) {
        s->show();
    }
    function->type = type;
    ast->functions[name] = function;
}

void Parser::parseParameters(std::vector<Token *>::iterator &it, Function *function) {
    if (typeid(**it) != typeid(Punctuation)
          && static_cast<Punctuation *>(*it)->type != Punctuation::Type::OpenParen) {
        (*it)->error("Expected '('");
    }
    it++;

    CodeBlock *context = function->body;
    while (true) {
        if (typeid(**it) == typeid(Primitive)) {
            Primitive *type = static_cast<Primitive *>(*it);
            it++;
            if (typeid(**it) == typeid(Identifier)) {
                Identifier *id = static_cast<Identifier *>(*it);
                it++;
                if (context->locals->find(id) != context->locals->end()) {
                    (*it)->error("Re-declaration of parameter %.*s", id->s.len,
                          id->s.start);
                }
                context->locals->insert({
                      id, {type->type, true}
                });
                function->parameters.push_back({id, type->type});
            } else {
                (*it)->error("Unexpected token following primitive");
            }
        }
        if (typeid(**it) == typeid(Punctuation)) {
            if (static_cast<Punctuation *>(*it)->type == Punctuation::Type::CloseParen) {
                it++;
                return;
            } else if (static_cast<Punctuation *>(*it)->type
                       == Punctuation::Type::Comma) {
                it++;
                continue;
            }
        }
        (*it)->error("Unexpected token; expected ')' or ','");
    }

    if (typeid(**it) != typeid(Punctuation)
          && static_cast<Punctuation *>(*it)->type != Punctuation::Type::CloseParen) {
        (*it)->error("Expected ')'");
    }
    it++;
}

void Parser::parseBlock(std::vector<Token *>::iterator &it, CodeBlock *context) {
    while (typeid(**it) != typeid(Punctuation)
           || static_cast<Punctuation *>(*it)->type != Punctuation::Type::CloseBrace) {
        Statement *s = parseStatement(it, context);
        if (s != nullptr) {
            context->statements.push_back(s);
        }
    }
}

Statement *Parser::parseStatement(std::vector<Token *>::iterator &it,
      CodeBlock *context) {
    while (typeid(**it) == typeid(Punctuation)
           && static_cast<Punctuation *>(*it)->type == Punctuation::Type::Semicolon) {
        it++;
    }
    if (typeid(**it) == typeid(Punctuation)
          && static_cast<Punctuation *>(*it)->type == Punctuation::Type::CloseBrace) {
        return nullptr;
    }

    rvalue *rval;
    if (typeid(**it) == typeid(Primitive)) {
        Primitive *type = static_cast<Primitive *>(*it);
        it++;
        if (typeid(**it) == typeid(Identifier)) {
            Identifier *id = static_cast<Identifier *>(*it);
            if (context->locals->find(id) != context->locals->end()) {
                (*it)->error("Re-declaration of variable %.*s", id->s.len, id->s.start);
            }
            context->locals->insert({
                  id, {type->type, false}
            });
            std::vector<Token *>::iterator it2 = it;
            rval = parseRvalue(it, context);
            if (rval == nullptr) {
                fprintf(stderr, "HELP\n");
                exit(EXIT_FAILURE);
            }
            if (typeid(*rval) != typeid(Assignment)) {
                it = it2 + 1;
                return nullptr;
            }
        } else {
            (*it)->error("Unexpected token following primitive");
        }
    } else if (typeid(**it) == typeid(Keyword)
               && static_cast<Keyword *>(*it)->type == Keyword::Type::RETURN) {
        std::vector<Token *>::iterator return_it = it;
        it++;
        rval = parseRvalue(it, context);
        if (typeid(**it) != typeid(Punctuation)
              || static_cast<Punctuation *>(*it)->type != Punctuation::Type::Semicolon) {
            if (static_cast<Punctuation *>(*it)->type == Punctuation::Type::Equals) {
                (*it)->error("LHS of assignment is not a variable");
            }
            (*it)->error("Expected ';' after statement");
        }
        Return *ret = new Return(rval, *return_it);
        return ret;
    } else {
        rval = parseRvalue(it, context);
    }

    if (typeid(**it) != typeid(Punctuation)
          || static_cast<Punctuation *>(*it)->type != Punctuation::Type::Semicolon) {
        if (static_cast<Punctuation *>(*it)->type == Punctuation::Type::Equals) {
            (*it)->error("LHS of assignment is not a variable");
        }
        (*it)->error("Expected ';' after statement");
    }
    if (rval != nullptr) {
        return new Expression(rval);
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

AST::rvalue *Parser::e0(std::vector<Token *>::iterator &it, CodeBlock *context) {
    if (typeid(**it) == typeid(Identifier)) {
        Identifier *id = static_cast<Identifier *>(*it);
        it++;
        bool isInt = true;
        for (size_t i = 0; i < id->s.len; i++) {
            if (!isdigit(id->s.start[i])) {
                isInt = false;
                break;
            }
        }
        if (isInt) {
            return new Literal(id);
        } else {
            return new Variable(id);
        }
    }
    return nullptr;
}

AST::rvalue *Parser::e1(std::vector<Token *>::iterator &it, CodeBlock *context) {
    AST::rvalue *temp = e0(it, context);

    if (typeid(**it) == typeid(Punctuation)
          && static_cast<Punctuation *>(*it)->type == Punctuation::Type::OpenParen) {
        it++;
        AST::rvalue *rval = parseRvalue(it, context);
        if (temp != nullptr && typeid(*temp) == typeid(Variable)) {
            Variable *id = static_cast<Variable *>(temp);
            CodeBlock::IdentifierStatus status = context->find(id);
            switch (status) {
                case CodeBlock::IdentifierStatus::VARIABLE: {
                    id->error("%s is not callable", id->variable->s);
                    break;
                }
                case CodeBlock::IdentifierStatus::UNKNOWN: {
                    context->defer(id);
                }
                case CodeBlock::IdentifierStatus::FUNCTION: {
                    break;
                }
            }
            FunctionCall *call = new FunctionCall(id);
            while (rval) {
                call->arguments.push_back(rval);
                if (typeid(**it) == typeid(Punctuation)
                      && static_cast<Punctuation *>(*it)->type
                               == Punctuation::Type::Comma) {
                    it++;
                    rval = parseRvalue(it, context);
                    if (rval == nullptr) {
                        (*it)->error("Expected expression");
                    }
                } else {
                    rval = nullptr;
                }
            }
            context->global->functionCalls.push_back(call);
            rval = call;
        }
        if (rval == nullptr) {
            (*it)->error("Expected expression");
        }
        if (typeid(**it) != typeid(Punctuation)
              || static_cast<Punctuation *>(*it)->type != Punctuation::Type::CloseParen) {
            (*it)->error("Expected ')'");
        }
        it++;
        return rval;
    }

    if (temp != nullptr && typeid(*temp) == typeid(Variable)) {
        Variable *id = static_cast<Variable *>(temp);
        CodeBlock::IdentifierStatus status = context->find(id);
        switch (status) {
            case CodeBlock::IdentifierStatus::FUNCTION: {
                id->error("%s is not a variable", id->variable->s);
                break;
            }
            case CodeBlock::IdentifierStatus::UNKNOWN: {
                context->defer(id);
            }
            case CodeBlock::IdentifierStatus::VARIABLE: {
                break;
            }
        }
    }
    return temp;
}

AST::rvalue *Parser::e2(std::vector<Token *>::iterator &it, CodeBlock *context) {
    return e1(it, context);
}

AST::rvalue *Parser::e3(std::vector<Token *>::iterator &it, CodeBlock *context) {
    AST::rvalue *operand1 = e2(it, context);
    while (true) {
        if (typeid(**it) == typeid(Punctuation)) {
            if (static_cast<Punctuation *>(*it)->type == Punctuation::Type::Times) {
                it++;
                operand1 = new Multiplication(operand1, e2(it, context));
            } else if (static_cast<Punctuation *>(*it)->type
                       == Punctuation::Type::Divide) {
                it++;
                operand1 = new Division(operand1, e2(it, context));
            } else if (static_cast<Punctuation *>(*it)->type == Punctuation::Type::Mod) {
                it++;
                operand1 = new Modulo(operand1, e2(it, context));
            } else {
                return operand1;
            }
        } else {
            return operand1;
        }
    }
    return e2(it, context);
}

AST::rvalue *Parser::e4(std::vector<Token *>::iterator &it, CodeBlock *context) {
    AST::rvalue *operand1 = e3(it, context);
    while (true) {
        if (typeid(**it) == typeid(Punctuation)) {
            if (static_cast<Punctuation *>(*it)->type == Punctuation::Type::Plus) {
                it++;
                operand1 = new Addition(operand1, e3(it, context));
            } else if (static_cast<Punctuation *>(*it)->type
                       == Punctuation::Type::Minus) {
                it++;
                operand1 = new Subtraction(operand1, e3(it, context));
            } else {
                return operand1;
            }
        } else {
            return operand1;
        }
    }
    return e3(it, context);
}

AST::rvalue *Parser::e5(std::vector<Token *>::iterator &it, CodeBlock *context) {
    return e4(it, context);
}

AST::rvalue *Parser::e6(std::vector<Token *>::iterator &it, CodeBlock *context) {
    return e5(it, context);
}

AST::rvalue *Parser::e7(std::vector<Token *>::iterator &it, CodeBlock *context) {
    return e6(it, context);
}

AST::rvalue *Parser::e8(std::vector<Token *>::iterator &it, CodeBlock *context) {
    return e7(it, context);
}

AST::rvalue *Parser::e9(std::vector<Token *>::iterator &it, CodeBlock *context) {
    return e8(it, context);
}

AST::rvalue *Parser::e10(std::vector<Token *>::iterator &it, CodeBlock *context) {
    return e9(it, context);
}

AST::rvalue *Parser::e11(std::vector<Token *>::iterator &it, CodeBlock *context) {
    return e10(it, context);
}

AST::rvalue *Parser::e12(std::vector<Token *>::iterator &it, CodeBlock *context) {
    return e11(it, context);
}

AST::rvalue *Parser::e13(std::vector<Token *>::iterator &it, CodeBlock *context) {
    return e12(it, context);
}

AST::rvalue *Parser::e14(std::vector<Token *>::iterator &it, CodeBlock *context) {
    if (typeid(**it) == typeid(Identifier)) {
        Identifier *id = static_cast<Identifier *>(*it);
        if (typeid(**(it + 1)) == typeid(Punctuation)
              && static_cast<Punctuation *>(*(it + 1))->type
                       == Punctuation::Type::Equals) {
            if (context->locals->find(id) == context->locals->end()) {
                id->error("Undeclared variable %.*s", id->s.len, id->s.start);
            }
            it += 2;
            return new Assignment(id, e14(it, context));
        } else {
            return e13(it, context);
        }
    }
    return e13(it, context);
}

AST::rvalue *Parser::e15(std::vector<Token *>::iterator &it, CodeBlock *context) {
    return e14(it, context);
}

rvalue *Parser::parseRvalue(std::vector<Token *>::iterator &it, CodeBlock *context) {
    return e15(it, context);
}
