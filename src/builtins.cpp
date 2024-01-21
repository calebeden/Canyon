#include "builtins.h"

#include <string_view>

Print::Print(AST::AST *ast) : Function(ast) {
	type = Type::VOID;
	parameters.emplace_back(nullptr, Type::INT);
}

void Print::compile(std::ostream &outfile, [[maybe_unused]] std::string_view name) const {
	outfile << "void print(int x) {\n"
	           "    printf(\"%d\\n\", x);\n"
	           "}\n";
}

void Print::forward(std::ostream &outfile, [[maybe_unused]] std::string_view name) const {
	outfile << "void print(int x);\n";
}
