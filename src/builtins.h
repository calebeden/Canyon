#ifndef BUILTINS_H
#define BUILTINS_H

#include "ast.h"
#include <string_view>

struct Print : public AST::Function {
	explicit Print(AST::AST *ast);
	void compile(std::ostream &outfile, const std::string_view &name) const override;
	void forward(std::ostream &outfile, const std::string_view &name) const override;
};

#endif
