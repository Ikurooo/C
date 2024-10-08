/**
 * @file client.c
 * @author Ivan Cankov 12219400 <e12219400@student.tuwien.ac.at>
 * @date 07.01.2024
 * @brief A simple HTTP client in C
 **/

#include "../util.h"

typedef struct {
    char *file;
    char *host;
    int success;
} URI;

/**
 * @brief Print a usage message to stderr and exit the process with EXIT_FAILURE.
 * @param process The name of the current process.
 */
void usage(const char *process) {
    fprintf(stderr, "[%s] USAGE: %s [-p PORT] [-d DIRECTORY | -o OUTPUT FILE] DOMAIN\n", process, process);
    exit(EXIT_FAILURE);
}

/**
 * @brief Parses the the provided URL into a URI struct.
 * @param url the URL you would like to parse
 * @return the uri itself, if the conversion was successful the uri.success value will be 0
 */
URI parseUrl(const char *url) {

    URI uri = {
        .file = NULL,
        .host = NULL,
        .success = -1
    };

	int hostOffset = 0;
	
    if (strncasecmp(url, "http://", 7) == 0) {
        hostOffset = 7;
    }
    
    if (strncasecmp(url, "https://", 8) == 0) {
        hostOffset = 8;
    }
    
    if ((strlen(url) - hostOffset) == 0) {
        return uri;
    }

    char* s = strpbrk(url + hostOffset, ";/?:@=&");
	int fileLength = -1;
    
    if (s == NULL) {
        if (asprintf(&uri.file, "/") == -1) {
            return uri;
        }
        fileLength = 0;
    } else if (s[0] != '/') {
        if (asprintf(&uri.file, "/%s", s) == -1) {
            return uri;
        }
        fileLength = strlen(uri.file);
    } else {
        if (asprintf(&uri.file, "%s", s) == -1) {
            return uri;
        }
        fileLength = strlen(uri.file);
    }

	// strlen(uri.file is messing me up)
    if (asprintf(&uri.host, "%.*s", (int)(strlen(url) - hostOffset - fileLength), (url + hostOffset)) == -1) {
        free(uri.file);
        return uri;
    }

    if (strlen(uri.host) == 0) {
        free(uri.host);
        free(uri.file);
        return uri;
    }

    uri.success = 0;
    return uri;
}

/**
 * @brief Validates the provided directory and if it is valid and does not yet exist it gets created.
 * @implnote This function mutates the original string if it is deemed a valid directory!
 * @param dir the directory you would like to validate
 * @return 0 if successful -1 otherwise
 */
int validateDir(char **dir, URI uri) {
    if (strpbrk(*dir, "/\\:*?\"<>|.") != NULL) {
        return -1;
    }

    struct stat st = {0};

    if (stat(*dir, &st) == -1) {
        mkdir(*dir, 0777);
    }

    char *tempDir = NULL;

    if (strncmp(uri.file, "/", 1) == 0) {
        asprintf(&tempDir, "%s/index.html", *dir);
    } else {
        asprintf(&tempDir, "%s%s", *dir, uri.file);
    }

    *dir = tempDir;
    
    return 0;
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
 * @brief Validates the response.
 * @param protocol the protocol
 * @param status the status
 * @return
 */
int validateResponseCode(char *protocol, char *status) {
    if (strncmp(protocol, "HTTP/1.1", 8) != 0) {
        return 2;
    }

    // Check if status contains only numeric characters
    if (strspn(status, "0123456789") != strlen(status)) {
        return 2;
    }

    if (strncmp(status, "200", 3) != 0) {
        return 3;
    }

    return 0;
}

// SYNOPSIS
//       client [-p PORT] [ -o FILE | -d DIR ] URL
// EXAMPLE
//       client http://www.example.com/

/**
 * @brief Entrypoint of the programme. (Sets up and runs client)
 * @details the program uses the HTTP protocol and can accept files that are in plain text.
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, char *argv[]) {
    int port = 80;
    char *path = NULL;
    char *url = NULL;
    URI uri;

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
                if (port == -1) {
                    fprintf(stderr, "An error occurred while parsing the port.\n");
                    exit(EXIT_FAILURE);
                }
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

    int length = (int) log10(port) + 1;
    char strPort[length + 1];
    snprintf(strPort, sizeof(strPort), "%d", port);

    if (argc - optind != 1) {
        usage(argv[0]);
        fprintf(stderr, "URL is missing.\n");
    }

    url = argv[optind];
    uri = parseUrl(url);
    if (uri.success == -1) {
        fprintf(stderr, "An error occurred while parsing the URL.\n");
        exit(EXIT_FAILURE);
    }

    // source: https://www.youtube.com/watch?v=MOrvead27B4
    int clientSocket;
    struct addrinfo hints;
    struct addrinfo *results;
    struct addrinfo *record;

    memset(&hints, 0, sizeof(struct addrinfo));

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    int error;
    if ((error = getaddrinfo(uri.host, strPort, &hints, &results)) != 0) {
        free(uri.host);
        free(uri.file);
        fprintf(stderr, "Failed getting address information. [%d]\n", error);
        exit(EXIT_FAILURE);
    }

    for (record = results; record != NULL; record = record->ai_next) {
        clientSocket = socket(record->ai_family, record->ai_socktype, record->ai_protocol);
        if (clientSocket == -1) continue;
        if (connect(clientSocket, record->ai_addr, record->ai_addrlen) != -1) break;
        close(clientSocket);
        fprintf(stderr, "Failed connecting to server.\n");
        exit(EXIT_FAILURE);
    }

    freeaddrinfo(results);
    if (record == NULL) {
        free(uri.host);
        free(uri.file);
        fprintf(stderr, "Failed connecting to server.\n");
        exit(EXIT_FAILURE);
    }

    char *request = NULL;
    asprintf(&request, "GET %s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n", uri.file, uri.host);
    send(clientSocket, request, strlen(request), 0);
    free(request);
    free(uri.host);

    FILE *socketFile = fdopen(clientSocket, "r+");

    if (socketFile == NULL) {
        free(uri.file);
        close(clientSocket);
        fprintf(stderr, "ERROR opening client socket as file.\n");
        exit(EXIT_FAILURE);
    }

    char *line = NULL;
    size_t linelen = 0;

    if (getline(&line, &linelen, socketFile) == -1) {
        free(uri.file);
        close(clientSocket);
        fprintf(stderr, "ERROR parsing first line of client socket as file.\n");
        exit(2);
    }

    char *protocol = strtok(line, " ");
    char *status = strtok(NULL, " ");
    char *misc = strtok(NULL, "\r\n");

    if (protocol == NULL || status == NULL || misc == NULL) {
        free(uri.file);
        close(clientSocket);
        fprintf(stderr, "ERROR parsing first line of client socket as file.\n");
        exit(2);
    }

    int response = validateResponseCode(protocol, status);
    if (response != 0) {
        free(uri.file);
        fprintf(stderr, "%s %s\n", status, misc);
        exit(response);
    }

    if (fileSet == true) {
        if (validateFile(path) == -1) {
            free(uri.file);
            fprintf(stderr, "An error occurred while parsing the file.\n");
            exit(EXIT_FAILURE);
        }
    }

    if (dirSet == true) {
        if (validateDir(&path, uri) == -1) {
            free(uri.file);
            fprintf(stderr, "An error occurred while parsing the directory.\n");
            exit(EXIT_FAILURE);
        }
    }

    FILE *outfile = (dirSet == false && fileSet == false) ? stdout : fopen(path, "w");
    free(uri.file);

    if (dirSet == true) {
        free(path);
    }

    if (outfile == NULL)  {
        free(line);
        fclose(socketFile);
        close(clientSocket);
        fprintf(stderr, "ERROR opening output file\n");
        exit(EXIT_FAILURE);
    }

    while (getline(&line, &linelen, socketFile) != -1) {
        if (strcmp(line, "\r\n") == 0) {
            break;
        }
        fprintf(stderr, "%s", line);
    }

    while (getline(&line, &linelen, socketFile) != -1) {
        fprintf(outfile, "%s", line);
    }

    free(line);
    fflush(socketFile);
    fclose(socketFile);
    close(clientSocket);
    exit(EXIT_SUCCESS);
}
