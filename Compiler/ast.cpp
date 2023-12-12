#include "ast.h"

#include "tokens.h"

#include <cstring>

Literal::Literal(Identifier *value) {
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

Variable::Variable(Identifier *variable) : variable(variable) {
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

Print::Print(rvalue *expression) : expression(expression) {
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

CodeBlock::CodeBlock() : locals(new std::unordered_map<Identifier *, Primitive *>) {
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

void Function::compile(FILE *outfile) {
    body->compile(outfile);
}

void AST::compile(FILE *outfile) {
    for (Function *f : functions) {
        f->compile(outfile);
    }
}
