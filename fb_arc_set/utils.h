/**
 * @file utils.h
 * @author Ivan Cankov 12219400
 * @date 12.09.2023
 * @brief OSUE Exercise 2 fb_arc_set
 * @details utilities for the generator and supervisor.
 */

#ifndef FB_ARC_SET_UTILS_H
#define FB_ARC_SET_UTILS_H

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <string.h>
#include <getopt.h>
#include <ctype.h>
#include <unistd.h>
#include <math.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <semaphore.h>
#include <limits.h>

#define SHM_NAME "/12219400_shm"
#define BUF_SIZE (25)
#define SEM_FREE "/12219400_free"
#define SEM_USED "/12219400_used"
#define SEM_MUTEX "/12219400_mutex"

typedef struct {
    long u;
    long v;
} edge;

typedef struct{
    edge list[8];
    size_t stored;
} edge_list;

typedef struct {
    edge_list data[BUF_SIZE];
    unsigned int readPos;
    unsigned int writePos;
    unsigned int terminate;
    unsigned int numberOfGenerators;
    int numOfGenerators;
    long numberOfSolutions;
} cbuf;

#endif //FB_ARC_SET_UTILS_H
