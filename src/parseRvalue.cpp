#include "parseRvalue.h"

#include "ast.h"
#include "tokens.h"
#include <unordered_map>

#include <vector>

using namespace AST;

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

/**
 * @brief Deeper than any operators - identifiers and literals
 *
 * @param it a reference to the iterator of Tokens to use. Expected to point to the first
 * token of the expression when this function is called. When this function returns it
 * will point to the token IMMEDIATELY AFTER the rvalue
 * @param context the CodeBlock in which the current expression occurs
 * @return the parsed rvalue
 */
static rvalue *e0(std::vector<Token *>::iterator &it, CodeBlock *context) {
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

/**
 * @brief () - grouping for precedence and function calls
 *
 * @param it a reference to the iterator of Tokens to use. Expected to point to the first
 * token of the expression when this function is called. When this function returns it
 * will point to the token IMMEDIATELY AFTER the rvalue
 * @param context the CodeBlock in which the current expression occurs
 * @return the parsed rvalue
 */
static rvalue *e1(std::vector<Token *>::iterator &it, CodeBlock *context) {
    rvalue *temp = e0(it, context);

    if (typeid(**it) == typeid(Punctuation)
          && static_cast<Punctuation *>(*it)->type == Punctuation::Type::OpenParen) {
        it++;
        rvalue *rval = parseRvalue(it, context);
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

static rvalue *e2(std::vector<Token *>::iterator &it, CodeBlock *context) {
    return e1(it, context);
}

/**
 * @brief Multiplication, division, modulo operators * / % (LTR)
 *
 * @param it a reference to the iterator of Tokens to use. Expected to point to the first
 * token of the expression when this function is called. When this function returns it
 * will point to the token IMMEDIATELY AFTER the rvalue
 * @param context the CodeBlock in which the current expression occurs
 * @return the parsed rvalue
 */
static rvalue *e3(std::vector<Token *>::iterator &it, CodeBlock *context) {
    rvalue *operand1 = e2(it, context);
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

/**
 * @brief Addition and subtraction operators + - (LTR)
 *
 * @param it a reference to the iterator of Tokens to use. Expected to point to the first
 * token of the expression when this function is called. When this function returns it
 * will point to the token IMMEDIATELY AFTER the rvalue
 * @param context the CodeBlock in which the current expression occurs
 * @return the parsed rvalue
 */
static rvalue *e4(std::vector<Token *>::iterator &it, CodeBlock *context) {
    rvalue *operand1 = e3(it, context);
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

static rvalue *e5(std::vector<Token *>::iterator &it, CodeBlock *context) {
    return e4(it, context);
}

static rvalue *e6(std::vector<Token *>::iterator &it, CodeBlock *context) {
    return e5(it, context);
}

static rvalue *e7(std::vector<Token *>::iterator &it, CodeBlock *context) {
    return e6(it, context);
}

static rvalue *e8(std::vector<Token *>::iterator &it, CodeBlock *context) {
    return e7(it, context);
}

static rvalue *e9(std::vector<Token *>::iterator &it, CodeBlock *context) {
    return e8(it, context);
}

static rvalue *e10(std::vector<Token *>::iterator &it, CodeBlock *context) {
    return e9(it, context);
}

static rvalue *e11(std::vector<Token *>::iterator &it, CodeBlock *context) {
    return e10(it, context);
}

static rvalue *e12(std::vector<Token *>::iterator &it, CodeBlock *context) {
    return e11(it, context);
}

static rvalue *e13(std::vector<Token *>::iterator &it, CodeBlock *context) {
    return e12(it, context);
}

/**
 * @brief Assignment operators = += -= *= /= %= <<= >>= &= ^= |= (RTL)
 *
 * @param it a reference to the iterator of Tokens to use. Expected to point to the first
 * token of the expression when this function is called. When this function returns it
 * will point to the token IMMEDIATELY AFTER the rvalue
 * @param context the CodeBlock in which the current expression occurs
 * @return the parsed rvalue
 */
static rvalue *e14(std::vector<Token *>::iterator &it, CodeBlock *context) {
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

static rvalue *e15(std::vector<Token *>::iterator &it, CodeBlock *context) {
    return e14(it, context);
}

rvalue *parseRvalue(std::vector<Token *>::iterator &it, CodeBlock *context) {
    return e15(it, context);
}
