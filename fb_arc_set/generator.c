/**
 * @file generator.c
 * @author Ivan Cankov 12219400
 * @date 12.09.2023
 * @brief OSUE Exercise 2 fb_arc_set
 * @details Generates possible solutions that get written
 * to the shared memory created by the supervisor.
 */

#include "utils.h"
static int shmFd = -1;
static cbuf *buf = NULL;
static sem_t *semUsed = NULL;
static sem_t *semFree = NULL;
static sem_t *semMutex = NULL;
static size_t num_of_edges;
static size_t num_of_vertices;

static const char* PROGRAM_NAME;

/**
 * @brief Print an error message to the standard error output.
 *
 * This function prints an error message to the standard error output. If
 * additional error details are provided, it appends them to the message.
 *
 * @param message A message describing the error.
 * @param error_details Additional details about the error (can be NULL).
 */
static void ERROR_MSG(char *message, char *error_details) {
    if (error_details == NULL) {
        fprintf(stderr, "[%s]: %s\n", PROGRAM_NAME, message);
    } else {
        fprintf(stderr, "[%s]: %s (%s)\n", PROGRAM_NAME, message, error_details);
    }
}

/**
 * @brief Print an error message and exit the program with failure status.
 *
 * This function prints an error message using the ERROR_MSG function and then
 * exits the program with a failure status using the exit(EXIT_FAILURE) call.
 *
 * @param message A message describing the error.
 * @param error_details Additional details about the error (can be NULL).
 */
static void ERROR_EXIT(char *message, char *error_details) {
    ERROR_MSG(message, error_details);
    exit(EXIT_FAILURE);
}

/**
 * @brief Print program usage information and exit with failure status.
 *
 * This function prints program usage information to the standard error output
 * and then exits the program with a failure status using the exit(EXIT_FAILURE) call.
 */
static void USAGE() {
    fprintf(stderr, "Usage: %s EDGE1 EDGE2 ...\n", PROGRAM_NAME);
    fprintf(stderr, "Example: %s 0-1 1-2 1-3 1-4 2-4 3-6 4-3 4-5 6-0\n", PROGRAM_NAME);
    exit(EXIT_FAILURE);
}

/**
 * @brief Perform cleanup operations on program shutdown.
 *
 * This function decrements the number of generators in the shared buffer,
 * unmaps shared memory, closes file descriptors, and closes semaphores.
 */
static void shutdown() {
    if (buf != NULL) {
        buf->numOfGenerators--;
        if (munmap(buf, sizeof(*buf)) < 0) {
            ERROR_MSG("Error unmapping shared memory", strerror(errno));
        }
    }

    if (shmFd == -1) {
        if (close(shmFd) < 0) {
            ERROR_MSG("Error closing shared memory fd", strerror(errno));
        }
    }

    if (semUsed != NULL) {
        if (sem_close(semUsed) < 0) {
            ERROR_MSG("Error closing sem_used", strerror(errno));
        }
    }

    if (semFree != NULL) {
        if (sem_close(semFree) < 0) {
            ERROR_MSG("Error closing sem_free", strerror(errno));
        }
    }

    if (semMutex != NULL) {
        if (sem_post(semMutex) < 0) { // Resolve possible deadlocks
            ERROR_MSG("Error while sem_post on sem_free", strerror(errno));
        }
        if (sem_close(semMutex) < 0) {
            ERROR_MSG("Error closing sem_mutex", strerror(errno));
        }
    }
}

/**
 * @brief Wait for semaphores before writing to the shared buffer.
 *
 * This function waits for the `semFree` semaphore to ensure there is space
 * in the shared buffer. If the program is flagged for termination, it exits
 * successfully. It also waits for the `semMutex` semaphore to prevent race
 * conditions with other writers.
 */
static void writeWait() {
    if (sem_wait(semFree) < 0) {
        if (errno == EINTR) {
            exit(EXIT_SUCCESS);
        }
        ERROR_EXIT("Error while waiting for sem_free", strerror(errno));
    }
    if (buf->terminate) {
        exit(EXIT_SUCCESS);
    }
    if (sem_wait(semMutex)) {
        if (errno == EINTR) {
            exit(EXIT_SUCCESS);
        }
        ERROR_EXIT("Error while waiting for sem_mutex", strerror(errno));
    }
}

/**
 * @brief Signal that writing to the shared buffer is complete.
 *
 * This function signals the completion of writing by posting to the `semMutex`
 * semaphore and the `semUsed` semaphore to indicate data availability.
 */
static void writeSignal() {
    if (sem_post(semMutex) < 0) {
        ERROR_EXIT("Error while posting sem_mutex", strerror(errno));
    }
    if (sem_post(semUsed) < 0) {
        ERROR_EXIT("Error while posting sem_used", strerror(errno));
    }
}

/**
 * @brief Write an edge_list to the shared buffer.
 *
 * This function waits for semaphores, writes an `edge_list` to the shared buffer,
 * signals the completion of writing, and releases the semaphores.
 *
 * @param candidate The edge_list to be written to the shared buffer.
 */
static void bufferWrite(edge_list candidate) {
    writeWait();
    buf->data[buf->writePos] = candidate;
    buf->writePos = (buf->writePos + 1) % BUF_SIZE;
    writeSignal();
}

/**
 * @brief Fill an array with sequential vertex indices.
 *
 * This function fills the provided array with sequential vertex indices from 0 to
 * `num_of_vertices - 1`.
 *
 * @param vertices An array to store sequential vertex indices.
 */
static void fill_vertex_array(long vertices[]) {
    for (size_t i = 0; i < num_of_vertices; i++) {
        vertices[i] = i;
    }
}

/**
 * @brief Generate a random permutation of vertex indices.
 *
 * This function generates a random permutation of vertex indices using the Fisher-Yates
 * shuffle algorithm.
 *
 * @param vertices An array containing sequential vertex indices.
 */
static void generate_random_permutation(long vertices[]) {
    for (size_t i = num_of_vertices - 1; i > 0; i--) {
        long j = rand() % (i + 1);
        long temp = vertices[j];
        vertices[j] = vertices[i];
        vertices[i] = temp;
    }
}


/**
 * @brief Generate and buffer solutions based on random permutations of edges.
 *
 * This function generates solutions by creating random permutations of vertex indices,
 * applying a random permutation to the given edges, and buffering a solution if
 * certain conditions are met.
 *
 * @param edges An array of edges to generate solutions from.
 */
static void generate_solutions(edge edges[]) {
    while (buf->terminate == 0) {
        long random_permutation[num_of_vertices];
        fill_vertex_array(random_permutation);
        generate_random_permutation(random_permutation);

        edge_list tmp;
        size_t delete_counter = 0;
        tmp.stored = 0;

        for (size_t i = 0; i < num_of_edges && delete_counter < 7; i++) {
            size_t pos_u = random_permutation[edges[i].u];
            size_t pos_v = random_permutation[edges[i].v];

            if (pos_u > pos_v) {
                tmp.list[delete_counter++] = edges[i];
                tmp.stored = delete_counter;
            }
        }

        if (delete_counter < 7) {
            bufferWrite(tmp);
            // another funny place
        }
    }
}

/**
 * @brief Get a random seed based on the current time, clock, and process ID.
 *
 * This function generates a random seed by combining the current time, clock, and
 * process ID. It can be used for seeding random number generation functions.
 *
 * @return An unsigned integer representing the random seed.
 */
static unsigned int get_random_seed() {
    return time(NULL) * clock() * getpid();
}


/**
 * @brief Perform startup operations for the generator process.
 *
 * This function sets up cleanup operations, opens shared memory, maps shared
 * memory, closes file descriptors, and opens semaphores for the generator process.
 */
static void startup() {
    if (atexit(shutdown) < 0) {
        ERROR_EXIT("Error setting cleanup function", NULL);
    }

    shmFd = shm_open(SHM_NAME, O_RDWR, 0600);
    if (shmFd < 0) {
        if (errno == ENOENT) {
            ERROR_MSG("Supervisor has to be started first!", NULL);
        }
        ERROR_EXIT("Error opening shared memory", strerror(errno));
    }

    // map shared memory object
    buf = mmap(NULL, sizeof(*buf), PROT_READ | PROT_WRITE, MAP_SHARED, shmFd, 0);
    if (buf == MAP_FAILED) {
        ERROR_EXIT("Error mapping shared memory", strerror(errno));
    }

    if (close(shmFd) < 0) {
        ERROR_EXIT("Error closing shared memory fd", strerror(errno));
    }
    shmFd = -1;

    // open semaphores
    semUsed = sem_open(SEM_USED, 0);
    if (semUsed == SEM_FAILED) {
        ERROR_EXIT("Error opening used", strerror(errno));
    }

    semFree = sem_open(SEM_FREE, 0);
    if (semFree == SEM_FAILED) {
        ERROR_EXIT("Error opening free", strerror(errno));
    }

    semMutex = sem_open(SEM_MUTEX, 0);
    if (semMutex == SEM_FAILED) {
        ERROR_EXIT("Error opening mutex", strerror(errno));
    }

    buf->numOfGenerators++;
}

/**
 * @brief Parse an input string to extract edge information.
 *
 * This function parses an input string to extract edge information. It handles
 * vertex indices and validates the input format.
 *
 * @param input The input string containing vertex and edge information.
 * @return An edge structure representing the parsed information.
 */
static edge parseEdge(const char *input) {
    char *tmp = strdup(input);

    if (tmp == NULL) {
        ERROR_EXIT("Error duplicating string", strerror(errno));
    }

    // parse first vertex
    char *endptr;
    char *vertex1 = tmp;
    long u = strtol(vertex1, &endptr, 0);

    if (endptr == vertex1) {
        fprintf(stderr, "[%s]: Invalid vertex index ('%s' is not a number)\n", PROGRAM_NAME, vertex1);
        free(tmp);
        USAGE();
    }

    if (u == LONG_MIN || u == LONG_MAX) {
        free(tmp);
        ERROR_EXIT("Overflow occurred while parsing vertex index", strerror(errno));
    }

    if (endptr[0] != '-') {
        fprintf(stderr, "[%s]: Invalid vertex delimiter '%c' (has to be '-')\n", PROGRAM_NAME, endptr[0]);
        free(tmp);
        USAGE();
    }

    if (u < 0) {
        fprintf(stderr, "[%s]: Negative vertex index %ld not allowed\n", PROGRAM_NAME, u);
        free(tmp);
        USAGE();
    }

    // shift string pointer by one
    char *vertex2 = endptr + 1;
    long v = strtol(vertex2, &endptr, 0);
    if (endptr == vertex2) {
        fprintf(stderr, "[%s]: Invalid vertex index ('%s' is not a number)\n", PROGRAM_NAME, vertex2);
        free(tmp);
        USAGE();
    }

    if (v == LONG_MIN || v == LONG_MAX) {
        free(tmp);
        ERROR_EXIT("Overflow occurred while parsing vertex index", strerror(errno));
    }

    if (endptr[0] != '\0') {
        fprintf(stderr, "[%s]: Invalid edge delimiter '%c' (has to be ' ')\n", PROGRAM_NAME, endptr[0]);
        free(tmp);
        USAGE();
    }

    if (v < 0) {
        fprintf(stderr, "[%s]: Negative vertex index %ld not allowed\n", PROGRAM_NAME, v);
        free(tmp);
        USAGE();
    }

    free(tmp);

    // update number of vertices
    if (u + 1 > num_of_vertices) {
        num_of_vertices = u + 1;
    }

    if (v + 1 > num_of_vertices) {
        num_of_vertices = v + 1;
    }

    edge e = {.u = u, .v = v};
    return e;
}

/**
 * @brief Parse command line input to extract edge information.
 *
 * This function parses the command line input to extract edge information. It
 * uses the parseEdge function to handle individual edges.
 *
 * @param argc The number of command line arguments.
 * @param argv An array of command line argument strings.
 * @param edges An array to store parsed edge information.
 */
static void parseInput(int argc, const char **argv, edge edges[]) {
    if (argc < 2) {
        USAGE();
    }

    // parse input
    for (size_t i = 1; i < argc; i++) {
        edges[i - 1] = parseEdge(argv[i]);
    }
}

/**
 * entrypoint
 * @param argc
 * @param argv
 * @return EXIT_SUCCESS if all went well else EXIT_FAILURE
 */
int main(int argc, const char** argv) {
    PROGRAM_NAME = argv[0];

    // initialize resources
    startup();

    // parse input
    num_of_edges = argc-1;
    edge edges[num_of_edges];
    parseInput(argc, argv, edges);

    // generate solution
    srand(get_random_seed());
    generate_solutions(edges);

    exit(EXIT_SUCCESS);
}