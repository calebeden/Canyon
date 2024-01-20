#include "builtins.h"

Print::Print(AST::AST *ast) : Function(ast) {
	type = Type::VOID;
	parameters.emplace_back(nullptr, Type::INT);
}

void Print::compile(FILE *outfile, [[maybe_unused]] const std::string &name) const {
	fprintf(outfile, "void print(int x) {\n"
	                 "    printf(\"%%d\\n\", x);\n"
	                 "}\n");
}

void Print::forward(FILE *outfile, [[maybe_unused]] const std::string &name) const {
	fprintf(outfile, "void print(int x);\n");
}
