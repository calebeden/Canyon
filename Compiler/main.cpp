#include "ast.h"
#include "parse.h"
#include <sys/mman.h>
#include <sys/stat.h>

#include <ctype.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "Unexpected argument count. Usage: ./build/main infile outfile\n");
        return 1;
    }
    int infile = open(argv[1], O_RDONLY);
    if (infile == -1) {
        perror("Failed to open infile");
        return 1;
    }
    FILE *outfile = fopen(argv[2], "w");
    if (outfile == nullptr) {
        perror("Failed to open outfile");
        return 1;
    }

    // Get the size of the file.
    struct stat fileInfo;
    if (fstat(infile, &fileInfo) != 0) {
        perror("Failed to get file information");
        close(infile);
        return 1;
    }
    if (fileInfo.st_size == 0) {
        fprintf(stderr, "Empty file");
        close(infile);
        return 1;
    }

    // Read the file into memory.
    char *fileData = (char *) mmap(NULL, fileInfo.st_size, PROT_READ,
          MAP_PRIVATE | MAP_FILE, infile, 0);
    if (fileData == MAP_FAILED) {
        perror("Failed to map the file into memory");
        close(infile);
        return 1;
    }
    close(infile);

    // printf("File contents: %s\n", fileData);
    AST *ast = tokenize(fileData, fileInfo.st_size);
    fprintf(outfile, "#include <stdio.h>\n"
                     "void canyonMain();\n"
                     "int main(int argc, char **argv) {\n"
                     "    canyonMain();\n"
                     "    return 0;\n"
                     "}\n");

    fprintf(outfile, "void canyonMain() {\n");
    ast->compile(outfile);
    fprintf(outfile, "}\n");

    // Unmap and close the file
    if (munmap(fileData, fileInfo.st_size) == -1) {
        perror("Error un-mmapping the file");
        close(infile);
        return -1;
    }

    return 0;
}
