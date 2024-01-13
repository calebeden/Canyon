#include "ast.h"

#include "builtins.h"
#include "tokens.h"

#include <cstring>
#include <stdarg.h>

namespace AST {

rvalue::rvalue(const char *const source, size_t row, size_t col)
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
    this->value = atoi(buf);
    delete buf;
}

void Literal::show() {
    fprintf(stderr, "%d", value);
}

void Literal::compile(FILE *outfile) {
    fprintf(outfile, "%d", value);
}

Type Literal::typeCheck([[maybe_unused]] CodeBlock *context) {
    return Type::INT;
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

Type Variable::typeCheck(CodeBlock *context) {
    return context->getType(variable);
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
    fprintf(outfile, "%.*s=", (int) variable->s.len, variable->s.start);
    expression->compile(outfile);
}

Type Assignment::typeCheck(CodeBlock *context) {
    Type varType = context->getType(variable);
    Type expType = expression->typeCheck(context);
    if (varType != expType) {
        error("Incompatible types");
    }
    return expType;
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

Type Addition::typeCheck(CodeBlock *context) {
    Type type1 = operand1->typeCheck(context);
    Type type2 = operand2->typeCheck(context);
    if (type1 != type2) {
        error("Incompatible types");
    }
    return type2;
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

Type Subtraction::typeCheck(CodeBlock *context) {
    Type type1 = operand1->typeCheck(context);
    Type type2 = operand2->typeCheck(context);
    if (type1 != type2) {
        error("Incompatible types");
    }
    return type2;
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

Type Multiplication::typeCheck(CodeBlock *context) {
    Type type1 = operand1->typeCheck(context);
    Type type2 = operand2->typeCheck(context);
    if (type1 != type2) {
        error("Incompatible types");
    }
    return type2;
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

Type Division::typeCheck(CodeBlock *context) {
    Type type1 = operand1->typeCheck(context);
    Type type2 = operand2->typeCheck(context);
    if (type1 != type2) {
        error("Incompatible types");
    }
    return type2;
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
    fprintf(outfile, "%%");
    operand2->compile(outfile);
    fprintf(outfile, ")");
}

Type Modulo::typeCheck(CodeBlock *context) {
    Type type1 = operand1->typeCheck(context);
    Type type2 = operand2->typeCheck(context);
    if (type1 != type2) {
        error("Incompatible types");
    }
    return type2;
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

Type Expression::typeCheck(CodeBlock *context, [[maybe_unused]] Type returnType) {
    return rval->typeCheck(context);
}

FunctionCall::FunctionCall(Variable *name)
    : rvalue(name->source, name->row, name->col), name(name) {
}

void FunctionCall::show() {
    fprintf(stderr, "Function Call: ");
    name->show();
    fprintf(stderr, "(");
    size_t size = arguments.size();
    if (size > 0) {
        for (size_t i = 0; i < arguments.size() - 1; i++) {
            arguments[i]->show();
            fprintf(stderr, ",");
        }
        arguments[size - 1]->show();
    }
    fprintf(stderr, ")");
}

void FunctionCall::compile(FILE *outfile) {
    name->compile(outfile);
    fprintf(outfile, "(");
    size_t size = arguments.size();
    if (size > 0) {
        for (size_t i = 0; i < size - 1; i++) {
            arguments[i]->compile(outfile);
            fprintf(outfile, ",");
        }
        arguments[size - 1]->compile(outfile);
    }
    fprintf(outfile, ")");
}

Type FunctionCall::typeCheck(CodeBlock *context) {
    std::unordered_map<std::string, Function *>::iterator function
          = context->global->functions.find(name->variable->s);
    if (function == context->global->functions.end()) {
        throw std::invalid_argument("Could not find function");
    }
    for (size_t i = 0; i < arguments.size(); i++) {
        if (arguments[i]->typeCheck(context) != function->second->parameters[i].second) {
            arguments[i]->error("Incorrect argument type");
        }
    }
    return function->second->type;
}

Return::Return(rvalue *rval, Token *token) : rval(rval), token(token) {
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

Type Return::typeCheck(CodeBlock *context, Type returnType) {
    if (rval == nullptr) {
        if (returnType != Type::VOID) {
            token->error("Invalid return from non-void function");
        }
        return Type::VOID;
    }
    Type type = rval->typeCheck(context);
    if (type != returnType) {
        token->error("Invalid return type");
    }
    return type;
}

CodeBlock::CodeBlock(AST *global)
    : locals(new std::unordered_map<Identifier *, std::tuple<Type, bool>, Hasher,
          Comparator>),
      global(global) {
}

void CodeBlock::compile(FILE *outfile) {
    for (std::pair var : *locals) {
        Identifier *name = var.first;
        std::tuple<Type, bool> info = var.second;
        if (!std::get<1>(info)) {
            Type type = std::get<0>(info);
            fprintf(outfile, "    ");
            Primitive::compile(outfile, type);
            fprintf(outfile, " ");
            name->compile(outfile);
            fprintf(outfile, ";\n");
        }
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
        if (global->functions.find(id->variable->s) == global->functions.end()) {
            id->error("Undeclared identifier %.*s", id->variable->s.len,
                  id->variable->s.start);
        }
    }
}

Type CodeBlock::getType(Identifier *var) {
    std::unordered_map<Identifier *, std::tuple<Type, bool>, Hasher, Comparator>::iterator
          local
          = locals->find(var);
    if (local != locals->end()) {
        return std::get<0>(local->second);
    }
    if (parent == nullptr) {
        throw std::invalid_argument("Could not find variable type and no parent context");
    }
    return parent->getType(var);
}

void CodeBlock::typeCheck(Type returnType) {
    for (Statement *s : statements) {
        s->typeCheck(this, returnType);
    }
}

Function::Function(AST *ast) : body(new CodeBlock(ast)) {
}

void Function::compile(FILE *outfile, std::string name) {
    Primitive::compile(outfile, type);
    fprintf(outfile, " %s(", name.c_str());
    size_t size = parameters.size();
    if (size > 0) {
        for (size_t i = 0; i < size - 1; i++) {
            std::pair<Identifier *, Type> param = parameters[i];
            Primitive::compile(outfile, param.second);
            fprintf(outfile, " ");
            param.first->compile(outfile);
            fprintf(outfile, ",");
        }
        std::pair<Identifier *, Type> param = parameters[size - 1];
        Primitive::compile(outfile, param.second);
        fprintf(outfile, " ");
        param.first->compile(outfile);
    }
    fprintf(outfile, "){\n");
    body->compile(outfile);
    fprintf(outfile, "}\n");
}

void Function::forward(FILE *outfile, std::string name) {
    Primitive::compile(outfile, type);
    fprintf(outfile, " %s(", name.c_str());
    size_t size = parameters.size();
    if (size > 0) {
        for (size_t i = 0; i < size - 1; i++) {
            std::pair<Identifier *, Type> param = parameters[i];
            Primitive::compile(outfile, param.second);
            fprintf(outfile, " ");
            param.first->compile(outfile);
            fprintf(outfile, ",");
        }
        std::pair<Identifier *, Type> param = parameters[size - 1];
        Primitive::compile(outfile, param.second);
        fprintf(outfile, " ");
        param.first->compile(outfile);
    }
    fprintf(outfile, ");\n");
}

void Function::resolve() {
    body->resolve();
}

void Function::typeCheck() {
    body->typeCheck(type);
}

AST::AST() {
    functions["print"] = new Print(this);
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
        f.second->resolve();
    }
    for (FunctionCall *call : functionCalls) {
        if (call->name->variable->s == "print") {
            size_t arg_size = call->arguments.size();
            if (call->arguments.size() != 1) {
                call->error(
                      "Function called with incorrect argument count (%ld instead of 1)",
                      arg_size);
            }
        } else {
            std::unordered_map<std::string, Function *>::iterator f
                  = functions.find(call->name->variable->s);
            if (f == functions.end()) {
                call->error("Function %.*s does not exist", call->name->variable->s.len,
                      call->name->variable->s.start);
            }
            size_t param_size = f->second->parameters.size();
            size_t arg_size = call->arguments.size();
            if (param_size != arg_size) {
                call->error("Function called with incorrect argument count (%ld instead "
                            "of %ld)",
                      arg_size, param_size);
            }
        }
    }
    for (std::pair<std::string, Function *> f : functions) {
        f.second->typeCheck();
    }
}

} // namespace AST
