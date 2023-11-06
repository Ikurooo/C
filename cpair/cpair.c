#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "unistd.h"

typedef struct {
    float x;
    float y;
} point;

void usage(const char *process) {
    fprintf(stderr, "[%s] ERROR: %s does not accept any arguments.\n", process, process);
    exit(EXIT_FAILURE);
}

int ptofile(FILE *file, point *p) {
    fprintf(file, "%f %f\n", p->x, p->y);
}

float sumpx(point *points, size_t stored) {
    float sum = 0;
    for (int i = 0; i < stored; i++) {
        sum += (points)[i].x;
    }
    return sum;
}

float meanpx(point *points, size_t stored) {
    return sumpx(points, stored) / (float)stored;
}

point strtop(char *input) {
    point p;

    // strtok statically binds the input string and if NULL is given works on with the last string passed  in
    char *x_str = strtok(input, " ");
    char *y_str = strtok(NULL, "\n");

    if (x_str == NULL || y_str == NULL) {
        fprintf(stderr, "Error: Malformed input line\n");
        exit(EXIT_FAILURE);
    }

    // strtof collects every unused char in the char pointer
    char *endptr_x;
    p.x = strtof(x_str, &endptr_x);

    char *endptr_y;
    p.y = strtof(y_str, &endptr_y);

    if (*endptr_x != '\0') {
        fprintf(stderr, "Error: Malformed input line\n");
        exit(EXIT_FAILURE);
    }

    if (*endptr_y != '\0') {
        fprintf(stderr, "Error: Malformed input line\n");
        exit(EXIT_FAILURE);
    }

    return p;
}

int stdintopa(point **points, size_t *stored)
{
    // Create an dynamic array to store the points in.
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
        if (capacity == *stored)
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
        (*points)[*stored] = p;
        (*stored)++;
    }

    // Free the line and return with a success value.
    free(line);
    return 0;
}


int main(int argc, char *argv[]) {

    const char *process = argv[0];
    if (argc != 1) {
        usage(process);
    }

    point *points;
    size_t stored = 0;
    stdintopa(&points, &stored);

    if (points == NULL)
    {
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < stored; i++) {
        ptofile(stdout, &points[i]);
    }

    printf("%f\n", meanpx(points, stored));


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
            free(points);
            exit(EXIT_SUCCESS);
        default:
            break;
    }

    int leftWritePipe[2];
    int rightWritePipe[2];
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
        close(leftWritePipe[0]);
        close(leftWritePipe[1]);

        close(leftReadPipe[0]);
        close(leftReadPipe[1]);

        close(rightWritePipe[0]);
        close(rightWritePipe[1]);

        close(rightReadPipe[0]);
        close(rightReadPipe[1]);

        exit(EXIT_FAILURE);
    }

    free(points);
    return 0;
}

