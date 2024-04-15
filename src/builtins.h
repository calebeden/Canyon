#ifndef BUILTINS_H
#define BUILTINS_H

#include "ast.h"
#include <string_view>

struct Print : public AST::Function {
	explicit Print(AST::Module &module);
	virtual void compile(std::ostream &outfile, std::string_view name) const override;
	virtual void forward(std::ostream &outfile, std::string_view name) const override;
	virtual ~Print() = default;
};

#endif
