/**
 * @file cpair.c
 * @author Ivan Cankov 12219400 <e12219400@student.tuwien.ac.at>
 * @date 05.11.2023
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
    fprintf(stderr, "[%s] ERROR: %s does not accept any arguments.\n", process, process);
    exit(EXIT_FAILURE);
}

/**
 * @brief Prints a point to a file.
 * @details It is assumed that both parameters are valid.
 * @param file The file you intend to write to.
 * @param p The point you intend to write to the file.
 */
int ptofile(FILE *file, point *p) {
    return fprintf(file, "%.3f %.3f\n", p->x, p->y);
}

/**
 * @brief Returns the mean of the x values of a point array.
 * @details It is assumed that both parameters are valid.
 * @param points The point array you intend to calculate the mean of.
 * @param stored The amount of points in the point array.
 * @param axis The axis along which you intend to sort the points.
 * @details The axis parameter DOES NOT check for the validity of the input character.
 * It is highly advised that you double check it when using this function.
 */
float meanpx(point *points, size_t stored, char axis) {
    float sum = 0.0f;
    for (size_t i = 0; i < stored; i++) {
        sum += (axis == 'x') ? points[i].x : points[i].y;
    }
    sum /= (float)stored;
    return (float)sum;
}

/**
 * @brief Parses a string to a point.
 * @details Throws an error and exits if the string is not of a valid format.
 * @param input The string you intend to covert to a point.
 * @param process The name of the current process.
 */
point strtop(char *input, const char *process) {
    point p;

    char *x_str = strtok(input, " ");
    char *y_str = strtok(NULL, "\n");

    if (x_str == NULL || y_str == NULL) {
        error("Malformed input line", process);
    }

    char *endptr_x;
    p.x = strtof(x_str, &endptr_x);

    char *endptr_y;
    p.y = strtof(y_str, &endptr_y);

    if (*endptr_x != '\0') {
        error("Malformed input line", process);
    }

    if (*endptr_y != '\0') {
        error("Malformed input line", process);
    }

    return p;
}

/**
 * @brief Closes all ends of both child pipe
 * @details It is assumed that all necessary file descriptor duplications were done prior to calling this function
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
 * @brief Converts standard in to a point array.
 * @details Dynamically allocates memory to store the points.
 * @param points &mut A pointer to an UNINITIALISED point array.
 * @param process The name of the current process.
 * @return A signed size that indicates the size of the array
 * -1 if it was unable to allocate memory.
 */
ssize_t stdintopa(point **points, const char *process)
{
    ssize_t stored = 0;
    size_t capacity = 2;
    *points = malloc(sizeof(point) * capacity);

    if (points == NULL)
    {
        error("Failed to allocate memory", process);
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
                error("Failed to allocate memory", process);
            }
            *points = tmp;
        }

        point p = strtop(line, process);
        (*points)[stored] = p;
        stored++;
    }

    free(line);
    return stored;
}

/**
 * @brief Converts child output to an array of (max) 2 points.
 * @param file The file to which the child has written its output.
 * @param points &mut An INITIALISED array of 2 points.
 * @param process The name of the current process.
 * @return An unsigned size that indicates the amount of points written.
 */
size_t ctop(FILE *file, point points[2], const char *process) {

    size_t stored = 0;
    size_t size = 0;
    char *line = NULL;

    while((getline(&line, &size, file)) != -1) {
        points[stored] = strtop(line, process);
        stored++;
    }
    free(line);
    return stored;
}

/**
 * @brief A function that calculates the euclidean distance of 2 points.
 * @param p1 Point one
 * @param p2 Point two
 * @return The euclidean distance of of the aforementioned points.
 */
float euclidean(point p1, point p2) {
    return sqrtf((float)(pow(p2.x - p1.x, 2) + pow(p2.y - p1.y, 2)));
}

/**
 * @brief Prints a pair of points to a given file.
 * @param file The file you intend to print to.
 * @param pair &mut An array of exactly 2 points.
 */
void printpairsorted(FILE *file, point pair[2], const char *process) {
    int result;

    if ((pair[0].x == pair[1].x && pair[0].y <= pair[1].y) ||
        (pair[0].x < pair[1].x)) {
        result = ptofile(file, &pair[0]);
        if (result == -1) {
            error("Error writing to file", process);
        }

        result = ptofile(file, &pair[1]);
        if (result == -1) {
            error("Error writing to file", process);
        }
    } else {
        result = ptofile(file, &pair[1]);
        if (result == -1) {
            error("Error writing to file", process);
        }

        result = ptofile(file, &pair[0]);
        if (result == -1) {
            error("Error writing to file", process);
        }
    }
}

/**
 * @brief Sends points to 2 child processes.
 * @param points The points you intend to send to a child process.
 * @param stored The number of points stored in the points array.
 * @param axis The axis along which you intend to split the points in two.
 * @param leftWriteFile The file descriptor of the left child.
 * @param rightWriteFile The file descriptor of the right child.
 */
void ptoc(point *points, ssize_t stored, char axis, FILE *leftWriteFile, FILE *rightWriteFile) {
    float mean = meanpx(points, stored, axis);

    for (int i = 0; i < stored; i++) {
        if ((axis == 'x' && points[i].x <= mean) || (axis == 'y' && points[i].y <= mean)) {
            ptofile(leftWriteFile, &points[i]);
        } else {
            ptofile(rightWriteFile, &points[i]);
        }
    }
}

/**
 * @brief Counts the number of coordinates that are identical*.
 * @details The function does not count ALL identical coordinates.
 * @param points The array of points of which you wish to count the identical* coordinates of.
 * @param stored The number of points stored in the points array.
 * @param axis The axis along which you intend to count the identical* coordinates.
 * @return The number of identical* coordinates.
 */
int countcoordinates(point *points, ssize_t stored, char axis) {
    int count = 0;
    float sameCoordinate = (axis == 'x') ? points[0].x : points[0].y;

    for (size_t i = 0; i < stored; i++) {
        if ((axis == 'x' && points[i].x == sameCoordinate) ||
            (axis == 'y' && points[i].y == sameCoordinate)) {
            count++;
        }
    }

    return count;
}

/**
 * @brief Finds the closest points from the two sub problems.
 * @param points The array of points of the upper sub problem.
 * @param stored The amount of points stored in the points array.
 * @param mergedChildren &mut Initially an array of points of the better pair of the two sub problems.
 * @param mean The mean of the points array.
 * @param axis The axis along which you intend to merge the points.
 */
void mergefinal(point *points, ssize_t stored, point mergedChildren[2], float mean, char axis) {

    float delta = euclidean(mergedChildren[0], mergedChildren[1]);
    int storedLeftSide = 0;
    point leftSide[stored];

    for (ssize_t i = 0; i < stored; ++i) {
        // Check if point is on the left side and within the delta region based on the specified axis
        if ((axis == 'x' && points[i].x < mean && (points[i].x + delta) > mean) ||
            (axis == 'y' && points[i].y < mean && (points[i].y + delta) > mean)) {
            leftSide[storedLeftSide] = points[i];
            storedLeftSide += 1;
        }
    }

    for (ssize_t j = 0; j < stored; ++j) {
        if ((axis == 'x' && points[j].x < mean) || (axis == 'y' && points[j].y < mean) ||
            (axis == 'x' && points[j].x - mean > delta) || (axis == 'y' && points[j].y - mean > delta)) {
            // Skip all irrelevant points based on the specified axis
            continue;
        }

        for (int k = 0; k < storedLeftSide; ++k) {
            float delta2 = euclidean(points[j], leftSide[k]);
            if (delta2 < delta) {
                delta = delta2;
                mergedChildren[0] = points[j];
                mergedChildren[1] = leftSide[k];
            }
        }
    }
}

/**
 * @brief Finds the better pair of from the 2 children.
 * @param child1Points The points from the first child.
 * @param a The amount of points from the first child.
 * @param child2Points The points from the second child.
 * @param b The amount of points from the first child.
 * @param mergedChildren &mut An initially empty array of points.
 * @return -1 if both children failed 0 if everything worked fine.
 */
int mergechildren(point child1Points[2], size_t a, point child2Points[2], size_t b, point mergedChildren[2]) {
    if (a == 0  && b == 0) {
        return -1;
    }

    if (a == 0)
    {
        mergedChildren[0] = child2Points[0];
        mergedChildren[1] = child2Points[1];
        return 0;
    }

    if (b == 0)
    {
        mergedChildren[0] = child1Points[0];
        mergedChildren[1] = child1Points[1];
        return 0;
    }

    if (euclidean(child1Points[0], child1Points[1]) <=
        euclidean(child2Points[0], child2Points[1]))
    {
        mergedChildren[0] = child1Points[0];
        mergedChildren[1] = child1Points[1];
        return 0;
    } else {
        mergedChildren[0] = child2Points[0];
        mergedChildren[1] = child2Points[1];
        return 0;
    }
}

/**
 * @brief The entrypoint of the program.
 * @param argc
 * @param argv
 * @return EXIT_SUCCESS if everything worked fine else EXIT_FAILURE
 */
int main(int argc, char *argv[]) {

    const char *process = argv[0];
    if (argc != 1) {
        usage(process);
    }

    point *points;
    ssize_t stored = stdintopa(&points, process);

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
            printpairsorted(stdout, points, process);
            fflush(stdout);
            free(points);
            exit(EXIT_SUCCESS);
        default:
            break;
    }

    int sameX = countcoordinates(points, stored, 'x');
    int sameY = countcoordinates(points, stored, 'y');
    point samePoints[2];

    // Take care of the case when 2 (or more) points are identical
    if (sameX == stored && sameY == stored) {
        samePoints[0] = points[0];
        samePoints[1] = points[1];
        printpairsorted(stdout, samePoints, process);
        free(points);
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
        free(points);
        exit(EXIT_FAILURE);
    }

    if (leftChild == 0) {
        // 1 is the write end of a pipe
        // 0 is the read end of a pipe
        if (dup2(leftReadPipe[1], STDOUT_FILENO) == -1 ||
            dup2(leftWritePipe[0], STDIN_FILENO) == -1) {
            error("Error duplicating pipes", process);
            exit(EXIT_FAILURE);
        }
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
        free(points);
        exit(EXIT_FAILURE);
    }

    if (rightChild == 0) {
        // 1 is the write end of a pipe
        // 0 is the read end of a pipe
        if (dup2(rightReadPipe[1], STDOUT_FILENO) == -1 ||
            dup2(rightWritePipe[0], STDIN_FILENO) == -1) {
            error("Error duplicating pipes", process);
            exit(EXIT_FAILURE);
        }

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

        close(leftWritePipe[1]);
        close(rightWritePipe[1]);
        close(leftReadPipe[0]);
        close(rightReadPipe[0]);

        int statusLeft, statusRight;
        waitpid(leftChild, &statusLeft, 0);
        waitpid(rightChild, &statusRight, 0);
        exit(EXIT_FAILURE);
    }

    (sameX == stored) ?
    ptoc(points, stored, 'y', leftWriteFile, rightWriteFile):
    ptoc(points, stored, 'x', leftWriteFile, rightWriteFile);

    fflush(leftWriteFile);
    fflush(rightWriteFile);
    fclose(leftWriteFile);
    fclose(rightWriteFile);

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

    size_t a = ctop(leftReadFile, child1Points, process);
    size_t b = ctop(rightReadFile, child2Points, process);

    close(leftWritePipe[1]);
    close(rightWritePipe[1]);
    close(leftReadPipe[0]);
    close(rightReadPipe[0]);

    char axis = (sameX == stored) ? 'y' : 'x';
    float mean = meanpx(points, stored, axis);

    if ((mergechildren(child1Points, a, child2Points, b, mergedChildren)) != 0) {
        free(points);
        exit(EXIT_FAILURE);
    }

    mergefinal(points, stored, mergedChildren, mean, axis);

    printpairsorted(stdout, mergedChildren, process);

    free(points);
    return EXIT_SUCCESS;
}
