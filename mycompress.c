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

void strip(char *str) {
    for (int i = 0; str[i]; i++) {
        if (str[i] == '\n') {
            str[i] = '\0';
        }
    }
}

int compress(FILE *in, FILE *out, uint16_t *read, uint16_t *written) {
    char *line;
    size_t size;
    int error;
    while ((error = getline(&line, &size, in)) >= 0) {
        if (error < 0) {
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

            fprintf(out, "%d%c", count, last_char);
            count = 0;
            last_char = line[i];
        }
        fprintf(out, "\n");
    }
    return 0;
}

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
            printf("[%s] ERROR: An error occurred while opening file %s\n", program_name, outfile_name);
            exit(EXIT_FAILURE);
        }
    }

    int length = argc - optind;
    char *filenames[length];
    FILE *infile = NULL;

    // segfault uwu
    for (int i = 0; i < length; i++) {
        filenames[i] = argv[optind + i];
        printf("%s\n", filenames[i]);
    }


    uint16_t read = 0;
    uint16_t written = 0;
    const char *infile_name = NULL;
    for (int i = 0; i < length; i++) {
        infile_name = filenames[i];
        infile = fopen(infile_name, "r");
        if (infile == NULL) {
            printf("[%s] ERROR: An error occurred while opening file %s\n", program_name, infile_name);
            fclose(outfile);
            exit(EXIT_FAILURE);
        }

        int error = compress(infile, outfile, &read, &written);
        if (error < 0) {
            printf("[%s] ERROR: An error occurred while compressing file %s\n",program_name, infile_name);
            fclose(outfile);
            fclose(infile);
        }
    }


    if (length == 0) {
        infile = stdin;
        int error = compress(infile, outfile, &read, &written);
        if (error != 0) {
            printf("[%s] ERROR: An error occurred while compressing stdin [%d]\n",program_name, error);
            fclose(outfile);
            fclose(infile);
            exit(EXIT_FAILURE);
        }
    }

    if (outfile != stdout)  {
        fclose(outfile);
    }

    printf("Written: %d\n", written);
    printf("Read: %d\n", read);
    return EXIT_SUCCESS;
}
