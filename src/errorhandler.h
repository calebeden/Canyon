#ifndef ERRORHANDLER_H
#define ERRORHANDLER_H

#include "tokens.h"

#include <iostream>
#include <queue>
#include <string>

class ErrorHandler {
	struct Error {
		std::string message;
		const char *source;
		size_t row;
		size_t col;
		Error(const char *source, size_t row, size_t col, std::string message);
	};

	std::queue<Error> errors;
public:
	void error(Token *token, std::string message);
	void error(const char *source, size_t row, size_t col, std::string message);
	bool printErrors(std::ostream &os);
};

#endif
