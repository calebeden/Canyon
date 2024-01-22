#include "ast.h"

#include "builtins.h"
#include "errorhandler.h"
#include "tokens.h"
#include <string_view>

#include <charconv>
#include <cstdarg>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <string>

namespace AST {

rvalue::rvalue(std::filesystem::path source, size_t row, size_t col)
    : source(source), row(row), col(col) {
}

Literal::Literal(const Identifier *const value)
    : rvalue(value->source, value->row, value->col), value(0) {
	// https://stackoverflow.com/a/56634586
	auto result = std::from_chars(value->s.data(), value->s.data() + value->s.size(),
	      this->value);
	if (result.ec == std::errc::invalid_argument
	      || result.ec == std::errc::result_out_of_range) {
		std::cerr << "Error creating Literal: Expected an integer\n";
		exit(EXIT_FAILURE);
	}
}

void Literal::print(std::ostream &os) const {
	os << value;
}

void Literal::compile(std::ostream &outfile) const {
	outfile << value;
}

Type Literal::typeCheck([[maybe_unused]] const CodeBlock &context,
      [[maybe_unused]] ErrorHandler &errors) const {
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

Type Variable::typeCheck(const CodeBlock &context,
      [[maybe_unused]] ErrorHandler &errors) const {
	return context.getType(variable);
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

Type Assignment::typeCheck(const CodeBlock &context, ErrorHandler &errors) const {
	Type varType = context.getType(variable);
	Type expType = expression->typeCheck(context, errors);
	if (varType != expType) {
		errors.error(source, row, col, "Incompatible types");
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

Type Addition::typeCheck(const CodeBlock &context, ErrorHandler &errors) const {
	Type type1 = operand1->typeCheck(context, errors);
	Type type2 = operand2->typeCheck(context, errors);
	if (type1 != type2) {
		errors.error(source, row, col, "Incompatible types");
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

Type Subtraction::typeCheck(const CodeBlock &context, ErrorHandler &errors) const {
	Type type1 = operand1->typeCheck(context, errors);
	Type type2 = operand2->typeCheck(context, errors);
	if (type1 != type2) {
		errors.error(source, row, col, "Incompatible types");
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

Type Multiplication::typeCheck(const CodeBlock &context, ErrorHandler &errors) const {
	Type type1 = operand1->typeCheck(context, errors);
	Type type2 = operand2->typeCheck(context, errors);
	if (type1 != type2) {
		errors.error(source, row, col, "Incompatible types");
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

Type Division::typeCheck(const CodeBlock &context, ErrorHandler &errors) const {
	Type type1 = operand1->typeCheck(context, errors);
	Type type2 = operand2->typeCheck(context, errors);
	if (type1 != type2) {
		errors.error(source, row, col, "Incompatible types");
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

Type Modulo::typeCheck(const CodeBlock &context, ErrorHandler &errors) const {
	Type type1 = operand1->typeCheck(context, errors);
	Type type2 = operand2->typeCheck(context, errors);
	if (type1 != type2) {
		errors.error(source, row, col, "Incompatible types");
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

Type Expression::typeCheck(const CodeBlock &context, [[maybe_unused]] Type returnType,
      ErrorHandler &errors) const {
	return rval->typeCheck(context, errors);
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

Type FunctionCall::typeCheck(const CodeBlock &context, ErrorHandler &errors) const {
	auto function = context.global->functions.find(name->variable->s);
	if (function == context.global->functions.end()) {
		throw std::invalid_argument("Could not find function");
	}
	for (size_t i = 0; i < arguments.size(); i++) {
		if (arguments[i]->typeCheck(context, errors)
		      != function->second->parameters[i].second) {
			errors.error(arguments[i]->source, arguments[i]->row, arguments[i]->col,
			      "Incorrect argument type");
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

Type Return::typeCheck(const CodeBlock &context, Type returnType,
      ErrorHandler &errors) const {
	if (rval == nullptr) {
		if (returnType != Type::VOID) {
			errors.error(token, "Invalid return from non-void function");
		}
		return Type::VOID;
	}
	Type type = rval->typeCheck(context, errors);
	if (type != returnType) {
		errors.error(token, "Invalid return type");
	}
	return type;
}

CodeBlock::CodeBlock(Module *global) : global(global) {
}

void CodeBlock::compile(std::ostream &outfile) const {
	for (std::pair<Identifier *, std::tuple<Type, bool>> var : locals) {
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

CodeBlock::IdentifierStatus CodeBlock::find(const Variable *id) const {
	if (locals.find(id->variable) != locals.end()) {
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

void CodeBlock::resolve(ErrorHandler &errors) {
	for (Variable *id : deferred) {
		if (global->functions.find(id->variable->s) == global->functions.end()) {
			errors.error(id->variable,
			      std::string("Undeclared identifier ").append(id->variable->s));
		}
	}
}

Type CodeBlock::getType(Identifier *var) const {
	auto local = locals.find(var);
	if (local != locals.end()) {
		return std::get<0>(local->second);
	}
	if (parent == nullptr) {
		throw std::invalid_argument("Could not find variable type and no parent context");
	}
	return parent->getType(var);
}

void CodeBlock::typeCheck(Type returnType, ErrorHandler &errors) const {
	for (Statement *s : statements) {
		s->typeCheck(*this, returnType, errors);
	}
}

Function::Function(Module *module) : body(new CodeBlock(module)) {
}

void Function::compile(std::ostream &outfile, std::string_view name) const {
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

void Function::forward(std::ostream &outfile, std::string_view name) const {
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

void Function::resolve(ErrorHandler &errors) {
	body->resolve(errors);
}

void Function::typeCheck(ErrorHandler &errors) const {
	body->typeCheck(type, errors);
}

Module::Module() {
	functions["print"] = new Print(this);
}

void Module::compile(std::ostream &outfile) const {
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

void Module::resolve(ErrorHandler &errors) {
	for (std::pair<std::string_view, Function *> f : functions) {
		f.second->resolve(errors);
	}
	for (FunctionCall *call : functionCalls) {
		if (call->name->variable->s == "print") {
			size_t arg_size = call->arguments.size();
			if (call->arguments.size() != 1) {
				errors.error(call->source, call->row, call->col,
				      "Function called with incorrect argument count ("
				            + std::to_string(arg_size) + " instead of 1)");
			}
		} else {
			auto f = functions.find(call->name->variable->s);
			if (f == functions.end()) {
				errors.error(call->source, call->row, call->col,
				      std::string("Function ").append(call->name->variable->s)
				            + " does not exist");
			}
			size_t param_size = f->second->parameters.size();
			size_t arg_size = call->arguments.size();
			if (param_size != arg_size) {
				errors.error(call->source, call->row, call->col,
				      "Function called with incorrect argument count ("
				            + std::to_string(arg_size) + " instead of "
				            + std::to_string(param_size) + ")");
			}
		}
	}
	for (std::pair<std::string_view, Function *> f : functions) {
		f.second->typeCheck(errors);
	}
}

} // namespace AST
