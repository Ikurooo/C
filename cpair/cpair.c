#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    float x;
    float y;
} point;

point strtop(char *input) {
    point p;

    // strtok saves a string using a static variable and if NULL is passed in continues to work with that same string
    // tokenise input
    char *x_str = strtok(input, " ");
    char *y_str = strtok(NULL, " ");
    char *excess = strtok(NULL, " ");

    if (x_str == NULL || y_str == NULL || excess != NULL) {
        fprintf(stderr, "Error: Malformed input line\n");
        exit(EXIT_FAILURE);
    } else {
        char *endptr;
        p.x = strtof(x_str, &endptr);
        if (*endptr != '\0' || endptr == x_str) {
            fprintf(stderr, "Error: Invalid float in the first part\n");
            exit(EXIT_FAILURE);
        }

        p.y = strtof(y_str, &endptr);
        if (*endptr != '\0' || endptr == y_str) {
            fprintf(stderr, "Error: Invalid float in the second part\n");
            exit(EXIT_FAILURE);
        }
    }

    return p;
}



int stdintopa(point **points, size_t *len)
{
    // Create an dynamic array to store the points in.
    size_t cap = 2;
    *points = malloc(sizeof(point) * cap);
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
        if (cap == *len)
        {
            cap *= 2;
            point *tmp = realloc(*points, sizeof(point) * cap);
            if (tmp == NULL)
            {
                free(line);
                free(*points);
                return -1;
            }
            *points = tmp;
        }


        point p = strtop(line);
        (*points)[*len] = p;
        (*len)++;
    }

    // Free the line and return with a success value.
    free(line);
    return 0;
}


int main(int argc, char *argv[]) {

    point *total_points;
    size_t len = 0;

    stdintopa(&total_points, &len);

    if (total_points == NULL)
    {
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < len; i++) {
        printf("Point %d: x = %f, y = %f\n", i + 1, total_points[i].x, total_points[i].y);
    }

    return 0;
}

