#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "unistd.h"

typedef struct {
    float x;
    float y;
} point;

int ptofile(FILE *file, point *p) {
    fprintf(file, "%f %f\n", p->x, p->y);
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

    // Read stdin line by line.
    char *line = NULL;
    size_t linecap = 0;
    while (getline(&line, &linecap, stdin) != -1)
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

    return 0;
}

