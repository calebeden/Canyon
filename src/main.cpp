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
#include <fstream>
#include <iostream>
#include <string>
#include <unistd.h>

int main(int argc, char **argv) {
	if (argc != 3) {
		std::cerr << "Unexpected argument count. Usage: ./build/main infile outfile\n";
		return EXIT_FAILURE;
	}

	// Read the source file into a string
	std::ifstream infile = std::ifstream(argv[1]);
	if (!infile) {
		int e = errno;
		std::cerr << "Failed to open source file " << argv[1] << ": " << strerror(e)
		          << '\n';
		return EXIT_FAILURE;
	}
	std::string fileData = std::string((std::istreambuf_iterator<char>(infile)),
	      std::istreambuf_iterator<char>());

	Lexer l = Lexer(fileData, argv[1]);
	std::vector<Token *> tokens = l.tokenize();

	Parser p;
	AST::Module module = p.parseModule(tokens);

	std::ofstream outfile = std::ofstream(argv[2], std::ios::out | std::ios::trunc);
	if (!outfile) {
		int e = errno;
		std::cerr << "Failed to open outfile " << argv[2] << ": " << strerror(e) << '\n';
		return EXIT_FAILURE;
	}

	module.compile(outfile);

	// Close the files
	outfile.close();
	if (!outfile) {
		int e = errno;
		std::cerr << "Error closing outfile " << argv[2] << ": " << strerror(e) << '\n';
		return EXIT_FAILURE;
	}
	infile.close();
	if (!infile) {
		int e = errno;
		std::cerr << "Error closing infile " << argv[1] << ": " << strerror(e) << '\n';
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
