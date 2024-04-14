#ifndef ERRORHANDLER_H
#define ERRORHANDLER_H

#include "tokens.h"

#include <filesystem>
#include <iostream>
#include <queue>
#include <string>

class ErrorHandler {
	struct Error {
		std::string message;
		std::filesystem::path source;
		size_t row;
		size_t col;
		Error(std::filesystem::path source, size_t row, size_t col, std::string message);
	};

	std::queue<Error> errors;
public:
	[[noreturn]] void error(Token *token, std::string message);
	[[noreturn]] void error(std::filesystem::path source, size_t row, size_t col, std::string message);
	bool printErrors(std::ostream &os);
	~ErrorHandler() = default;
};

#endif
