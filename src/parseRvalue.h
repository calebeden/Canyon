#ifndef PARSE_RVALUE_H
#define PARSE_RVALUE_H

#include "ast.h"
#include "tokens.h"
#include <unordered_map>

#include <cstdint>
#include <vector>

/**
 * @brief Parses entire rvalues, taking operator precedence into consideration
 *
 * @param it a reference to the iterator of Tokens to use. Expected to point to the first
 * token of the expression when this function is called. When this function returns it
 * will point to the token IMMEDIATELY AFTER the rvalue
 * @param context the CodeBlock in which the current expression occurs
 * @return the parsed rvalue
 */
AST::rvalue *parseRvalue(std::vector<Token *>::iterator &it, AST::CodeBlock *context);

#endif
