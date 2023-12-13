#ifndef PARSE_RVALUE_H
#define PARSE_RVALUE_H

#include "ast.h"
#include "tokens.h"
#include <unordered_map>

#include <cstdint>
#include <vector>

rvalue *parseRvalue(std::vector<Token *> tokens, size_t &i,
      std::unordered_map<Identifier *, Primitive *, Hasher, Comparator> *locals);

#endif
