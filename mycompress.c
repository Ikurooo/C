/**
 * @file mycompress.c
 * @author Ivan Cankov 12219400 <e12219400@student.tuwien.ac.at>
 * @date 31.10.2023
 *
 * @brief A C program for compressing different files.
 **/

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/**
 * @brief Print a usage message to stderr and exit the process with EXIT_FAILURE.
 *
 * @param program_name The name of the current program.
 */
void usage(const char *program_name) {
    fprintf(stderr, "[%s] Usage: %s [-o outfile] [file...]\n", program_name, program_name);
    exit(EXIT_FAILURE);
}


/**
 * @brief Compresses the input file and writes it out to the output file.
 * It additionally modifies the amount of characters read and written.
 *
 * @details It is assumed that all pointers passed in are valid pointers.
 * Will throw a segfault if invalid.
 * @param in The input file to be compressed.
 * @param out The output file to write the compressed elements to.
 * @param read Pointer to the read characters counter.
 * @param written Pointer to the written characters counter.
 * @return 0 on success, -1 otherwise.
 */
int compress(FILE *in, FILE *out, uint16_t *read, uint16_t *written) {
    char *line = NULL;
    size_t size = 0;
    int error;

    while ((error = getline(&line, &size, in)) >= 0) {
        if (error < 0) {
            free(line);
            return -1;
        }

        u_char last_char;
        int count = 0;

        for (int i = 0; line[i]; i++) {
            if (count == 0) {
                last_char = line[i];
                count = 1;
                continue;
            }

            if (line[i] == last_char) {
                count++;
                continue;
            }

            error = fprintf(out, "%c%d", last_char, count);
            if (error < 0) {
                return -1;
            }

            *read += count;
            *written += error;
            count = 1;
            last_char = line[i];
        }

        error = fprintf(out, "\n");
        if (error < 0) {
            return -1;
        }

        *read += error;
        *written += error;
        free(line);
        line = NULL;
        size = 0;
    }

    return 0;
}

/**
 * @brief Entrypoint; error handling and file opening/closing is done here.
 * @param argc
 * @param argv
 * @return EXIT_SUCCESS on success, otherwise EXIT_FAILURE.
 **/

int main(int argc, char *argv[]) {
    const char *program_name = argv[0];
    const char *outfile_name = NULL;

    int option;
    while ((option = getopt(argc, argv, "o:")) != -1) {
        switch (option) {
            case 'o':
                if (outfile_name != NULL) {
                    fprintf(stderr, "[%s] ERROR: flag -o can only appear once\n", program_name);
                    usage(program_name);
                }
                outfile_name = optarg;
                break;
            case '?':
                usage(program_name);
                break;
            default:
                assert(0);
        }
    }

    FILE *outfile = stdout;
    if (outfile_name != NULL) {
        outfile = fopen(outfile_name, "w");
        if (outfile == NULL) {
            fprintf(stderr, "[%s] ERROR: An error occurred while opening file %s\n", program_name, outfile_name);
            exit(EXIT_FAILURE);
        }
    }

    int length = argc - optind;
    char *filenames[length];

    for (int i = 0; i < length; i++) {
        filenames[i] = argv[optind + i];
    }

    uint16_t read = 0;
    uint16_t written = 0;
    FILE *infile = NULL;
    const char *infile_name = NULL;

    for (int i = 0; i < length; i++) {
        infile_name = filenames[i];
        infile = fopen(infile_name, "r");
        if (infile == NULL) {
            goto SINGLE_FILE;
        }

        int error = compress(infile, outfile, &read, &written);
        if (error < 0) {
            goto BOTH_FILES;
        }
    }

    if (length == 0) {
        infile = stdin;
        int error = compress(infile, outfile, &read, &written);
        if (error != 0) {
            fprintf(stderr, "[%s] ERROR: An error occurred while compressing stdin\n", program_name);
            goto BOTH_FILES;
        }
    }

    if (outfile != stdout) {
        fclose(outfile);
    }

    printf("Written: %d\n", written);
    printf("Read: %d\n", read);
    return EXIT_SUCCESS;

    BOTH_FILES:
    fprintf(stderr, "[%s] ERROR: An error occurred while compressing file %s\n", program_name, infile_name);
    fclose(infile);
    SINGLE_FILE:
    fclose(outfile);
    exit(EXIT_FAILURE);
}
