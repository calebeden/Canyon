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
#include <unistd.h>

int main(int argc, char **argv) {
	if (argc != 3) {
		std::cerr << "Unexpected argument count. Usage: ./build/main infile outfile\n";
		return EXIT_FAILURE;
	}
	int infile = open(argv[1], O_RDONLY);
	if (infile == -1) {
		std::cerr << "Failed to open " << argv[1] << ": " << strerror(errno) << '\n';
		return EXIT_FAILURE;
	}

	// Get the size of the file.
	struct stat fileInfo;
	if (fstat(infile, &fileInfo) != 0) {
		std::cerr << "Failed to get file information for " << argv[1] << ": "
		          << strerror(errno) << '\n';
		return EXIT_FAILURE;
	}

	// Read the file into memory.
	char *fileData = (char *) mmap(NULL, std::max(1L, fileInfo.st_size), PROT_READ,
	      MAP_PRIVATE | MAP_FILE, infile, 0);
	if (fileData == MAP_FAILED) {
		std::cerr << "Failed to map the file " << argv[1]
		          << " into memory: " << strerror(errno) << '\n';
		return EXIT_FAILURE;
	}
	if (close(infile) != 0) {
		std::cerr << "Failed to close " << argv[1] << ": " << strerror(errno) << '\n';
		return EXIT_FAILURE;
	}

	Lexer l = Lexer(fileData, fileInfo.st_size, argv[1]);
	std::vector<Token *> tokens = l.tokenize();

	Parser p;
	AST::Module module = p.parseModule(tokens);

	std::ofstream outfile = std::ofstream(argv[2], std::ios::out | std::ios::trunc);
	if (!outfile) {
		std::cerr << "Failed to open outfile " << argv[2] << ": " << strerror(errno)
		          << '\n';
		return EXIT_FAILURE;
	}

	module.compile(outfile);

	// Unmap and close the files
	outfile.close();
	if (!outfile) {
		std::cerr << "Error closing outfile " << argv[2] << ": " << strerror(errno)
		          << '\n';
		return EXIT_FAILURE;
	}
	if (munmap(fileData, std::max(1L, fileInfo.st_size)) == -1) {
		std::cerr << "Error un-mapping infile " << argv[1] << ": " << strerror(errno)
		          << '\n';
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
