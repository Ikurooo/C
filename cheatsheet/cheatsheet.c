#include "stdio.h"
#include "unistd.h"
#include "string.h"
#include "stdlib.h"

typedef struct {
    float x;
    float y;
} point;

void error(char *message, const char *process) {
    fprintf(stderr, "%s ERROR: %s\n", process, message);
    exit(EXIT_FAILURE);
}

int ptofile(FILE *file, point *p) {
    return fprintf(file, "%.3f %.3f\n", p->x, p->y);
}

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


int main(int argc, char *argv[]) {

    point *points;

    ssize_t stored = stdintopa(&points, argv[0]);

    fprintf(stdout, "[]!\n");
    for (ssize_t i = 0; i < stored; ++i) {
        ptofile(stdout, &points[i]);
    }

    return EXIT_SUCCESS;
}