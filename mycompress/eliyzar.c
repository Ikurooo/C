#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

void usage(const char *program_name) {
    fprintf(stderr, "[%s] Usage: %s [-o outfile] [file...]\n", program_name, program_name);
    exit(EXIT_FAILURE);
}

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
        lastChar = currentChar;
        count = 1;
    }

    int written = fprintf(outFile, "\n");
    *writeCount += written;
}

int main(int argc, char *argv[]) {
    const char *process = argv[0];
    const char *outFileName = NULL;
    int opt;

    while ((opt = getopt(argc, argv, "o:")) != -1) {
        switch (opt) {
            case 'o':
                if (outFileName != NULL) {
                    usage(process);
                }
                outFileName = optarg;
                break;
            case '?':
                usage(process);
                break;
            default:
                assert(0);
        }
    }

    FILE *outFile = stdout;
    if (outFileName != NULL) {
        outFile = fopen(outFileName, "w");
        if (outFile == NULL) {
            fprintf(stderr, "[%s] ERROR: An error occurred while opening file %s\n", process, outFileName);
            exit(EXIT_FAILURE);
        }
    }

    int readCount = 0;
    int writeCount = 0;

    if (argc - optind == 0) {
        char *line = NULL;
        size_t size = 0;

        while (getline(&line, &size, stdin) != -1) {
            compressAndWrite(line, strlen(line), outFile, &writeCount);
            readCount += (int) strlen(line);
        }
        free(line);
    }

    for (int i = optind; i < argc; ++i) {
        FILE *inFile = fopen(argv[i], "r");
        if (inFile == NULL) {
            fprintf(stderr, "[%s] ERROR: An error occurred while opening file %s\n", process, argv[i]);
            if (outFile != stdout) {
                fclose(outFile);
            }
            exit(EXIT_FAILURE);
        }

        char *line = NULL;
        size_t size = 0;

        while (getline(&line, &size, inFile) != -1) {
            compressAndWrite(line, strlen(line), outFile, &writeCount);
            readCount += (int) strlen(line);
        }
        free(line);

        fclose(inFile);
    }

    if (outFile != stdout) {
        fclose(outFile);
    }

    fprintf(stderr, "Read: %d characters\n", readCount);
    fprintf(stderr, "Written: %d characters\n", writeCount);

    return EXIT_SUCCESS;
}