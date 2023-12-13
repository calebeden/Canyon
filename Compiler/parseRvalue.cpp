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
 * @param tokens
 * @param i
 * @return rvalue*
 */
static rvalue *e0(std::vector<Token *> tokens, size_t &i, CodeBlock *context) {
    if (typeid(*tokens[i]) == typeid(Identifier)) {
        Identifier *id = static_cast<Identifier *>(tokens[i]);
        i++;
        if (id->s == "print") {
            rvalue *toPrint = parseRvalue(tokens, i, context);
            if (toPrint != nullptr) {
                return new Print(toPrint);
            } else {
                tokens[i - 1]->error("No expression to print");
            }
        } else {
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
    }
    return nullptr;
}

/**
 * @brief () - grouping for precedence and function calls
 *
 * @param tokens the vector of tokens to parse from
 * @param i a reference to the index in the vector corresponding to the first token of the
 * expression. By the end of the function it will contain the index corresponding to the
 * token IMMEDIATELY AFTER the rvalue
 * @return the parsed rvalue
 */
static rvalue *e1(std::vector<Token *> tokens, size_t &i, CodeBlock *context) {
    rvalue *temp = e0(tokens, i, context);

    if (typeid(*tokens[i]) == typeid(Punctuation)
          && static_cast<Punctuation *>(tokens[i])->type
                   == Punctuation::Type::OpenParen) {
        i++;
        rvalue *rval = parseRvalue(tokens, i, context);
        if (temp != nullptr && typeid(*temp) == typeid(Variable)) {
            if (rval != nullptr) {
                fprintf(stderr, "Can only support void arguments for now\n");
                exit(EXIT_FAILURE);
            }
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
            rval = new FunctionCall(id);
        }
        if (rval == nullptr) {
            tokens[i]->error("Expected expression");
        }
        if (typeid(*tokens[i]) != typeid(Punctuation)
              || static_cast<Punctuation *>(tokens[i])->type
                       != Punctuation::Type::CloseParen) {
            tokens[i]->error("Expected ')'");
        }
        i++;

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

static rvalue *e2(std::vector<Token *> tokens, size_t &i, CodeBlock *context) {
    return e1(tokens, i, context);
}

/**
 * @brief Multiplication, division, modulo operators * / % (LTR)
 *
 * @param tokens the vector of tokens to parse from
 * @param i a reference to the index in the vector corresponding to the first token of the
 * expression. By the end of the function it will contain the index corresponding to the
 * token IMMEDIATELY AFTER the rvalue
 * @return the parsed rvalue
 */
static rvalue *e3(std::vector<Token *> tokens, size_t &i, CodeBlock *context) {
    rvalue *operand1 = e2(tokens, i, context);
    while (true) {
        if (typeid(*tokens[i]) == typeid(Punctuation)) {
            if (static_cast<Punctuation *>(tokens[i])->type == Punctuation::Type::Times) {
                i++;
                operand1 = new Multiplication(operand1, e2(tokens, i, context));
            } else if (static_cast<Punctuation *>(tokens[i])->type
                       == Punctuation::Type::Divide) {
                i++;
                operand1 = new Division(operand1, e2(tokens, i, context));
            } else if (static_cast<Punctuation *>(tokens[i])->type
                       == Punctuation::Type::Mod) {
                i++;
                operand1 = new Modulo(operand1, e2(tokens, i, context));
            } else {
                return operand1;
            }
        } else {
            return operand1;
        }
    }
    return e2(tokens, i, context);
}

/**
 * @brief Addition and subtraction operators + - (LTR)
 *
 * @param tokens the vector of tokens to parse from
 * @param i a reference to the index in the vector corresponding to the first token of the
 * expression. By the end of the function it will contain the index corresponding to the
 * token IMMEDIATELY AFTER the rvalue
 * @return the parsed rvalue
 */
static rvalue *e4(std::vector<Token *> tokens, size_t &i, CodeBlock *context) {
    rvalue *operand1 = e3(tokens, i, context);
    while (true) {
        if (typeid(*tokens[i]) == typeid(Punctuation)) {
            if (static_cast<Punctuation *>(tokens[i])->type == Punctuation::Type::Plus) {
                i++;
                operand1 = new Addition(operand1, e3(tokens, i, context));
            } else if (static_cast<Punctuation *>(tokens[i])->type
                       == Punctuation::Type::Minus) {
                i++;
                operand1 = new Subtraction(operand1, e3(tokens, i, context));
            } else {
                return operand1;
            }
        } else {
            return operand1;
        }
    }
    return e3(tokens, i, context);
}

static rvalue *e5(std::vector<Token *> tokens, size_t &i, CodeBlock *context) {
    return e4(tokens, i, context);
}

static rvalue *e6(std::vector<Token *> tokens, size_t &i, CodeBlock *context) {
    return e5(tokens, i, context);
}

static rvalue *e7(std::vector<Token *> tokens, size_t &i, CodeBlock *context) {
    return e6(tokens, i, context);
}

static rvalue *e8(std::vector<Token *> tokens, size_t &i, CodeBlock *context) {
    return e7(tokens, i, context);
}

static rvalue *e9(std::vector<Token *> tokens, size_t &i, CodeBlock *context) {
    return e8(tokens, i, context);
}

static rvalue *e10(std::vector<Token *> tokens, size_t &i, CodeBlock *context) {
    return e9(tokens, i, context);
}

static rvalue *e11(std::vector<Token *> tokens, size_t &i, CodeBlock *context) {
    return e10(tokens, i, context);
}

static rvalue *e12(std::vector<Token *> tokens, size_t &i, CodeBlock *context) {
    return e11(tokens, i, context);
}

static rvalue *e13(std::vector<Token *> tokens, size_t &i, CodeBlock *context) {
    return e12(tokens, i, context);
}

/**
 * @brief Assignment operators = += -= *= /= %= <<= >>= &= ^= |= (RTL)
 *
 * @param tokens the vector of tokens to parse from
 * @param i a reference to the index in the vector corresponding to the first token of the
 * expression. By the end of the function it will contain the index corresponding to the
 * token IMMEDIATELY AFTER the rvalue
 * @return the parsed rvalue
 */
static rvalue *e14(std::vector<Token *> tokens, size_t &i, CodeBlock *context) {
    if (typeid(*tokens[i]) == typeid(Identifier)) {
        Identifier *id = static_cast<Identifier *>(tokens[i]);
        if (typeid(*tokens[i + 1]) == typeid(Punctuation)
              && static_cast<Punctuation *>(tokens[i + 1])->type
                       == Punctuation::Type::Equals) {
            if (context->locals->find(id) == context->locals->end()) {
                id->error("Undeclared variable %.*s", id->s.len, id->s.start);
            }
            i += 2;
            return new Assignment(id, e14(tokens, i, context));
        } else {
            return e13(tokens, i, context);
        }
    }
    return e13(tokens, i, context);
}

static rvalue *e15(std::vector<Token *> tokens, size_t &i, CodeBlock *context) {
    return e14(tokens, i, context);
}

rvalue *parseRvalue(std::vector<Token *> tokens, size_t &i, CodeBlock *context) {
    return e15(tokens, i, context);
}
