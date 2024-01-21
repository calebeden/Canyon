#include "ast.h"

#include "builtins.h"
#include "tokens.h"

#include <charconv>
#include <cstdarg>
#include <cstring>
#include <iostream>

namespace AST {

rvalue::rvalue(const char *const source, size_t row, size_t col)
    : source(source), row(row), col(col) {
}

void rvalue::error(const char *const format, ...) const {
	fprintf(stderr, "Error at %s:%ld:%ld: ", source, row, col);
	va_list args;
	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);
	fprintf(stderr, "\n");
	exit(EXIT_FAILURE);
}

Literal::Literal(Identifier *value) : rvalue(value->source, value->row, value->col) {
	// https://stackoverflow.com/a/56634586
	int val;
	auto result
	      = std::from_chars(value->s.data(), value->s.data() + value->s.size(), val);
	if (result.ec == std::errc::invalid_argument) {
		std::cerr << "Error creating Literal: Expected an integer\n";
		exit(EXIT_FAILURE);
	}
	this->value = static_cast<int32_t>(val);
}

void Literal::print(std::ostream &os) const {
	os << value;
}

void Literal::compile(std::ostream &outfile) const {
	outfile << value;
}

Type Literal::typeCheck([[maybe_unused]] const CodeBlock *context) const {
	return Type::INT;
}

Variable::Variable(Identifier *variable)
    : rvalue(variable->source, variable->row, variable->col), variable(variable) {
}

void Variable::print(std::ostream &os) const {
	os << "Variable: " << variable->s << '\n';
}

void Variable::compile(std::ostream &outfile) const {
	variable->compile(outfile);
}

Type Variable::typeCheck(const CodeBlock *context) const {
	return context->getType(variable);
}

Assignment::Assignment(Identifier *variable, rvalue *expression)
    : rvalue(variable->source, variable->row, variable->col), variable(variable),
      expression(expression) {
}

void Assignment::print(std::ostream &os) const {
	os << "Assignment Statement: " << variable->s << " = ";
	expression->print(os);
	os << '\n';
}

void Assignment::compile(std::ostream &outfile) const {
	outfile << variable->s << '=';
	expression->compile(outfile);
}

Type Assignment::typeCheck(const CodeBlock *context) const {
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

void Addition::print(std::ostream &os) const {
	os << '(';
	operand1->print(os);
	os << " + ";
	operand2->print(os);
	os << ')';
}

void Addition::compile(std::ostream &outfile) const {
	outfile << '(';
	operand1->compile(outfile);
	outfile << '+';
	operand2->compile(outfile);
	outfile << ')';
}

Type Addition::typeCheck(const CodeBlock *context) const {
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

void Subtraction::print(std::ostream &os) const {
	os << '(';
	operand1->print(os);
	os << " - ";
	operand2->print(os);
	os << ')';
}

void Subtraction::compile(std::ostream &outfile) const {
	outfile << '(';
	operand1->compile(outfile);
	outfile << '-';
	operand2->compile(outfile);
	outfile << ')';
}

Type Subtraction::typeCheck(const CodeBlock *context) const {
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

void Multiplication::print(std::ostream &os) const {
	os << '(';
	operand1->print(os);
	os << " * ";
	operand2->print(os);
	os << ')';
}

void Multiplication::compile(std::ostream &outfile) const {
	outfile << '(';
	operand1->compile(outfile);
	outfile << '*';
	operand2->compile(outfile);
	outfile << ')';
}

Type Multiplication::typeCheck(const CodeBlock *context) const {
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

void Division::print(std::ostream &os) const {
	os << '(';
	operand1->print(os);
	os << " / ";
	operand2->print(os);
	os << ')';
}

void Division::compile(std::ostream &outfile) const {
	outfile << '(';
	operand1->compile(outfile);
	outfile << '/';
	operand2->compile(outfile);
	outfile << ')';
}

Type Division::typeCheck(const CodeBlock *context) const {
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

void Modulo::print(std::ostream &os) const {
	os << '(';
	operand1->print(os);
	os << " % ";
	operand2->print(os);
	os << ')';
}

void Modulo::compile(std::ostream &outfile) const {
	outfile << '(';
	operand1->compile(outfile);
	outfile << '%';
	operand2->compile(outfile);
	outfile << ')';
}

Type Modulo::typeCheck(const CodeBlock *context) const {
	Type type1 = operand1->typeCheck(context);
	Type type2 = operand2->typeCheck(context);
	if (type1 != type2) {
		error("Incompatible types");
	}
	return type2;
}

Expression::Expression(rvalue *rval) : rval(rval) {
}

void Expression::print(std::ostream &os) const {
	os << "Expression Statement: ";
	rval->print(os);
	os << '\n';
}

void Expression::compile(std::ostream &outfile) const {
	rval->compile(outfile);
	outfile << ";\n";
}

Type Expression::typeCheck(const CodeBlock *context,
      [[maybe_unused]] Type returnType) const {
	return rval->typeCheck(context);
}

FunctionCall::FunctionCall(Variable *name)
    : rvalue(name->source, name->row, name->col), name(name) {
}

void FunctionCall::print(std::ostream &os) const {
	os << "Function Call: ";
	name->print(os);
	os << '(';
	size_t size = arguments.size();
	if (size > 0) {
		for (size_t i = 0; i < arguments.size() - 1; i++) {
			arguments[i]->print(os);
			os << ',';
		}
		arguments[size - 1]->print(os);
	}
	os << ')';
}

void FunctionCall::compile(std::ostream &outfile) const {
	name->compile(outfile);
	outfile << '(';
	size_t size = arguments.size();
	if (size > 0) {
		for (size_t i = 0; i < size - 1; i++) {
			arguments[i]->compile(outfile);
			outfile << ',';
		}
		arguments[size - 1]->compile(outfile);
	}
	outfile << ')';
}

Type FunctionCall::typeCheck(const CodeBlock *context) const {
	auto function = context->global->functions.find(name->variable->s);
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

void Return::print(std::ostream &os) const {
	if (rval == nullptr) {
		os << "Return (void)\n";
	} else {
		os << "Return: ";
		rval->print(os);
		os << '\n';
	}
}

void Return::compile(std::ostream &outfile) const {
	if (rval == nullptr) {
		outfile << "return;\n";
	} else {
		outfile << "return ";
		rval->compile(outfile);
		outfile << ";\n";
	}
}

Type Return::typeCheck(const CodeBlock *context, Type returnType) const {
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

void CodeBlock::compile(std::ostream &outfile) const {
	for (std::pair<Identifier *, std::tuple<Type, bool>> var : *locals) {
		Identifier *name = var.first;
		std::tuple<Type, bool> info = var.second;
		if (!std::get<1>(info)) {
			Type type = std::get<0>(info);
			outfile << "    ";
			Primitive::compile(outfile, type);
			outfile << " ";
			name->compile(outfile);
			outfile << ";\n";
		}
	}
	for (Statement *s : statements) {
		outfile << "    ";
		s->compile(outfile);
	}
}

void CodeBlock::defer(Variable *rval) {
	deferred.push_back(rval);
}

CodeBlock::IdentifierStatus CodeBlock::find(Variable *id) const {
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
			id->error("Undeclared identifier %.*s", id->variable->s.size(),
			      id->variable->s.data());
		}
	}
}

Type CodeBlock::getType(Identifier *var) const {
	auto local = locals->find(var);
	if (local != locals->end()) {
		return std::get<0>(local->second);
	}
	if (parent == nullptr) {
		throw std::invalid_argument("Could not find variable type and no parent context");
	}
	return parent->getType(var);
}

void CodeBlock::typeCheck(Type returnType) const {
	for (Statement *s : statements) {
		s->typeCheck(this, returnType);
	}
}

Function::Function(AST *ast) : body(new CodeBlock(ast)) {
}

void Function::compile(std::ostream &outfile, const std::string_view &name) const {
	Primitive::compile(outfile, type);
	outfile << ' ' << name << '(';
	size_t size = parameters.size();
	if (size > 0) {
		for (size_t i = 0; i < size - 1; i++) {
			std::pair<Identifier *, Type> param = parameters[i];
			Primitive::compile(outfile, param.second);
			outfile << ' ';
			param.first->compile(outfile);
			outfile << ',';
		}
		std::pair<Identifier *, Type> param = parameters[size - 1];
		Primitive::compile(outfile, param.second);
		outfile << ' ';
		param.first->compile(outfile);
	}
	outfile << "){\n";
	body->compile(outfile);
	outfile << "}\n";
}

void Function::forward(std::ostream &outfile, const std::string_view &name) const {
	Primitive::compile(outfile, type);
	outfile << ' ' << name << '(';
	size_t size = parameters.size();
	if (size > 0) {
		for (size_t i = 0; i < size - 1; i++) {
			std::pair<Identifier *, Type> param = parameters[i];
			Primitive::compile(outfile, param.second);
			outfile << ' ';
			param.first->compile(outfile);
			outfile << ',';
		}
		std::pair<Identifier *, Type> param = parameters[size - 1];
		Primitive::compile(outfile, param.second);
		outfile << ' ';
		param.first->compile(outfile);
	}
	outfile << ");\n";
}

void Function::resolve() {
	body->resolve();
}

void Function::typeCheck() const {
	body->typeCheck(type);
}

AST::AST() {
	functions["print"] = new Print(this);
}

void AST::compile(std::ostream &outfile) const {
	outfile << "#include <stdio.h>\n";
	// Forward declarations
	for (std::pair<std::string_view, Function *> f : functions) {
		f.second->forward(outfile, f.first);
	}
	outfile << "int main(int argc, char **argv) {\n"
	           "    canyonMain();\n"
	           "    return 0;\n"
	           "}\n";
	// Actual code
	for (std::pair<std::string_view, Function *> f : functions) {
		f.second->compile(outfile, f.first);
	}
}

void AST::resolve() {
	for (std::pair<std::string_view, Function *> f : functions) {
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
			auto f = functions.find(call->name->variable->s);
			if (f == functions.end()) {
				call->error("Function %.*s does not exist",
				      call->name->variable->s.size(), call->name->variable->s.data());
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
	for (std::pair<std::string_view, Function *> f : functions) {
		f.second->typeCheck();
	}
}

} // namespace AST
