#include <sys/mman.h>
#include <sys/stat.h>

#include <ctype.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char **argv) {
    int infile = open("sample.canyon", O_RDONLY);
    if (infile == -1) {
        perror("Failed to open the file");
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
        puts("Empty file");
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

    printf("File contents: %s\n", fileData);

    // Unmap and close the file
    if (munmap(fileData, fileInfo.st_size) == -1) {
        perror("Error un-mmapping the file");
        close(infile);
        return -1;
    }

    return 0;
}
