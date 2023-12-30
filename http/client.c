#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <zlib.h>


/**
 * @brief Print an error message to stderr and exit the process with EXIT_FAILURE.
 * @param process The name of the current process.
 */
void error(char *message, const char *process) {
    fprintf(stderr, "%s ERROR: %s\n", process, message);
    exit(EXIT_FAILURE);
}

/**
 * @brief Print a usage message to stderr and exit the process with EXIT_FAILURE.
 * @param process The name of the current process.
 */
void usage(const char *process) {
    fprintf(stderr, "[%s] USAGE: %s [-p PORT] [-o FILE | -d DIR] URL\n", process, process);
    exit(EXIT_FAILURE);
}

int parsePort(const char *portStr) {
    char *endptr;
    errno = 0;  // Set errno to 0 before the call to strtol to detect errors properly

    long port = strtol(portStr, &endptr, 10);

    // Check for errors during conversion
    if ((errno == ERANGE && (port == LONG_MAX || port == LONG_MIN)) || (errno != 0 && port == 0)) {
        return -1;  // Error during conversion
    }

    if (endptr == portStr || *endptr != '\0') {
        return -1;
    }

    return (int)port;  // Return the parsed port as an integer
}

int parseUrl(const char *url) {

}

// SYNOPSIS
//       client [-p PORT] [ -o FILE | -d DIR ] URL
// EXAMPLE
//       client http://www.example.com/
int main(int argc, char *argv[]) {
    int port = 80;
    const char *path;
    bool portSet = false;
    bool fileSet = false;
    bool dirSet = false;

    int option;
    while ((option = getopt(argc, argv, "p:o:d:")) != -1) {
        switch (option) {
            case 'p':
                if (portSet) {
                    usage(argv[0]);
                }
                portSet = true;
                port = parsePort(optarg);
                break;
            case 'o':
                if (dirSet || fileSet) {
                    usage(argv[0]);
                }
                fileSet = true;
                path = optarg;
                break;
            case 'd':
                if (dirSet || fileSet) {
                    usage(argv[0]);
                }
                dirSet = true;
                path = optarg;
                break;
            case '?':
                usage(argv[0]);
                break;
            default:
                assert(0);
        }
    }


    printf("%d", port);
}
