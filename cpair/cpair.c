/**
 * @file cpair.c
 * @author Ivan Cankov 12219400 <e12219400@student.tuwien.ac.at>
 * @date 05.11.2023
 *
 * @brief A C program for finding the closest pair
 **/

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "unistd.h"
#include "errno.h"
#include "sys/wait.h"
#include "sys/types.h"
#include "math.h"

typedef struct {
    float x;
    float y;
} point;

const char *process;

/**
 * @brief Print an error message to stderr and exit the process with EXIT_FAILURE.
 */
void error(char *message) {
    fprintf(stderr, "%s ERROR: %s\n", process, message);
    exit(EXIT_FAILURE);
}

/**
 * @brief Print a usage message to stderr and exit the process with EXIT_FAILURE.
 */
void usage() {
    fprintf(stderr, "[%s] ERROR: %s does not accept any arguments.\n", process, process);
    exit(EXIT_FAILURE);
}

/**
 * @brief Prints a point to a file
 * @details It is assumed that both parameters are valid
 * @param file The file you intend to write to
 * @param p The point you intend to write to the file
 */
int ptofile(FILE *file, point *p) {
    fprintf(file, "%f %f\n", p->x, p->y);
}

/**
 * @brief Returns the mean of the x values of a point array
 * @details It is assumed that both parameters are valid
 * @param points The point array you intend to calculate the mean of
 * @param stored The amount of points in the point array
 */
float meanpx(point *points, size_t stored) {
    float sum = 0;
    for (int i = 0; i < stored; i++) {
        sum += (points)[i].x;
    }
    return sum / (float)stored;
}

/**
 * @brief Parses a string to a point
 * @details Throws an error and exits if the string is not of a valid format
 * @param input The string you intend to covert to a point
 */
point strtop(char *input) {
    point p;

    // strtok statically binds the input string and if NULL is given works on with the last string passed  in
    char *x_str = strtok(input, " ");
    char *y_str = strtok(NULL, "\n");

    if (x_str == NULL || y_str == NULL) {
        error("Malformed line");
    }

    // strtof collects every unused char in the char pointer
    char *endptr_x;
    p.x = strtof(x_str, &endptr_x);

    char *endptr_y;
    p.y = strtof(y_str, &endptr_y);

    if (*endptr_x != '\0') {
        error("Malformed input line");
    }

    if (*endptr_y != '\0') {
        error("Malformed input line");
    }

    return p;
}

/**
 * @brief Closes all ends of both child pipe
 * @details It is assumed that all necessary file descriptor duplications were done
 * prior to calling this function
 * @param rightReadPipe The right parent read pipe that you intend to close.
 * @param leftReadPipe The left parent read pipe that you intend to close.
 * @param rightWritePipe The left parent write pipe that you intend to close.
 * @param leftWritePipe The left parent write pipe that you intend to close.
 */
void closepipes(int rightReadPipe[2], int leftReadPipe[2], int rightWritePipe[2], int leftWritePipe[2]) {
    close(rightReadPipe[0]);
    close(rightReadPipe[1]);

    close(rightWritePipe[0]);
    close(rightWritePipe[1]);

    close(leftReadPipe[0]);
    close(leftWritePipe[1]);

    close(leftReadPipe[1]);
    close(leftWritePipe[0]);
}

/**
 * @brief Converts standard in to a point array
 * @details Dynamically allocates memory to store the points
 * @param points A pointer to an UNINITIALISED point array
 * @return A signed size that indicates the size of the array
 * -1 if it was unable to allocate memory
 */
ssize_t stdintopa(point **points)
{
    ssize_t stored = 0;
    size_t capacity = 2;
    *points = malloc(sizeof(point) * capacity);

    if (points == NULL)
    {
        return -1;
    }

    char *line = NULL;
    size_t linelen = 0;
    while (getline(&line, &linelen, stdin) != -1)
    {
        // Resize
        if (capacity == stored)
        {
            capacity *= 2;
            point *tmp = realloc(*points, sizeof(point) * capacity);
            if (tmp == NULL)
            {
                free(line);
                free(*points);
                return -1;
            }
            *points = tmp;
        }

        point p = strtop(line);
        (*points)[stored] = p;
        stored++;
    }

    free(line);
    return stored;
}

/**
 * @brief Converts child output to an array of (max) 2 points
 * @param file The file to which the child has written its output
 * @param points An INITIALISED array of points
 * @return An unsigned size that indicates the amount of points written
 */
size_t ctop(FILE *file, point points[2]) {

    size_t stored = 0;
    size_t size = 0;
    char *line = NULL;

    while((getline(&line, &size, file)) != -1) {
        // TODO: ask what the difference is between *points[stored] and (*points)[stored]
        points[stored] = strtop(line);
        stored++;
    }
    free(line);
    return stored;
}

int euclidean(point p1, point p2) {
    return sqrt((pow(p2.x - p1.x, 2) + pow(p2.y - p1.y, 2)));
}

int main(int argc, char *argv[]) {

    process = argv[0];
    if (argc != 1) {
        usage();
    }

    point *points;
    ssize_t stored;

    if ((stored = stdintopa(&points)) == -1)
    {
        error("Failed to allocate memory");
        exit(EXIT_FAILURE);
    }

    switch (stored) {
        case 0:
            fprintf(stderr, "[%s] ERROR: No points provided via stdin!\n", process);
            free(points);
            exit(EXIT_FAILURE);
            break;
        case 1:
            free(points);
            exit(EXIT_SUCCESS);
            break;
        case 2:
            ptofile(stdout, &points[0]);
            ptofile(stdout, &points[1]);
            fflush(stdout);
            free(points);
            exit(EXIT_SUCCESS);
        default:
            break;
    }
    // Parent writes to this
    int leftWritePipe[2];
    int rightWritePipe[2];

    // Parent reads from this
    int leftReadPipe[2];
    int rightReadPipe[2];

    if (pipe(leftWritePipe) == -1 || pipe(rightWritePipe) == -1 ||
        pipe(leftReadPipe) == -1 || pipe(rightReadPipe) == -1)
    {
        fprintf(stderr, "[%s] ERROR: Cannot pipe\n", process);
        free(points);
        exit(EXIT_FAILURE);
    }

    pid_t leftChild = fork();

    if (leftChild == -1) {
        fprintf(stderr, "[%s] ERROR: Cannot fork\n", process);
        closepipes(rightReadPipe, leftReadPipe, rightWritePipe, leftWritePipe);
        exit(EXIT_FAILURE);
    }

    if (leftChild == 0) {
        // 1 is the write end of a pipe
        // 0 is the read end of a pipe
        // TODO: error handling
        dup2(leftReadPipe[1], STDOUT_FILENO);
        dup2(leftWritePipe[0], STDIN_FILENO);

        closepipes(rightReadPipe, leftReadPipe, rightWritePipe, leftWritePipe);

        execlp(process, process, NULL);
        fprintf(stderr, "[%s] ERROR: Cannot exec: %s\n", process, strerror(errno));
        free(points);
        exit(EXIT_FAILURE);
    }

    pid_t rightChild = fork();

    if (rightChild == -1) {
        fprintf(stderr, "[%s] ERROR: Cannot fork\n", process);

        closepipes(rightReadPipe, leftReadPipe, rightWritePipe, leftWritePipe);

        exit(EXIT_FAILURE);
    }

    if (rightChild == 0) {
        // 1 is the write end of a pipe
        // 0 is the read end of a pipe
        // TODO: error handling
        dup2(rightReadPipe[1], STDOUT_FILENO);
        dup2(rightWritePipe[0], STDIN_FILENO);

        closepipes(rightReadPipe, leftReadPipe, rightWritePipe, leftWritePipe);

        execlp(process, process, NULL);
        fprintf(stderr, "[%s] ERROR: Cannot exec: %s\n", process, strerror(errno));
        free(points);
        exit(EXIT_FAILURE);
    }

    // 1 is the write end of a pipe
    // 0 is the read end of a pipe
    // Close off unused pipe ends (parent)

    close(leftWritePipe[0]);
    close(rightWritePipe[0]);

    close(leftReadPipe[1]);
    close(rightReadPipe[1]);

    // parent writes child reads
    FILE *leftWriteFile = fdopen(leftWritePipe[1], "w");
    FILE *rightWriteFile = fdopen(rightWritePipe[1], "w");

    // child writes parent reads
    FILE *leftReadFile = fdopen(leftReadPipe[0], "r");
    FILE *rightReadFile = fdopen(rightReadPipe[0], "r");

    if (leftReadFile == NULL || leftWriteFile == NULL ||
        rightReadFile == NULL || rightWriteFile == NULL)
    {
        fprintf(stderr, "[%s] ERROR: Cannot create file descriptor: %s\n", process, strerror(errno));

        free(points);
        closepipes(rightReadPipe, leftReadPipe, rightWritePipe, leftWritePipe);

        int statusLeft, statusRight;
        waitpid(leftChild, &statusLeft, 0);
        waitpid(rightChild, &statusRight, 0);
        exit(EXIT_FAILURE);
    }

    float mean = meanpx(points, stored);
    for (int i = 0; i < stored; i++) {
        if (points[i].x <= mean) {
            ptofile(leftWriteFile, &points[i]);
        }
        else {
            ptofile(rightWriteFile, &points[i]);
        }
    }

    fflush(leftWriteFile);
    fflush(rightWriteFile);
    fclose(leftWriteFile);
    fclose(rightWriteFile);

    // TODO: Add error handling
    int statusLeft, statusRight;
    waitpid(leftChild, &statusLeft, 0);
    waitpid(rightChild, &statusRight, 0);

    if (WEXITSTATUS(statusLeft) == EXIT_FAILURE) {exit(EXIT_FAILURE);}
    if (WEXITSTATUS(statusRight) == EXIT_FAILURE) {exit(EXIT_FAILURE);}

    // TODO: ask why this doesn't work without malloc
    point child1Points[2];
    point child2Points[2];

    size_t a = ctop(leftReadFile, child1Points);
    size_t b = ctop(rightReadFile, child2Points);

    printf("Got from left: %zu\nGot from right: %zu\n", a, b);
    for (int i = 0; i < 2; i++) {
        printf("Left at [%d]: ", i + 1);
        ptofile(stdout, &child1Points[i]);
        printf("Right at [%d]: ", i + 1);
        ptofile(stdout, &child2Points[i]);
    }

    // TODO: ask how i should combine the points cause i really don't understand

    free(points);
    return 0;
}

