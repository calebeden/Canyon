#include "errorhandler.h"
#include "lexer.h"
#include "tokens.h"

#include <iostream>
#include <memory>
#include <vector>

int main() {
	ErrorHandler e;
	Lexer l = Lexer("", "", e);
	std::vector<std::unique_ptr<Token>> tokens = l.lex();
	e.handleErrors(std::cerr);
	return 0;
}
