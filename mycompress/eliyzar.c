#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

void usage(const char *program_name) {
    fprintf(stderr, "[%s] Usage: %s [-o outfile] [file...]\n", program_name, program_name);
    exit(EXIT_FAILURE);
}

// Function to compress and write data
void compressAndWrite(char *buffer, size_t len, FILE *outFile, int *writeCount) {
    u_char lastChar;
    u_char currentChar;
    int count = 0;

    for (int i = 0; i < len; ++i) {

        currentChar = buffer[i];

        if (count == 0) {
            lastChar = currentChar;
            count = 1;
            continue;
        }

        if (lastChar == currentChar) {
            count++;
            continue;
        }

        int written = fprintf(outFile, "%c%d", lastChar, count);
        if (written == -1) {
            exit(EXIT_FAILURE);
        }

        *writeCount += written;
    }
    int written = fprintf(outFile, "\n");
    *writeCount += written;
}

int main(int argc, char *argv[]) {
    const char *process = argv[0];
    FILE *outFile = stdout;

    const char *inFileName = NULL;
    int opt;

    // "o:ab" -> o can have an argument after it a and b can't
    while ((opt = getopt(argc, argv, "o:")) != -1) {
        switch (opt) {
            case 'o':
                if (inFileName != NULL) {
                    usage(process);
                }
                inFileName = optarg;
                break;

                // anything that isn't specified in the last param of getopt()
            case '?':
                usage(process);
                break;
                // unreachable code
            default:
                assert(0);
        }
    }

    FILE *inFile = stdin;
    // add necessary checks to open the outfile

    char *line = NULL;
    size_t size = 0;
    int readCount = 0, writeCount = 0;

    // saves a line up until \n into line !!DYNAMICALLY free() needed
    while (getline(&line, &size, inFile) != -1) {
        compressAndWrite(line, strlen(line), stdout, &writeCount);
        readCount += (int) strlen(line);
    }
    free(line);

    // Cleanup
    if (outFile != stdout) {
        fclose(outFile);
    }

    // Print character counts to stderr
    fprintf(stderr, "Read: %d characters\n", readCount);
    fprintf(stderr, "Written: %d characters\n", writeCount);

    return EXIT_SUCCESS;
}

