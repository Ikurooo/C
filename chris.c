#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>

char *programName;

/**
 * @brief Print a usage message to stderr and exit the program with EXIT_FAILURE.
 */
void usage(void) {
    fprintf(stderr, "USAGE: %s [-o outputFile] [inputFile]\n", programName);
    exit(EXIT_FAILURE);
}

/**
 * @brief compress takes an input and tries to compress it. It then writes it into the output file.
 * @param input A pointer to a character array (string) representing the input data that needs to be compressed.
 * @param outputFile A pointer to a FILE object that represents the output file where the compressed data will be written.
 * @param read  amount of read characters including \n
 * @param written amount of written characters including \n
 * @details function does not check for newlines > manually done in main()
 */
void compress(char *input, FILE *outputFile, uint64_t *read, uint64_t *written) {

    u_char last_char;
    int count = 0;

    *read = *read + strlen(input);
    for (int i = 0; input[i]; ++i) {

        if (count == 0) {
            count = 1;
            last_char = input[i];
            continue;
        }

        if (last_char == input[i]) {
            count++;
            continue;
        }

        int printed = fprintf(outputFile, "%c%d", last_char, count);
        if (printed < 0) {
            fprintf(stderr, "[%s] ERROR: Error while writing to file\n", programName);
        }

        *written += printed;
        count = 1;
        last_char = input[i];

    }
    fprintf(outputFile, "\n");
}

/**
 * @brief Entrypoint; Main methode in which all files Pointers are opened and closed
 * @param argc
 * @param argv
 * @return EXIT_SUCCESS on success, otherwise EXIT_FAILURE.
 */
int main(int argc, char *argv[]) {
    programName = argv[0];
    const char *output_filename = NULL;
    int inputFileAmount = argc - optind;


    int option;
    while ((option = getopt(argc, argv, "o:")) != -1) {
        switch (option) {
            case 'o':
                if (output_filename != NULL) {
                    fprintf(stderr, "[%s] ERROR: flag -o can only appear once\n", programName);
                    usage();
                }
                output_filename = optarg;
                break;
            case '?':
                usage();
                break;
            default:
                assert(0);
        }
    }

    //Parameters and links
    uint64_t charsRead = 0;
    uint64_t charsWritten = 0;

    //Preparing the output file:
    FILE *out = stdout;

    //Check if the output file exists or not and if we have permission:
    if (output_filename != NULL) {
        if (access(output_filename, F_OK) != 0 ||
            access(output_filename, R_OK) != 0 ||
            access(output_filename, W_OK) != 0) {
            fprintf(stderr, "[%s] ERROR: Opening file: %s. %s\n", programName, output_filename, strerror(errno));
            exit(EXIT_FAILURE);
        }
        out = fopen(output_filename, "w");
    }

    //If no input file was specified, then read from stdin
    if (inputFileAmount == 0) {
        char *line = NULL;
        size_t size = 0;
        while ( getline(&line, &size, stdin) >= 0) {
            compress(line, out, &charsRead, &charsWritten);
        }
    } else {
        for (int i = 1; i < argc; ++i) {

            //Check if the input file exists or not and if we have permission:
            if (access(argv[optind + i], F_OK) != 0 ||
                access(argv[optind + i], R_OK) != 0 ||
                access(argv[optind + i], W_OK) != 0) {
                fprintf(stderr, "[%s] ERROR: Opening file: %s. %s\n", programName, optind + i, strerror(errno));
                exit(EXIT_FAILURE);
            }

            char *line = NULL;
            size_t size = 0;
            FILE *inFile = fopen(argv[optind + i], "r");

            while (getline(&line, &size, inFile) >= 0) {
                compress(line, out, &charsRead, &charsWritten);
            }
            fclose(inFile);
        }
    }

    //Write the amount of characters read and written into stderr:
    fprintf(stderr, "READ: %lu characters\nWritten: %lu characters\n", charsRead, charsWritten);

    if(out != stdout) {fclose(out);}
    exit(EXIT_SUCCESS);
}