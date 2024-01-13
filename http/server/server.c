#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <limits.h>
#include <sys/stat.h>
#include <stdbool.h>

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
    if (file == NULL) return 0;
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

int checkDocumentExistence(char *fullPath) {
    return (access(fullPath, F_OK) == -1) ? 404 : 0;
}

int getFullPath(const char *path, const char *root, char *fullPath, size_t maxLength) {
    size_t requiredLength = strlen(path) + strlen(root) + 2;
    if (requiredLength > maxLength) {
        return -1;
    }

    memset(fullPath, 0, sizeof(maxLength));
    strcpy(fullPath, root);
    strcat(fullPath, path);

    if (checkDocumentExistence(fullPath) != 0) {
        return -1;
    }

    return 0;
}

// FREE THE CHILDREN FROM THE CURSE OF A MEMORY LEAK
int validateRequest(char *request, char **path, char *index, char *root) {
    int response = 200;

    char *type = strtok(request, " ");
    *path = strtok(NULL, " ");
    char *protocol = strtok(NULL, " ");

    if (strncmp(protocol, "HTTP/1.1", 8) != 0 ) {
        response = 400;
    }

    if (strncmp(type, "GET", 3) != 0) {
        response = 501;
    }

    if (strncmp(*path, "/", 1) == 0 && strlen(*path) == 1) {
        *path = strdup(index);
    }

    size_t maxLength = strlen(index) + strlen(*path) + 2;
    char fullPath[maxLength];
    if (getFullPath(*path, root, fullPath, maxLength) != 0) {
        response = 404;
    } else {
        *path = strdup(fullPath);
    }

    return response;
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

    if (argc - optind != 1) {
        usage(argv[0]);
    }
    root = argv[optind];

    if (validateDir(&root) != 0) {
        fprintf(stderr, "Invalid doc-root directory name.\n");
        exit(EXIT_FAILURE);
    }

    if (validateFile(index) != 0) {
        fprintf(stderr, "Invalid index file name.\n");
        exit(EXIT_FAILURE);
    }

    if (index == NULL) {
        index = "/index.html";
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

    const size_t bufferSize = 1024; // TODO: think about this
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

    bool quit = false;
    while (1) {
        int clientSocket;
        struct sockaddr clientAddress;
        socklen_t clientAddressLength = sizeof(clientAddress);

        if ((clientSocket = accept(serverSocket, &clientAddress, &clientAddressLength)) < 0) {
            fprintf(stderr, "Failed to accept client socket.\n");
            exit(EXIT_FAILURE);
        }

        char buffer[bufferSize];

        if ((recv(clientSocket, buffer, sizeof(buffer), 0)) == -1) {
            fprintf(stderr, "Failed to receive message.\n");
            exit(EXIT_FAILURE);
        }

        FILE *writeFile = fdopen(clientSocket, "r+");
        if(writeFile == NULL){
            close(serverSocket);
            fprintf(stderr, "Error fdopen failed\n");
            exit(EXIT_FAILURE);
        }

        printf("%s\n", buffer);

        char **path = malloc(sizeof(buffer));
        switch (validateRequest(buffer, path, index, root)) {
            case 400:
                if(fprintf(writeFile, "HTTP/1.1 400 Bad Request.\r\nConnection: close\r\n\r\n") == -1){
                    fprintf(stderr, "Error writing to client.\n");
                }
                fflush(writeFile);
                break;
            case 404:
                if(fprintf(writeFile, "HTTP/1.1 404 Not Found.\r\nConnection: close\r\n\r\n") == -1){
                    fprintf(stderr, "Error writing to client.\n");
                }
                fflush(writeFile);
                break;
            case 501:
                if(fprintf(writeFile, "HTTP/1.1 501 Not Implemented.\r\nConnection: close\r\n\r\n") == -1){
                    fprintf(stderr, "Error writing to client.\n");
                }
                fflush(writeFile);
                break;
            case 200:
                if(fprintf(writeFile, "HTTP/1.1 200 OK.\r\nConnection: close\r\n\r\n") == -1){
                    fprintf(stderr, "Error writing to client.\n");
                }
                FILE *readFile = fopen(*path, "r");
                if (readFile == NULL) {
                    fprintf(stderr, "Error opening file.\n");
                }
                size_t read = 0;
                while((read = fread(buffer, sizeof(char), bufferSize, readFile)) != 0){
                    fwrite(buffer, sizeof(char), read, writeFile);
                }
                fflush(writeFile);
                break;
            default:
                assert(0);
        }

        fclose(writeFile);
        free(path);
    }

    exit(EXIT_SUCCESS);
}
