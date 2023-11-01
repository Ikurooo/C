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
 */
void compress(char *input, FILE *outputFile, uint64_t *read, uint64_t *written) {
    char *output = (char *) malloc(2 * strlen(input) + 1);
//    memset(output, 0, 2 * (strlen(input) + 1)); //Setting every memory address of output to 0

    char currentChar = input[0];
    int concurrentChars = 0;
    int outputIndex = 0;

    *read = *read + strlen(input);
    for (int i = 0; i <= strlen(input); ++i) {
        if ((char) input[0] == '\n') break;

        //ignore every ASCII that is not on the Keyboard
        if ((char) input[i] != '\0' && (input[i] <= 31 || input[i] >= 127) && (char) input[i] != '\n') {
            continue;
        }
        if ((char) input[i] == EOF) break;
        if ((char) input[i] == '\n') *written = *written + 0;
        else if (i < strlen(input) && input[i] == currentChar)concurrentChars++; // ^It may throw an error. So to be safe I checked "i < strlen(input)" again
        else {
            output[outputIndex++] = currentChar;

            //Converting an int to char and adding it to the output
            char buffer[100]; //10^100 should be enough. Probably
            sprintf(buffer, "%d", concurrentChars);

            //add every digit to the output
            for (int j = 0; j < sizeof(buffer); ++j) {
                if ((char) buffer[j] == '\0') break;
                output[outputIndex++] = buffer[j];
            }

            currentChar = input[i];
            concurrentChars = 1;
        }
    }

    //Setting the last character to \0 to indicate that the array has ended. This prevents that other data from other programs is returned.
    output[outputIndex] = '\0';
    *written = *written + fprintf(outputFile, "%s\n", output);

    free(output);
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
            //If the -o option is found, then skip it so that we don't mistake it with the input file(s)
            if (strncmp(argv[i], "-o", 2) == 0) {
                i++;
                continue;
            }

            //Check if the input file exists or not and if we have permission:
            if (access(argv[i], F_OK) != 0 ||
                access(argv[i], R_OK) != 0 ||
                access(argv[i], W_OK) != 0) {
                fprintf(stderr, "[%s] ERROR: Opening file: %s. %s\n", programName, argv[i], strerror(errno));
                exit(EXIT_FAILURE);
            }

            FILE *inFile = fopen(argv[i], "r");
            char *line = NULL;
            size_t size = 0;
            while (getline(&line, &size, inFile) >= 0) {
                compress(line, out, &charsRead, &charsWritten);
            }
            if (feof(inFile)) { break;}
            fclose(inFile);
        }
    }

    //Write the amount of characters read and written into stderr:
    fprintf(stderr, "READ: %lu characters\nWritten: %lu characters\n", charsRead, charsWritten);

    fclose(out);
    exit(EXIT_SUCCESS);
}