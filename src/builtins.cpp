#include "builtins.h"

Print::Print(AST::AST *ast) : Function(ast) {
    type = Type::VOID;
    parameters.push_back({nullptr, Type::INT});
}

void Print::compile(FILE *outfile, [[maybe_unused]] std::string name) {
    fprintf(outfile, "void print(int x) {\n"
                     "    printf(\"%%d\\n\", x);\n"
                     "}\n");
}

void Print::forward(FILE *outfile, [[maybe_unused]] std::string name) {
    fprintf(outfile, "void print(int x);\n");
}