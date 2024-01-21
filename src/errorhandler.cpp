#include "errorhandler.h"

#include "tokens.h"

#include <iostream>
#include <string>
#include <utility>
#include <vector>

ErrorHandler::Error::Error(const char *const source, size_t row, size_t col,
      std::string message)
    : message(std::move(message)), source(source), row(row), col(col) {
}

void ErrorHandler::error(Token *token, std::string message) {
	return error(token->source, token->row, token->col, std::move(message));
}

void ErrorHandler::error(const char *const source, size_t row, size_t col,
      std::string message) {
	errors.push(Error(source, row, col, std::move(message)));
	// TODO make this actually queueable
	printErrors(std::cerr);
	exit(EXIT_FAILURE);
}

bool ErrorHandler::printErrors(std::ostream &os) {
	bool hasErrors = !errors.empty();
	for (; !errors.empty(); errors.pop()) {
		Error &e = errors.front();
		os << "Error at " << e.source << ':' << e.row << ':' << e.col << ": "
		   << e.message;
	}
	return hasErrors;
}
