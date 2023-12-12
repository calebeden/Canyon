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

Addition::Addition(rvalue *operand1, rvalue *operand2)
    : operand1(operand1), operand2(operand2) {
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
    : operand1(operand1), operand2(operand2) {
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
    : operand1(operand1), operand2(operand2) {
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
    : operand1(operand1), operand2(operand2) {
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
    : operand1(operand1), operand2(operand2) {
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
