#include "ast.h"

#include <fcntl.h>

/**
 * @brief Parses the Canyon source code into an abstract syntax tree
 * @param program the source code to parse
 */
AST *tokenize(char *program, off_t size);
