#include "lexer.h"

#include "ast.h"
#include <string_view>

#include <queue>
#include <stdexcept>
#include <vector>

Lexer::Lexer(const char *const program, off_t size, const char *const source,
      uint32_t tabSize)
    : program(program), current(program), size(size), source(source), tabSize(tabSize) {
	if (tabSize == 0) {
		throw std::invalid_argument("Tab size must be greater than 0");
	}
}

std::vector<Token *> Lexer::tokenize() {
	slice();

	std::vector<Token *> tokens;
	// for (Slice s : slices) {
	for (; !slices.empty(); slices.pop()) {
		Slice &s = slices.front();
		std::cerr << s.contents << '\n';
		Keyword *keyword = createKeyword(s);
		if (keyword) {
			tokens.push_back(keyword);
			continue;
		}
		Primitive *primitive = createPrimitive(s);
		if (primitive) {
			tokens.push_back(primitive);
			continue;
		}
		Punctuation *punctuation = createPunctuation(s);
		if (punctuation) {
			tokens.push_back(punctuation);
			continue;
		}
		tokens.push_back(createIdentifier(s));
	}

	for (Token *t : tokens) {
		t->print(std::cerr);
		std::cerr << '\n';
	}

	return tokens;
}

void Lexer::slice() {
	size_t line = 1;
	size_t col = 1;

	while (current - program < size) {
		while (isspace(*current)) {
			if (*current == '\n') {
				line++;
				col = 1;
			} else if (*current == '\r') {
				line++;
				col = 1;
				if (current + 1 - program < size && current[1] == '\n') {
					current++;
				}
			} else if (*current == '\t') {
				col = ((col + tabSize - 1) / tabSize) * tabSize + 1;
			} else {
				col++;
			}
			current++;
		}
		if (current - program >= size) {
			break;
		}
		const char *tokenStart = current;
		size_t startCol = col;
		do {
			current++;
			col++;
		} while (current - program < size && !isSep(current));
		slices.emplace(std::string_view(tokenStart, current - tokenStart), source, line,
		      startCol);
	}
}

bool Lexer::isSep(const char *const c) {
	// If c is any of these characters, it is by default a separator
	if (isspace(c[0]) || c[0] == '(' || c[0] == ')' || c[0] == ';' || c[0] == '{'
	      || c[0] == '}' || c[0] == ',' || c[0] == '+' || c[0] == '-' || c[0] == '*'
	      || c[0] == '/' || c[0] == '%' || c[0] == '=') {
		return true;
	}

	// If c is alnum, it is sep as long as prev is not alnum
	if (isalnum(c[0]) && !isalnum(c[-1])) {
		return true;
	}

	return false;
}

Keyword *Lexer::createKeyword(const Slice &s) {
	if (s.contents == "void") {
		return new Keyword(s, Keyword::Type::VOID);
	}
	if (s.contents == "return") {
		return new Keyword(s, Keyword::Type::RETURN);
	}
	return nullptr;
}

Primitive *Lexer::createPrimitive(const Slice &s) {
	if (s.contents == "int") {
		return new Primitive(s, Type::INT);
	}
	if (s.contents == "byte") {
		return new Primitive(s, Type::BYTE);
	}
	if (s.contents == "short") {
		return new Primitive(s, Type::SHORT);
	}
	if (s.contents == "long") {
		return new Primitive(s, Type::LONG);
	}
	if (s.contents == "float") {
		return new Primitive(s, Type::FLOAT);
	}
	if (s.contents == "double") {
		return new Primitive(s, Type::DOUBLE);
	}
	if (s.contents == "bool") {
		return new Primitive(s, Type::BOOL);
	}
	if (s.contents == "char") {
		return new Primitive(s, Type::CHAR);
	}
	return nullptr;
}

Punctuation *Lexer::createPunctuation(const Slice &s) {
	if (s.contents == "(") {
		return new Punctuation(s, Punctuation::Type::OpenParen);
	}
	if (s.contents == ")") {
		return new Punctuation(s, Punctuation::Type::CloseParen);
	}
	if (s.contents == ";") {
		return new Punctuation(s, Punctuation::Type::Semicolon);
	}
	if (s.contents == "{") {
		return new Punctuation(s, Punctuation::Type::OpenBrace);
	}
	if (s.contents == "}") {
		return new Punctuation(s, Punctuation::Type::CloseBrace);
	}
	if (s.contents == ",") {
		return new Punctuation(s, Punctuation::Type::Comma);
	}
	if (s.contents == "=") {
		return new Punctuation(s, Punctuation::Type::Equals);
	}
	if (s.contents == "+") {
		return new Punctuation(s, Punctuation::Type::Plus);
	}
	if (s.contents == "-") {
		return new Punctuation(s, Punctuation::Type::Minus);
	}
	if (s.contents == "*") {
		return new Punctuation(s, Punctuation::Type::Times);
	}
	if (s.contents == "/") {
		return new Punctuation(s, Punctuation::Type::Divide);
	}
	if (s.contents == "%") {
		return new Punctuation(s, Punctuation::Type::Mod);
	}
	return nullptr;
}

Identifier *Lexer::createIdentifier(const Slice &s) {
	return new Identifier(s);
}
