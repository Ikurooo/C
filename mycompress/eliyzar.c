#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void usage(const char *program_name) {
    fprintf(stderr, "[%s] Usage: %s [-o outfile] [file...]\n", program_name, program_name);
    exit(EXIT_FAILURE);
}

void compressAndWrite(char *buffer, FILE *outFile, int *writeCount) {
    char lastChar = buffer[0];
    int count = 1;

    for (int i = 1; buffer[i] != '\0'; ++i) {
        if (lastChar == buffer[i]) {
            count++;
        } else {
            int written = fprintf(outFile, "%c%d", lastChar, count);
            if (written == -1) {
                exit(EXIT_FAILURE);
            }
            *writeCount += written;
            lastChar = buffer[i];
            count = 1;
        }
    }

    fprintf(outFile, "\n");
    *writeCount += 1;
}

int main(int argc, char *argv[]) {
    const char *process = argv[0];
    const char *outFileName = NULL;
    int opt;

    while ((opt = getopt(argc, argv, "o:")) != -1) {
        if (opt == 'o' && outFileName == NULL) {
            outFileName = optarg;
        } else {
            usage(process);
        }
    }

    FILE *outFile = (outFileName != NULL) ? fopen(outFileName, "w") : stdout;
    if (outFile == NULL) {
        fprintf(stderr, "[%s] ERROR: An error occurred while opening file %s\n", process, outFileName);
        exit(EXIT_FAILURE);
    }

    int readCount = 0;
    int writeCount = 0;

    if (argc - optind == 0) {
        char *line = NULL;
        size_t size = 0;

        while (getline(&line, &size, stdin) != -1) {
            compressAndWrite(line, outFile, &writeCount);
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
            compressAndWrite(line, outFile, &writeCount);
            readCount += (int) strlen(line);
        }
        free(line);

        fclose(inFile);
    }

    if (outFile != stdout) {
        fclose(outFile);
    }
    fprintf(stderr, "Written: %d characters\n", writeCount);
    fprintf(stderr, "Read: %d characters\n", readCount);

    return EXIT_SUCCESS;
}
