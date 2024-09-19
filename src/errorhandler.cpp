#include "errorhandler.h"

#include "tokens.h"

#include <filesystem>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

ErrorHandler::Error::Error(std::filesystem::path source, std::string message)
    : message(std::move(message)), source(std::move(source)) {
}

std::string ErrorHandler::Error::toString() {
	std::stringstream stream;
	stream << "Error at " << source.string() << ": " << message;
	return stream.str();
}

ErrorHandler::ErrorWithLocation::ErrorWithLocation(std::filesystem::path source,
      size_t row, size_t col, std::string message)
    : Error(std::move(source), std::move(message)), row(row), col(col) {
}

std::string ErrorHandler::ErrorWithLocation::toString() {
	std::stringstream stream;
	stream << "Error at " << source.string() << ':' << row << ':' << col << ": "
	       << message;
	return stream.str();
}

void ErrorHandler::error(const Slice &slice, std::string message) {
	return error(slice.source, slice.row, slice.col, std::move(message));
}

void ErrorHandler::error(const Token &token, std::string message) {
	return error(token.s.source, token.s.row, token.s.col, std::move(message));
}

void ErrorHandler::error(std::filesystem::path source, size_t row, size_t col,
      std::string message) {
	errors.push(std::make_unique<ErrorWithLocation>(std::move(source), row, col,
	      std::move(message)));
}

void ErrorHandler::error(std::filesystem::path source, std::string message) {
	errors.push(std::make_unique<Error>(std::move(source), std::move(message)));
}

bool ErrorHandler::handleErrors(std::ostream &os) {
	bool hasErrors = !errors.empty();
	for (; !errors.empty(); errors.pop()) {
		std::unique_ptr<Error> &e = errors.front();
		os << e->toString() << std::endl;
	}
	if (hasErrors) {
		exit(EXIT_FAILURE);
	}
	return hasErrors;
}
