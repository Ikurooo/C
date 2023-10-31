#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void usage(const char *program_name) {
    fprintf(stderr, "[%s] Usage: %s [-o outfile] [file...]\n", program_name, program_name);
    exit(EXIT_FAILURE);
}

int compress(FILE *in, FILE *out, uint16_t *read, uint16_t *written) {
    int count = 0;
    int last_char;

    while (true) {
        int current_char = fgetc(in);
        // If end of file is reached, we still need to write the last compressed thing.
        if (feof(in)) {
            break;
        }

        *read = *read + 1;
        if (count == 0) {
            count++;
            last_char = current_char;
            continue;
        }

        if (current_char == last_char) {
            count++;
            continue;
        }

        // Else, it failed all conditions, so we need to write it out
        int error = fprintf(out, "%c%d", count, last_char);
        if (error < 0) {
            return error;
        }

        *written = *written + error;
        count = 1;
        last_char = current_char;
    }

    int error = fprintf(out, "%c%d", last_char, count);
    if (error < 0) {
        return error;
    }
    *written = *written + error;
    return 0;
}

int main(int argc, char *argv[]) {
    const char *program_name = argv[0];
    const char *outfile = NULL;

    int option;
    while ((option = getopt(argc, argv, "o:")) != -1) {
        switch (option) {
            case 'o':
                if (outfile != NULL) {
                    fprintf(stderr, "[%s] ERROR: flag -o can only appear once\n", program_name);
                    usage(program_name);
                }
                outfile = optarg;
                break;
            case '?':
                usage(program_name);
                break;
            default:
                assert(0);
        }
    }

    int infile_count = argc - optind;
    char const *infiles[infile_count];
    for (int i = 0; i < infile_count; i++) {
        infiles[i] = argv[optind + i];
    }

    // Set the default output to stdout
    FILE *outfile_name = stdout;
    if (outfile != NULL) {
        // Check if the file exists or an error occurs while opening it
        outfile_name = fopen(outfile, "w");
        if (outfile_name == NULL) {
            fprintf(stderr, "[%s] ERROR: opening file %s failed: %s\n", program_name, outfile, strerror(errno));
            exit(EXIT_FAILURE);
        }
    }

    uint16_t read = 0;
    uint16_t written = 0;

    for (int i = 0; i < infile_count; i++) {
        FILE *infile_name = fopen(infiles[i], "r");
        if (infile_name == NULL) {
            fprintf(stderr, "[%s] ERROR: opening file %s failed: %s\n",
                    argv[0], infiles[i], strerror(errno));
            fclose(outfile_name);
            exit(EXIT_FAILURE);
        }

        int error = compress(infile_name, outfile_name, &read, &written);
        if (error != 0) {
            fprintf(stderr, "[%s] ERROR: An error occurred while compressing file %s: %s\n",
                    argv[0], infiles[i], strerror(errno));
            fclose(infile_name);
            exit(EXIT_FAILURE);
        }
        fclose(infile_name);
    }

    // Close the output file if it wasn't stdout
    if (outfile_name != stdout) {
        fclose(outfile_name);
    }

    // Print the statistics
    fprintf(stderr, "Read: %7hu characters\nWritten: %4hu characters\nCompression ratio: %4.1f%%\n",
            read, written, ((double)(written) / read) * 100.0);
    return EXIT_SUCCESS;
}
