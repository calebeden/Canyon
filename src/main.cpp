#include "ast.h"
#include "errorhandler.h"
#include "lexer.h"
#include "parser.h"
#include "tokens.h"

#include <cerrno>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <span>
#include <string>
#include <vector>

int main(int argc, char **argv) {
	if (argc != 3) {
		std::cerr << "Unexpected argument count. Usage: ./build/main infile outfile\n";
		return EXIT_FAILURE;
	}

	auto args = std::span(argv, size_t(argc));
	std::filesystem::path infileName = std::filesystem::path(args[1]);
	std::filesystem::path outfileName = std::filesystem::path(args[2]);

	// Read the source file into a string
	std::ifstream infile = std::ifstream(infileName);
	if (!infile) {
		int e = errno;
		std::cerr << "Failed to open source file " << infileName << ": " << strerror(e)
		          << '\n';
		return EXIT_FAILURE;
	}
	std::string fileData = std::string((std::istreambuf_iterator<char>(infile)),
	      std::istreambuf_iterator<char>());

	ErrorHandler errorHandler;

	Lexer l = Lexer(fileData, infileName, errorHandler);
	std::vector<std::unique_ptr<Token>> tokens = l.lex();
	if (errorHandler.handleErrors(std::cerr)) {
		return EXIT_FAILURE;
	}

	Parser p = Parser(std::move(tokens), errorHandler);
	std::unique_ptr<Module> mod = p.parse();
	if (errorHandler.handleErrors(std::cerr)) {
		return EXIT_FAILURE;
	}

	std::ofstream outfile = std::ofstream(outfileName, std::ios::out | std::ios::trunc);
	if (!outfile) {
		int e = errno;
		std::cerr << "Failed to open outfile " << outfileName << ": " << strerror(e)
		          << '\n';
		return EXIT_FAILURE;
	}

	outfile << "// Compiled code\n";

	// Close the files
	outfile.close();
	if (!outfile) {
		int e = errno;
		std::cerr << "Error closing outfile " << outfileName << ": " << strerror(e)
		          << '\n';
		return EXIT_FAILURE;
	}
	infile.close();
	if (!infile) {
		int e = errno;
		std::cerr << "Error closing infile " << infileName << ": " << strerror(e) << '\n';
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
