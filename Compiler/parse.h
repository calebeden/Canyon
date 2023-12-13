#ifndef PARSE_H
#define PARSE_H

#include "ast.h"

#include <fcntl.h>

#define TAB_WIDTH 4

/**
 * @brief Parses the Canyon source code into an abstract syntax tree
 * @param program the source code to parse
 * @param source the name of the source code file
 */
AST::AST *tokenize(char *program, off_t size, char *source);

#endif
