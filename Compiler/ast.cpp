#include "ast.h"

#include "tokens.h"

#include <cstring>
#include <stdarg.h>

namespace AST {

rvalue::rvalue(char *source, size_t row, size_t col)
    : source(source), row(row), col(col) {
}

void rvalue::error(const char *const format, ...) {
    fprintf(stderr, "Error at %s:%ld:%ld: ", source, row, col);
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fprintf(stderr, "\n");
    exit(EXIT_FAILURE);
}

Literal::Literal(Identifier *value) : rvalue(value->source, value->row, value->col) {
    char *buf = new char[value->s.len];
    strncpy(buf, value->s.start, value->s.len);
    this->value = atol(buf);
    delete buf;
}

void Literal::show() {
    fprintf(stderr, "%ld", value);
}

void Literal::compile(FILE *outfile) {
    fprintf(outfile, "%d", (int) value);
}

Variable::Variable(Identifier *variable)
    : rvalue(variable->source, variable->row, variable->col), variable(variable) {
}

void Variable::show() {
    fprintf(stderr, "Variable: ");
    variable->show();
    fprintf(stderr, "\n");
}

void Variable::compile(FILE *outfile) {
    variable->compile(outfile);
}

Assignment::Assignment(Identifier *variable, rvalue *expression)
    : rvalue(variable->source, variable->row, variable->col), variable(variable),
      expression(expression) {
}

void Assignment::show() {
    fprintf(stderr, "Assignment Statement: ");
    variable->show();
    fprintf(stderr, " = ");
    expression->show();
    fprintf(stderr, "\n");
}

void Assignment::compile(FILE *outfile) {
    fprintf(outfile, "%.*s = ", (int) variable->s.len, variable->s.start);
    expression->compile(outfile);
}

Addition::Addition(rvalue *operand1, rvalue *operand2)
    : rvalue(operand1->source, operand1->row, operand1->col), operand1(operand1),
      operand2(operand2) {
}

void Addition::show() {
    fprintf(stderr, "(");
    operand1->show();
    fprintf(stderr, " + ");
    operand2->show();
    fprintf(stderr, ")");
}

void Addition::compile(FILE *outfile) {
    fprintf(outfile, "(");
    operand1->compile(outfile);
    fprintf(outfile, "+");
    operand2->compile(outfile);
    fprintf(outfile, ")");
}

Subtraction::Subtraction(rvalue *operand1, rvalue *operand2)
    : rvalue(operand1->source, operand1->row, operand1->col), operand1(operand1),
      operand2(operand2) {
}

void Subtraction::show() {
    fprintf(stderr, "(");
    operand1->show();
    fprintf(stderr, " - ");
    operand2->show();
    fprintf(stderr, ")");
}

void Subtraction::compile(FILE *outfile) {
    fprintf(outfile, "(");
    operand1->compile(outfile);
    fprintf(outfile, "-");
    operand2->compile(outfile);
    fprintf(outfile, ")");
}

Multiplication::Multiplication(rvalue *operand1, rvalue *operand2)
    : rvalue(operand1->source, operand1->row, operand1->col), operand1(operand1),
      operand2(operand2) {
}

void Multiplication::show() {
    fprintf(stderr, "(");
    operand1->show();
    fprintf(stderr, " * ");
    operand2->show();
    fprintf(stderr, ")");
}

void Multiplication::compile(FILE *outfile) {
    fprintf(outfile, "(");
    operand1->compile(outfile);
    fprintf(outfile, "*");
    operand2->compile(outfile);
    fprintf(outfile, ")");
}

Division::Division(rvalue *operand1, rvalue *operand2)
    : rvalue(operand1->source, operand1->row, operand1->col), operand1(operand1),
      operand2(operand2) {
}

void Division::show() {
    fprintf(stderr, "(");
    operand1->show();
    fprintf(stderr, " / ");
    operand2->show();
    fprintf(stderr, ")");
}

void Division::compile(FILE *outfile) {
    fprintf(outfile, "(");
    operand1->compile(outfile);
    fprintf(outfile, "/");
    operand2->compile(outfile);
    fprintf(outfile, ")");
}

Modulo::Modulo(rvalue *operand1, rvalue *operand2)
    : rvalue(operand1->source, operand1->row, operand1->col), operand1(operand1),
      operand2(operand2) {
}

void Modulo::show() {
    fprintf(stderr, "(");
    operand1->show();
    fprintf(stderr, " %% ");
    operand2->show();
    fprintf(stderr, ")");
}

void Modulo::compile(FILE *outfile) {
    fprintf(outfile, "(");
    operand1->compile(outfile);
    fprintf(outfile, " %% ");
    operand2->compile(outfile);
    fprintf(outfile, ")");
}

Expression::Expression(rvalue *rval) : rval(rval) {
}

void Expression::show() {
    fprintf(stderr, "Expression Statement: ");
    rval->show();
    fprintf(stderr, "\n");
}

void Expression::compile(FILE *outfile) {
    rval->compile(outfile);
    fprintf(outfile, ";\n");
}

Print::Print(rvalue *expression)
    : rvalue(expression->source, expression->row, expression->col),
      expression(expression) {
}

void Print::show() {
    fprintf(stderr, "Print: ");
    expression->show();
    fprintf(stderr, "\n");
}

void Print::compile(FILE *outfile) {
    fprintf(outfile, "printf(\"%%d\\n\",");
    expression->compile(outfile);
    fprintf(outfile, ")");
}

FunctionCall::FunctionCall(Variable *name)
    : rvalue(name->source, name->row, name->col), name(name) {
}

void FunctionCall::show() {
    fprintf(stderr, "Function Call: ");
    name->show();
}

void FunctionCall::compile(FILE *outfile) {
    name->compile(outfile);
    fprintf(outfile, "()");
}

Return::Return(rvalue *rval) : rval(rval) {
}

void Return::show() {
    if (rval == nullptr) {
        fprintf(stderr, "Return (void)\n");
    } else {
        fprintf(stderr, "Return: ");
        rval->show();
        fprintf(stderr, "\n");
    }
}

void Return::compile(FILE *outfile) {
    if (rval == nullptr) {
        fprintf(outfile, "return;\n");
    } else {
        fprintf(outfile, "return ");
        rval->compile(outfile);
        fprintf(outfile, ";\n");
    }
}

CodeBlock::CodeBlock(AST *global)
    : locals(new std::unordered_map<Identifier *, Primitive *, Hasher, Comparator>),
      global(global) {
}

void CodeBlock::compile(FILE *outfile) {
    for (std::pair var : *locals) {
        Identifier *name = var.first;
        Primitive *type = var.second;
        fprintf(outfile, "    ");
        type->compile(outfile);
        fprintf(outfile, " ");
        name->compile(outfile);
        fprintf(outfile, ";\n");
    }
    for (Statement *s : statements) {
        fprintf(outfile, "    ");
        s->compile(outfile);
    }
}

void CodeBlock::defer(Variable *rval) {
    deferred.push_back(rval);
}

CodeBlock::IdentifierStatus CodeBlock::find(Variable *id) {
    if (locals->find(id->variable) != locals->end()) {
        return VARIABLE;
    }
    if (parent == nullptr) {
        if (global->functions.find(id->variable->s) != global->functions.end()) {
            return FUNCTION;
        }
        return UNKNOWN;
    }
    return parent->find(id);
}

void CodeBlock::resolve() {
    for (Variable *id : deferred) {
        IdentifierStatus status = find(id);
        switch (status) {
            case UNKNOWN: {
                id->error("Undeclared identifier %.*s", id->variable->s.len,
                      id->variable->s.start);
            }
            case VARIABLE:
            case FUNCTION: {
                // TODO check if it is appropriate given the invokation
                break;
            }
        }
    }
}

void Function::compile(FILE *outfile, std::string name) {
    Primitive::compile(outfile, type);
    fprintf(outfile, " %s(){\n", name.c_str());
    body->compile(outfile);
    fprintf(outfile, "}\n");
}

void Function::forward(FILE *outfile, std::string name) {
    Primitive::compile(outfile, type);
    fprintf(outfile, " %s();\n", name.c_str());
}

void AST::compile(FILE *outfile) {
    fprintf(outfile, "#include <stdio.h>\n");
    // Forward declarations
    for (std::pair<std::string, Function *> f : functions) {
        f.second->forward(outfile, f.first);
    }
    fprintf(outfile, "int main(int argc, char **argv) {\n"
                     "    canyonMain();\n"
                     "    return 0;\n"
                     "}\n");
    // Actual code
    for (std::pair<std::string, Function *> f : functions) {
        f.second->compile(outfile, f.first);
    }
}

void AST::resolve() {
    for (std::pair<std::string, Function *> f : functions) {
        f.second->body->resolve();
    }
}

} // namespace AST
