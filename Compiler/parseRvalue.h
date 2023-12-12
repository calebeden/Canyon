#ifndef PARSE_RVALUE_H
#define PARSE_RVALUE_H

#include "ast.h"

#include <cstdint>
#include <vector>

rvalue *parseRvalue(std::vector<Token *> tokens, size_t &i);

#endif
