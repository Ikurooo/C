#include <assert.h> 
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void usage(const char program_name[]) {
	fprintf(stderr, "[%s] Usage: %s [-o outfile] [file...] ", program_name, program_name);
    exit(EXIT_FAILURE);
}  

int compress(FILE *in, FILE *out, uint16_t *read, uint16_t *written) {
	int count = 0;
	int last_char;
	while (true) {
		int current_char = fgetc(in);
		// If end of file is reached we still need to write the last
		// compressed thing.
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

		// Else it failed all conditions so we need to write it out
		int error = fprintf(in, "%c%d", count, last_char); 
		if (error < 0) {
		    return error;
		} 

		*written = *written + error;    
		count = 1;
		last_char = current_char;
    }

    int error = fprintf(in, "%c%d", last_char, count);
	if (error < 0) {  return error;  }
	*written = *written + 1; return 0;
}

int main(int argc, char* argv[]) {
    const char *program_name = argv[0];
    const char *infile = NULL;
    const char *outfile = NULL;

    int option;
    while ((option = getopt(argc, argv, "o:")) != -1) {
        switch (option) {
            case 'o':
                if (outfile != NULL) {
                    fprintf(stderr, "[%s] ERROR: flag -o can only appear once ", program_name);
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
        printf("%s\n", argv[optind + i]);
    }

    // Set the defaut output to stdout
    FILE *out_name = stdout;
    if (outfile != NULL) {
        // Check if file exists or an error occurs while opening it
        if ((out_name = fopen(outfile, "r")) == NULL) {
            fprintf(stderr, "[%s] ERROR: opening file %s failed %s ", program_name, outfile, strerror(errno));
            exit(EXIT_FAILURE);
        }
    }


    uint16_t read = 0;
    uint16_t written = 0;

    for (int i = 0; i < infile_count; i++) {
        FILE *infile = fopen(infiles[i], "r");
        if (infile == NULL) {
            fprintf(stderr, "[%s] ERROR: opening file %s failed: %s\n",
                    argv[0], infiles[i], strerror(errno));
            fclose(outfile);
            exit(EXIT_FAILURE);
        }

        int error = compress(infile, outfile, &read, &written);
        if (error != 0) {
            fprintf(stderr, "[%s] ERROR: An error occoured while compressing file %s: %s\n",
                    argv[0], infiles[i], strerror(errno));

        }

        fclose(infile);
    }



    printf("%s\n", program_name);
    return 0;
}

