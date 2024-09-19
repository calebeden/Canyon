#ifndef ERRORHANDLER_H
#define ERRORHANDLER_H

#include "tokens.h"

#include <cstdint>
#include <filesystem>
#include <memory>
#include <queue>
#include <string>

#ifdef DEBUG_TEST_MODE
#	define test_virtual virtual
#else
#	define test_virtual
#endif

/**
 * @brief Collects and reports errors that occur while compiling Canyon code
 * 
 */
class ErrorHandler {
protected:
	struct Error {
		std::string message;
		std::filesystem::path source;
		Error(std::filesystem::path source, std::string message);
		virtual std::string toString();
		virtual ~Error() = default;
	};

	struct ErrorWithLocation : Error {
		size_t row;
		size_t col;
		ErrorWithLocation(std::filesystem::path source, size_t row, size_t col,
		      std::string message);
		std::string toString() override;
		virtual ~ErrorWithLocation() = default;
	};

	std::queue<std::unique_ptr<Error>> errors;
public:
	ErrorHandler() = default;
	test_virtual void error(const Slice &slice, std::string message);
	test_virtual void error(const Token &token, std::string message);
	test_virtual void error(std::filesystem::path source, size_t row, size_t col,
	      std::string message);
	test_virtual void error(std::filesystem::path source, std::string message);
	test_virtual bool handleErrors(std::ostream &os);
	test_virtual ~ErrorHandler() = default;
};

#endif
