#ifndef BUILTINS_H
#define BUILTINS_H

#include "ast.h"

struct Print : public AST::Function {
	explicit Print(AST::AST *ast);
	void compile(FILE *outfile, std::string name) const override;
	void forward(FILE *outfile, std::string name) const override;
};

#endif
