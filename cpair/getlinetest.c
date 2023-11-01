#include <stdio.h>
#include <stdlib.h>

typedef struct {
    float x;
    float y;
} point;

typedef struct {
    point *array;
    size_t capacity;
    size_t size;
} DynamicArray;

void printPoints(point *points, int size) {
    for (int i = 0; i < size; i++) {
        printf("Point %d: x = %f, y = %f\n", i, points[i].x, points[i].y);
    }
}

// Initialize a dynamic array
void initializeDynamicArray(DynamicArray *dynArray, size_t initialCapacity) {
    dynArray->array = (point *)malloc(initialCapacity * sizeof(point));
    dynArray->capacity = initialCapacity;
    dynArray->size = 0;
}

// Add a point to the dynamic array
void addPoint(DynamicArray *dynArray, point p) {
    if (dynArray->size >= dynArray->capacity) {
        // Resize the array (e.g., double its size)
        dynArray->capacity *= 2;
        dynArray->array = (point *)realloc(dynArray->array, dynArray->capacity * sizeof(point));
        if (dynArray->array == NULL) {
            fprintf(stderr, "Memory reallocation failed\n");
            exit(1);
        }
    }

    dynArray->array[dynArray->size] = p;
    dynArray->size++;
}

// Free memory used by the dynamic array
void freeDynamicArray(DynamicArray *dynArray) {
    free(dynArray->array);
}

int main(void) {
    char *line = NULL;
    size_t size = 0;
    point myPoint;
    DynamicArray dynArray;
    initializeDynamicArray(&dynArray, 2);

    while (getline(&line, &size, stdin) != -1) {
        if (line[0] == '\n') {
            break; // Exit on an empty line
        }
        strpoint(line, &myPoint);
        printf("%f %f\n", myPoint.x, myPoint.y);
        addPoint(&dynArray, myPoint);
    }

    printf("Number of points: %zu\n", dynArray.size);
    printPoints(dynArray.array, dynArray.size);

    // Don't forget to free memory when done
    freeDynamicArray(&dynArray);
    free(line);

    return 0;
}

