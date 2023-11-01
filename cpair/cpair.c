#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    float x;
    float y;
} point;

void printPoints(point *points, int size) {
    for (int i = 0; i < size; i++) {
        printf("Point %d: x = %f, y = %f\n", i, points[i].x, points[i].y);
    }
}

void strpoint(char *line, point *pt) {
    char *token = strtok(line, " ");
    pt->x = strtof(token, NULL);
    token = strtok(NULL, " ");
    pt->y = strtof(token, NULL);
}

int dynarray(point *p, point **points, int *size, int *capacity) {
    if (*size >= *capacity) {
        *capacity *= 2;
        point *newPoints = (point *)realloc(*points, (*capacity) * sizeof(point));

        if (newPoints == NULL) {
            free(*points);  // Free the original memory if realloc fails
            return -1;
        }

        *points = newPoints;
    }

    (*points)[*size] = *p;
    (*size)++;

    return 0;
}


int main(void) {
    char *line = NULL;
    size_t size = 0;
    point myPoint;
    point *points[1];
    int stored = 0;
    int capacity = 2; // Initialize the capacity

    while (getline(&line, &size, stdin) != -1) {
        strpoint(line, &myPoint);
        printf("%f %f\n", myPoint.x, myPoint.y);
        if (dynarray(&myPoint, &points, &stored, &capacity) == -1) {
            fprintf(stderr, "Memory reallocation failed\n");
            free(line);
            free(points);
            return 1;
        }
    }

    printf("%d\n", stored);
    printPoints(points, stored);

    free(line); // Free dynamically allocated memory
    free(points); // Free the points array
    return 0;
}

