#include "ast.h"
#include "lexer.h"
#include "parser.h"
#include "tokens.h"
#include <sys/mman.h>
#include <sys/stat.h>

#include <cstring>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr,
              "Unexpected argument count. Usage: ./build/main infile outfile\n");
        return 1;
    }
    int infile = open(argv[1], O_RDONLY);
    if (infile == -1) {
        fprintf(stderr, "Failed to open %s: %s\n", argv[1], strerror(errno));
        return 1;
    }

    // Get the size of the file.
    struct stat fileInfo;
    if (fstat(infile, &fileInfo) != 0) {
        fprintf(stderr, "Failed to get file information for %s: %s\n", argv[1],
              strerror(errno));
        close(infile);
        return 1;
    }
    if (fileInfo.st_size == 0) {
        fprintf(stderr, "Empty source code file %s\n", argv[1]);
        close(infile);
        return 1;
    }

    // Read the file into memory.
    char *fileData = (char *) mmap(NULL, fileInfo.st_size, PROT_READ,
          MAP_PRIVATE | MAP_FILE, infile, 0);
    if (fileData == MAP_FAILED) {
        fprintf(stderr, "Failed to map the file %s into memory: %s\n", argv[1],
              strerror(errno));
        close(infile);
        return 1;
    }
    close(infile);

    // printf("File contents: %s\n", fileData);
    Lexer l = Lexer(fileData, fileInfo.st_size, argv[1]);
    std::vector<Token *> *tokens = l.tokenize();

    Parser p;
    AST::AST *ast = p.parseModule(tokens);

    FILE *outfile = fopen(argv[2], "w");
    if (outfile == nullptr) {
        fprintf(stderr, "Failed to open outfile %s: %s\n", argv[2], strerror(errno));
        return 1;
    }

    ast->compile(outfile);

    // Unmap and close the file
    if (munmap(fileData, fileInfo.st_size) == -1) {
        fprintf(stderr, "Error un-mmapping infile %s: %s\n", argv[1], strerror(errno));
        close(infile);
        return -1;
    }

    return 0;
}