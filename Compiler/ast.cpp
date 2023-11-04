#include "ast.h"

#include "tokens.h"

#include <cstring>

Literal::Literal(Identifier &value) {
    char *buf = new char[value.s.len];
    strncpy(buf, value.s.start, value.s.len);
    this->value = atol(buf);
    delete buf;
}

void Literal::show() {
    fprintf(stderr, "%ld", value);
}

void Literal::compile(FILE *outfile) {
    fprintf(outfile, "%d", (int) value);
}

Variable::Variable(Identifier &value) : s(value.s) {
}

void Variable::show() {
    fprintf(stderr, "Variable: ");
    s.show();
    fprintf(stderr, "\n");
}

void Variable::compile(FILE *outfile) {
    fprintf(outfile, "%.*s", (int) s.len, s.start);
}

Declaration::Declaration(Primitive *type, Variable *variable)
    : type(type), variable(variable) {
}

void Declaration::show() {
    fprintf(stderr, "Declaration: ");
    variable->show();
    fprintf(stderr, "\n");
}

void Declaration::compile(FILE *outfile) {
    type->compile(outfile);
    fprintf(outfile, " ");
    variable->compile(outfile);
    fprintf(outfile, ";\n");
}

Assignment::Assignment(Identifier *variable, Expression *expression)
    : variable(variable), expression(expression) {
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
    fprintf(outfile, ";\n");
}

Print::Print(Expression *expression) : expression(expression) {
}

void Print::show() {
    fprintf(stderr, "Print: ");
    expression->show();
    fprintf(stderr, "\n");
}

void Print::compile(FILE *outfile) {
    fprintf(outfile, "printf(\"%%d\\n\",");
    expression->compile(outfile);
    fprintf(outfile, ");\n");
}

void Return::show() {
    fprintf(stderr, "Return (void)\n");
}

void Return::compile(FILE *outfile) {
    fprintf(outfile, "return;\n");
}

void CodeBlock::compile(FILE *outfile) {
    for (Statement *s : statements) {
        fprintf(outfile, "    ");
        s->compile(outfile);
    }
}
