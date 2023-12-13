#include "parseRvalue.h"

#include "ast.h"
#include "tokens.h"
#include <unordered_map>

#include <vector>

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
static rvalue *e0(std::vector<Token *> tokens, size_t &i,
      std::unordered_map<Identifier *, Primitive *, Hasher, Comparator> *locals) {
    if (typeid(*tokens[i]) == typeid(Identifier)) {
        Identifier *id = static_cast<Identifier *>(tokens[i]);
        i++;
        if (id->s == "print") {
            rvalue *toPrint = parseRvalue(tokens, i, locals);
            if (toPrint != nullptr) {
                return new Print(toPrint);
            } else {
                tokens[i - 1]->parse_error("No expression to print");
            }
        } else {
            bool isInt;
            for (size_t i = 0; i < id->s.len; i++) {
                if (!isdigit(id->s.start[i])) {
                    isInt = false;
                    break;
                }
            }
            if (isInt) {
                return new Literal(id);
            } else {
                if (locals->find(id) == locals->end()) {
                    id->parse_error("Undeclared variable %.*s", id->s.len, id->s.start);
                }
                return new Variable(id);
            }
        }
    }
    return nullptr;
}

/**
 * @brief Grouping by ()
 *
 * @param tokens the vector of tokens to parse from
 * @param i a reference to the index in the vector corresponding to the first token of the
 * expression. By the end of the function it will contain the index corresponding to the
 * token IMMEDIATELY AFTER the rvalue
 * @return the parsed rvalue
 */
static rvalue *e1(std::vector<Token *> tokens, size_t &i,
      std::unordered_map<Identifier *, Primitive *, Hasher, Comparator> *locals) {
    if (typeid(*tokens[i]) == typeid(Punctuation)
          && static_cast<Punctuation *>(tokens[i])->type
                   == Punctuation::Type::OpenParen) {
        i++;
        rvalue *rval = parseRvalue(tokens, i, locals);
        if (rval == nullptr) {
            tokens[i]->parse_error("Expected expression");
        }
        if (typeid(*tokens[i]) != typeid(Punctuation)
              || static_cast<Punctuation *>(tokens[i])->type
                       != Punctuation::Type::CloseParen) {
            tokens[i]->parse_error("Expected ')'");
        }
        i++;
        return rval;
    }
    return e0(tokens, i, locals);
}

static rvalue *e2(std::vector<Token *> tokens, size_t &i,
      std::unordered_map<Identifier *, Primitive *, Hasher, Comparator> *locals) {
    return e1(tokens, i, locals);
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
static rvalue *e3(std::vector<Token *> tokens, size_t &i,
      std::unordered_map<Identifier *, Primitive *, Hasher, Comparator> *locals) {
    rvalue *operand1 = e2(tokens, i, locals);
    while (true) {
        if (typeid(*tokens[i]) == typeid(Punctuation)) {
            if (static_cast<Punctuation *>(tokens[i])->type == Punctuation::Type::Times) {
                i++;
                operand1 = new Multiplication(operand1, e2(tokens, i, locals));
            } else if (static_cast<Punctuation *>(tokens[i])->type
                       == Punctuation::Type::Divide) {
                i++;
                operand1 = new Division(operand1, e2(tokens, i, locals));
            } else if (static_cast<Punctuation *>(tokens[i])->type
                       == Punctuation::Type::Mod) {
                i++;
                operand1 = new Modulo(operand1, e2(tokens, i, locals));
            } else {
                return operand1;
            }
        } else {
            return operand1;
        }
    }
    return e2(tokens, i, locals);
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
static rvalue *e4(std::vector<Token *> tokens, size_t &i,
      std::unordered_map<Identifier *, Primitive *, Hasher, Comparator> *locals) {
    rvalue *operand1 = e3(tokens, i, locals);
    while (true) {
        if (typeid(*tokens[i]) == typeid(Punctuation)) {
            if (static_cast<Punctuation *>(tokens[i])->type == Punctuation::Type::Plus) {
                i++;
                operand1 = new Addition(operand1, e3(tokens, i, locals));
            } else if (static_cast<Punctuation *>(tokens[i])->type
                       == Punctuation::Type::Minus) {
                i++;
                operand1 = new Subtraction(operand1, e3(tokens, i, locals));
            } else {
                return operand1;
            }
        } else {
            return operand1;
        }
    }
    return e3(tokens, i, locals);
}

static rvalue *e5(std::vector<Token *> tokens, size_t &i,
      std::unordered_map<Identifier *, Primitive *, Hasher, Comparator> *locals) {
    return e4(tokens, i, locals);
}

static rvalue *e6(std::vector<Token *> tokens, size_t &i,
      std::unordered_map<Identifier *, Primitive *, Hasher, Comparator> *locals) {
    return e5(tokens, i, locals);
}

static rvalue *e7(std::vector<Token *> tokens, size_t &i,
      std::unordered_map<Identifier *, Primitive *, Hasher, Comparator> *locals) {
    return e6(tokens, i, locals);
}

static rvalue *e8(std::vector<Token *> tokens, size_t &i,
      std::unordered_map<Identifier *, Primitive *, Hasher, Comparator> *locals) {
    return e7(tokens, i, locals);
}

static rvalue *e9(std::vector<Token *> tokens, size_t &i,
      std::unordered_map<Identifier *, Primitive *, Hasher, Comparator> *locals) {
    return e8(tokens, i, locals);
}

static rvalue *e10(std::vector<Token *> tokens, size_t &i,
      std::unordered_map<Identifier *, Primitive *, Hasher, Comparator> *locals) {
    return e9(tokens, i, locals);
}

static rvalue *e11(std::vector<Token *> tokens, size_t &i,
      std::unordered_map<Identifier *, Primitive *, Hasher, Comparator> *locals) {
    return e10(tokens, i, locals);
}

static rvalue *e12(std::vector<Token *> tokens, size_t &i,
      std::unordered_map<Identifier *, Primitive *, Hasher, Comparator> *locals) {
    return e11(tokens, i, locals);
}

static rvalue *e13(std::vector<Token *> tokens, size_t &i,
      std::unordered_map<Identifier *, Primitive *, Hasher, Comparator> *locals) {
    return e12(tokens, i, locals);
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
static rvalue *e14(std::vector<Token *> tokens, size_t &i,
      std::unordered_map<Identifier *, Primitive *, Hasher, Comparator> *locals) {
    if (typeid(*tokens[i]) == typeid(Identifier)) {
        Identifier *id = static_cast<Identifier *>(tokens[i]);
        if (typeid(*tokens[i + 1]) == typeid(Punctuation)
              && static_cast<Punctuation *>(tokens[i + 1])->type
                       == Punctuation::Type::Equals) {
            if (locals->find(id) == locals->end()) {
                id->parse_error("Undeclared variable %.*s", id->s.len, id->s.start);
            }
            i += 2;
            return new Assignment(id, e14(tokens, i, locals));
        } else {
            return e13(tokens, i, locals);
        }
    }
    return e13(tokens, i, locals);
}

static rvalue *e15(std::vector<Token *> tokens, size_t &i,
      std::unordered_map<Identifier *, Primitive *, Hasher, Comparator> *locals) {
    return e14(tokens, i, locals);
}

rvalue *parseRvalue(std::vector<Token *> tokens, size_t &i,
      std::unordered_map<Identifier *, Primitive *, Hasher, Comparator> *locals) {
    return e15(tokens, i, locals);
}