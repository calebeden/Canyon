#ifndef BUILTINS_H
#define BUILTINS_H

#include "ast.h"

struct Print : public AST::Function {
	Print(AST::AST *ast);
	void compile(FILE *outfile, std::string name) override;
	void forward(FILE *outfile, std::string name) override;
};

#endif
