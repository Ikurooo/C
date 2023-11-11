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
#include "float.h"
#include "stdbool.h"

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
    return fprintf(file, "%.3f %.3f\n", p->x, p->y);
}


// TODO: update docs
/**
 * @brief Returns the mean of the x values of a point array
 * @details It is assumed that both parameters are valid
 * @param points The point array you intend to calculate the mean of
 * @param stored The amount of points in the point array
 */
float meanpx(point *points, size_t stored, char coordinate) {
    float sum = 0.0f;
    for (size_t i = 0; i < stored; i++) {
        sum += (coordinate == 'x') ? points[i].x : points[i].y;
    }
    sum /= (float)stored;
    return (float)sum;
}

/**
 * @brief Parses a string to a point
 * @details Throws an error and exits if the string is not of a valid format
 * @param input The string you intend to covert to a point
 */
point strtop(char *input) {
    point p;

    char *x_str = strtok(input, " ");
    char *y_str = strtok(NULL, "\n");

    if (x_str == NULL || y_str == NULL) {
        error("Malformed input line");
    }

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
        points[stored] = strtop(line);
        stored++;
    }
    free(line);
    return stored;
}

float euclidean(point p1, point p2) {
    return sqrtf((float)(pow(p2.x - p1.x, 2) + pow(p2.y - p1.y, 2)));
}

void printPairSorted(FILE *file, point pair[2]) {
    if (pair[0].x == pair[1].x) {
        // If x values are equal, sort based on y values
        if (pair[0].y <= pair[1].y) {
            ptofile(file, &pair[0]);
            ptofile(file, &pair[1]);
        } else {
            ptofile(file, &pair[1]);
            ptofile(file, &pair[0]);
        }
    } else {
        // If x values are different, sort based on x values
        if (pair[0].x < pair[1].x) {
            ptofile(file, &pair[0]);
            ptofile(file, &pair[1]);
        } else {
            ptofile(file, &pair[1]);
            ptofile(file, &pair[0]);
        }
    }
}

void ptoc(point *points, ssize_t stored, char coordinate, FILE *leftWriteFile, FILE *rightWriteFile) {
    float mean = meanpx(points, stored, coordinate);

    for (int i = 0; i < stored; i++) {
        if ((coordinate == 'x' && points[i].x <= mean) || (coordinate == 'y' && points[i].y <= mean)) {
            ptofile(leftWriteFile, &points[i]);
//            fprintf(stderr, "Looooooooooooooo\n");
//            ptofile(stderr, &points[i]);
        } else {
            ptofile(rightWriteFile, &points[i]);
//            fprintf(stderr, "Rooooooooooooooo\n");
//            ptofile(stderr, &points[i]);
        }
    }
}

int countCoordinates(point *points, ssize_t stored, char coordinate) {
    int count = 0;
    float sameCoordinate = (coordinate == 'x') ? points[0].x : points[0].y;

    for (size_t i = 0; i < stored; i++) {
        if ((coordinate == 'x' && points[i].x == sameCoordinate) ||
            (coordinate == 'y' && points[i].y == sameCoordinate)) {
            count++;
        }
    }

    return count;
}

int mergechildren(point child1Points[2], int a, point child2Points[2], int b, point mergedChildren[2]) {
    if (a == 0  && b == 0) {
        return -1;
    }

    if (a == 0)
    {
        // The left child retuned with no result so the right must have one
        mergedChildren[0] = child2Points[0];
        mergedChildren[1] = child2Points[1];
        return 0;
    }
    else if (b == 0)
    {
        // The right child returned with no result so the left must have one
        mergedChildren[0] = child1Points[0];
        mergedChildren[1] = child1Points[1];
        return 0;
    }

    // Both children have a result so we need to find out which one has the
    // best result.
    if (euclidean(child2Points[0], child2Points[1]) <=
        euclidean(child1Points[0], child2Points[1]))
    {
        mergedChildren[0] = child2Points[0];
        mergedChildren[1] = child2Points[1];
        return 0;
    }
    else
    {
        mergedChildren[0] = child1Points[0];
        mergedChildren[1] = child1Points[1];
        return 0;
    }
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
            printPairSorted(stderr, points);
            fprintf(stderr, "============\n");

            printPairSorted(stdout, points);
            fflush(stdout);
            free(points);
            exit(EXIT_SUCCESS);
        default:
            break;
    }

    int sameX = countCoordinates(points, stored, 'x');
    int sameY = countCoordinates(points, stored, 'y');
    point samePoints[2];

    // Take care of the case when 2 points are identical
    if (sameX == stored || sameY == stored) {
        samePoints[0] = points[0];
        samePoints[1] = points[1];
        printPairSorted(stdout, samePoints);
        exit(EXIT_SUCCESS);
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

    if (sameX == stored) {
        // Sort based on y coordinate
        ptoc(points, stored, 'y', leftWriteFile, rightWriteFile);
    } else {
        // Sort based on x coordinate
        ptoc(points, stored, 'x', leftWriteFile, rightWriteFile);
    }

    fflush(leftWriteFile);
    fflush(rightWriteFile);
    fclose(leftWriteFile);
    fclose(rightWriteFile);

    // TODO: Add error handling
    int statusLeft, statusRight;
    waitpid(leftChild, &statusLeft, 0);
    waitpid(rightChild, &statusRight, 0);

    if (WEXITSTATUS(statusLeft) == EXIT_FAILURE) {
        free(points);
        exit(EXIT_FAILURE);
    }
    if (WEXITSTATUS(statusRight) == EXIT_FAILURE) {
        free(points);
        exit(EXIT_FAILURE);
    }

    point child1Points[2];
    point child2Points[2];
    point mergedChildren[2];

    size_t a = ctop(leftReadFile, child1Points);
    size_t b = ctop(rightReadFile, child2Points);

    printPairSorted(stdout, child1Points);
    printPairSorted(stdout, child2Points);

    free(points);
    return 0;
}