#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <math.h>
#include <limits.h>
#include <sys/stat.h>

/**
 * @brief Print a usage message to stderr and exit the process with EXIT_FAILURE.
 * @param process The name of the current process.
 */
void usage(const char *process) {
    fprintf(stderr, "[%s] USAGE: %s [-p PORT] [-i INDEX] DOC_ROOT\n", process, process);
    exit(EXIT_FAILURE);
}

/**
 * @brief Parses the port from a string into an integer.
 * @param portStr the port you would like to convert
 * @return 0 if successful -1 otherwise
 */
int parsePort(const char *portStr) {
    errno = 0;
    char *endptr;
    long port = strtol(portStr, &endptr, 10);

    if ((errno == ERANGE && (port == LONG_MAX || port == LONG_MIN)) || (errno != 0 && port == 0) ||
        endptr == portStr || *endptr != '\0' || port < 0 || port > 65535) {
        return -1;
    }

    return (int)port;
}

/**
 * @brief Validates the provided file.
 * @param file the file you would like to validate
 * @return 0 if successful -1 otherwise
 */
int validateFile(char *file) {
    return (strspn(file, "/\\:*?\"<>|") != 0 || strlen(file) > 255) ? -1 : 0;
}

/**
 * @brief Validates the provided directory and if it is valid and does not yet exist it gets created.
 * @implnote This function mutates the original string if it is deemed a valid directory!
 * @param dir the directory you would like to validate
 * @return 0 if successful -1 otherwise
 */
int validateDir(char **dir) {
    if (strpbrk(*dir, "\\:*?\"<>|.") != NULL) {
        return -1;
    }

    struct stat st = {0};
    return stat(*dir, &st);
}

// SYNOPSIS
//     server [-p PORT] [-i INDEX] DOC_ROOT
// EXAMPLE
//     server -p 1280 -i index.html ˜/Documents/my_website/
int main(int argc, char *argv[]) {
    char* portStr = NULL;
    char* index = NULL;
    char* root = NULL;

    int option;
    while ((option = getopt(argc, argv, "p:i:")) != -1) {
        switch (option) {
            case 'p':
                if (portStr != NULL) {
                    usage(argv[0]);
                }
                portStr = optarg;
                break;
            case 'i':
                if (index != NULL) {
                    usage(argv[0]);
                }
                index = optarg;
                break;
            case '?':
                usage(argv[0]);
                break;
            default:
                assert(0);
        }
    }

    if (index == NULL) {
        index = "index.html";
    }

    if (validateDir(&root) != 0) {
        fprintf(stderr, "Invalid doc-root directory name.\n");
        exit(EXIT_FAILURE);
    }

    if (validateFile(index) != 0) {
        fprintf(stderr, "Invalid index file name.\n");
        exit(EXIT_FAILURE);
    }

    if (argc - optind != 1) {
        usage(argv[0]);
    }
    root = argv[optind];

    if (portStr == NULL) {
        portStr = "80";
    }
    int port = parsePort(portStr);

    if (port == -1) {
        fprintf(stderr, "Invalid port number.\n");
        exit(EXIT_FAILURE);
    }

    const size_t bufferSize = 1024;
    const int backlog = 1;
    int serverSocket;

    struct addrinfo hints;
    struct addrinfo *results;
    struct addrinfo *record;

    memset(&hints, 0, sizeof(struct addrinfo));

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    if (getaddrinfo(NULL, portStr, &hints, &results) != 0) {
        fprintf(stderr, "Failed to translate server socket.\n");
        exit(EXIT_FAILURE);
    }

    for (record = results; record != NULL; record = record->ai_next) {
        serverSocket = socket(record->ai_family, record->ai_socktype, record->ai_protocol);
        if (serverSocket == -1) continue;
        int enable = 1;
        setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));
        if (bind(serverSocket, record->ai_addr, record->ai_addrlen) == 0) break;
        close(serverSocket);
    }

    if (record == NULL) {
        fprintf(stderr, "Failed to create or connect client socket.\n");
        exit(EXIT_FAILURE);
    }

    freeaddrinfo(results);

    if (listen(serverSocket, backlog) == -1) {
        fprintf(stderr, "Failed to start server socket listen.\n");
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}
