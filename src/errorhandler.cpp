#include "errorhandler.h"

#include "tokens.h"

#include <filesystem>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

ErrorHandler::Error::Error(std::filesystem::path source, size_t row, size_t col,
      std::string message)
    : message(std::move(message)), source(std::move(source)), row(row), col(col) {
}

void ErrorHandler::error(const Token &token, std::string message) {
	return error(token.source, token.row, token.col, std::move(message));
}

void ErrorHandler::error(std::filesystem::path source, size_t row, size_t col,
      std::string message) {
	errors.push(Error(std::move(source), row, col, std::move(message)));
}

bool ErrorHandler::handleErrors(std::ostream &os) {
	bool hasErrors = !errors.empty();
	for (; !errors.empty(); errors.pop()) {
		Error &e = errors.front();
		os << "Error at " << e.source << ':' << e.row << ':' << e.col << ": "
		   << e.message;
	}
	if (hasErrors) {
		exit(EXIT_FAILURE);
	}
	return hasErrors;
}
