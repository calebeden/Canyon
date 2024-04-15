#include "ast.h"
#include "lexer.h"
#include "parser.h"
#include "tokens.h"
#include <sys/mman.h>
#include <sys/stat.h>

#include <cctype>
#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <span>
#include <string>
#include <unistd.h>

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

	Lexer l = Lexer(fileData, infileName);
	std::vector<std::unique_ptr<Token>> tokens = l.tokenize();

	Parser p;
	AST::Module module = p.parseModule(tokens);

	std::ofstream outfile = std::ofstream(outfileName, std::ios::out | std::ios::trunc);
	if (!outfile) {
		int e = errno;
		std::cerr << "Failed to open outfile " << outfileName << ": " << strerror(e)
		          << '\n';
		return EXIT_FAILURE;
	}

	module.compile(outfile);

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
