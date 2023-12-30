#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdbool.h>


void stripnewline(char *line) {
    size_t linelen = strlen(line);
    if (linelen > 0 && line[linelen - 1] == '\n') {
        line[linelen - 1] = '\0';
    }
}

ssize_t filetostrarray(FILE *file, char ***strings) {
    ssize_t stored = 0;
    ssize_t capacity = 2;
    *strings = malloc(sizeof(char *) * capacity);

    if (*strings == NULL) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }

    char *line = NULL;
    size_t linelen = 0;
    while (getline(&line, &linelen, file) != -1) {
        stripnewline(line);
        (*strings)[stored] = line;

        if ((*strings)[stored] == NULL) {
            perror("Memory allocation failed");
            free(line);
            free(strings);
            exit(EXIT_FAILURE);
        }

        stored += 1;

        if (stored == capacity) {
            capacity *= 2;
            *strings = realloc(*strings, sizeof(char *) * capacity);

            if (*strings == NULL) {
                perror("Memory reallocation failed");
                free(line);
                free(strings);
                exit(EXIT_FAILURE);
            }
        }
    }
    free(line);
    return stored;
}

int writetofile(FILE *left, FILE *right, char ***strings, ssize_t stored) {
    for (ssize_t i = 0; i < stored / 2; ++i) {
        if (fprintf(left, "%s\n", (*strings)[i]) == -1) {
            return -1;
        }
    }

    for (ssize_t i = stored / 2; i < stored; ++i) {
        if (fprintf(right, "%s\n", (*strings)[i]) == -1) {
            return -1;
        }
    }

    return 0;
}

void closepipes(int leftWritePipe[2], int rightWritePipe[2], int leftReadPipe[2], int rightReadPipe[2]) {
    close(leftWritePipe[0]);
    close(leftWritePipe[1]);

    close(rightWritePipe[0]);
    close(rightWritePipe[1]);

    close(leftReadPipe[0]);
    close(leftReadPipe[1]);

    close(rightReadPipe[0]);
    close(rightReadPipe[1]);
}

int main(int argc, char *argv[]) {

    switch (argc) {
        case 1:
            break;
        default:
            perror("[%s] ERROR: %s does not take any arguments!");
            exit(EXIT_FAILURE);
    }

    char **strings;
    ssize_t stored = filetostrarray(stdin, &strings);

    switch (stored) {
        case 0:
            free(strings);
            exit(EXIT_FAILURE);
            break;
        case 1:
            fprintf(stdout, "%s\n", strings[0]);
            fflush(stdout);
            free(strings);
            exit(EXIT_SUCCESS);
            break;
        default:
            break;
    }

    // Parent writes to this.
    int leftWritePipe[2];
    int rightWritePipe[2];

    // Parent reads from this.
    int leftReadPipe[2];
    int rightReadPipe[2];

    if (pipe(leftWritePipe) == -1 || pipe(rightWritePipe) == -1 ||
        pipe(leftReadPipe) == -1 || pipe(rightReadPipe) == -1) {

        free(strings);
        perror("Failed creating pipes.");
        exit(EXIT_FAILURE);
    }

    pid_t leftChild;
    switch (leftChild = fork()) {
        case -1:
            free(strings);
            perror("Left child failed to fork.");
            exit(EXIT_FAILURE);
            break;
        case 0:
            // 1 is the write end of a pipe
            // 0 is the read end of a pipe
            if (dup2(leftWritePipe[0], STDIN_FILENO) == -1 ||
                dup2(leftReadPipe[1], STDOUT_FILENO) == -1) {

                free(strings);
                perror("Failed to duplicate file descriptors.");
                exit(EXIT_FAILURE);
            }

            closepipes(leftWritePipe, rightWritePipe, leftReadPipe, rightReadPipe);
            execlp(argv[0], argv[0], NULL);

            perror("Failed to exec.");
            free(strings);
            exit(EXIT_FAILURE);
    }

    pid_t rightChild;
    switch (rightChild = fork()) {
        case -1:
            free(strings);
            perror("Right child failed to fork.");
            exit(EXIT_FAILURE);
            break;
        case 0:
            // 1 is the write end of a pipe
            // 0 is the read end of a pipe
            if (dup2(rightWritePipe[0], STDIN_FILENO) == -1 ||
                dup2(rightReadPipe[1], STDOUT_FILENO) == -1) {

                free(strings);
                perror("Failed to duplicate file descriptors.");
                exit(EXIT_FAILURE);
            }

            closepipes(leftWritePipe, rightWritePipe, leftReadPipe, rightReadPipe);
            execlp(argv[0], argv[0], NULL);

            perror("Failed to exec.");
            free(strings);
            exit(EXIT_FAILURE);
    }

    // 1 is the write end of a pipe
    // 0 is the read end of a pipe
    close(leftReadPipe[1]);
    close(leftWritePipe[0]);

    close(rightReadPipe[1]);
    close(rightWritePipe[0]);

    FILE *leftWriteFile = fdopen(leftWritePipe[1], "w");
    FILE *rightWriteFile = fdopen(rightWritePipe[1], "w");

    FILE *leftReadFile = fdopen(leftReadPipe[0], "r");
    FILE *rightReadFile = fdopen(leftReadPipe[0], "r");

    if (leftWriteFile == NULL || rightWriteFile == NULL ||
        leftReadFile == NULL || rightReadFile == NULL) {

        free(strings);

        close(leftReadPipe[0]);
        close(leftWritePipe[1]);

        close(rightReadPipe[0]);
        close(rightWritePipe[1]);

        perror("Error opening file descriptors.");
        exit(EXIT_FAILURE);
    }

    writetofile(leftWriteFile, rightWriteFile, &strings, stored);
    fflush(leftWriteFile);
    fflush(rightWriteFile);
    fclose(leftWriteFile);
    fclose(rightWriteFile);

    int statusLeft;
    int statusRight;

    waitpid(leftChild, &statusLeft, 0);
    waitpid(rightChild, &statusRight, 0);

    if (WIFEXITED(statusLeft) == true && WIFEXITED(statusRight) == true) {
        if (WEXITSTATUS(leftChild) == EXIT_FAILURE || WEXITSTATUS(rightChild) == EXIT_FAILURE) {
            free(strings);

            close(leftReadPipe[0]);
            close(leftWritePipe[1]);

            close(rightReadPipe[0]);
            close(rightWritePipe[1]);

            perror("Child died.");
            exit(EXIT_FAILURE);
        }
    }

    char *leftLine = NULL;
    char *rightLine = NULL;

    size_t leftLineLen = 0;
    size_t rightLineLen = 0;

    getline(&leftLine, &leftLineLen, leftReadFile);
    getline(&rightLine, &rightLineLen, rightReadFile);


    for (ssize_t i = 0; i < stored; ++i) {


    }


    free(leftLine);
    free(rightLine);


    close(leftReadPipe[0]);
    close(leftWritePipe[1]);

    close(rightReadPipe[0]);
    close(rightWritePipe[1]);

    free(strings);
    return EXIT_SUCCESS;
}
