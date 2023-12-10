/**
 * @file supervisor.c
 * @author Ivan Cankov 12219400
 * @date 12.09.2023
 * @brief OSUE Exercise 2 fb_arc_set
 * @details The supervisor program creates the shared memory
 * where the generators will write and reads from said shared
 * memory
 */


#include "utils.h"

static int shmFd = -1;
static cbuf *buf = NULL;
static sem_t *semUsed = NULL;
static sem_t *semFree = NULL;
static sem_t *semMutex = NULL;

static const char* PROGRAM_NAME;

/**
 * @bried Prints an error message to stdout.
 *
 * @param message the message you wish to print
 * @param error the error message from the implementation of libraries
 */
static void ERROR_MSG(char *message, char *error) {
    if (error == NULL) {
        fprintf(stderr, "[%s]: %s\n", PROGRAM_NAME, message);
    } else {
        fprintf(stderr, "[%s]: %s (%s)\n", PROGRAM_NAME, message, error);
    }
}
/**
 * @brief Exit the program with an error message.
 *
 * This function prints an error message to the standard error output using the
 * ERROR_MSG macro and then exits the program with a failure status using the
 * exit(EXIT_FAILURE) call.
 *
 * @param message A message describing the error.
 * @param error Additional information about the error.
 */
static void ERROR_EXIT(char *message, char *error) {
    ERROR_MSG(message, error);
    exit(EXIT_FAILURE);
}

/**
 * @brief Print the program usage and exit.
 *
 * This function prints a usage message to the standard error output and then
 * exits the program with a failure status using the exit(EXIT_FAILURE) call.
 */
static void USAGE() {
    fprintf(stderr, "Usage: %s\n", PROGRAM_NAME);
    exit(EXIT_FAILURE);
}


/**
 * @brief Signal handler function to handle termination signal.
 *
 * This function is a signal handler that sets the termination flag in the
 * shared memory buffer when a termination signal is received.
 *
 * @param signal The signal number that triggered the handler.
 */
static void handleSignal(int signal) {
    buf->terminate = 1;
}

/**
 * @brief Perform cleanup and shutdown operations.
 *
 * This function performs cleanup operations, such as setting the termination
 * flag, releasing semaphores, closing file descriptors, and unlinking shared
 * memory, in preparation for program termination.
 */
static void shutdown() {
    if (buf != NULL) {
        buf->terminate = 1;

        // Stop all waiting generators from waiting
        if (semFree != NULL) {
            for (size_t i = 0; i < buf->numOfGenerators; i++) {
                if (sem_post(semFree) < 0) {
                    ERROR_MSG("Error while sem_post for sem_free", strerror(errno));
                }
            }
        }
    }

    if (shmFd != -1) {
        if (close(shmFd) < 0) {
            ERROR_MSG("Error closing shared memory fd", strerror(errno));
        }
        shmFd = -1;
    }

    if (semUsed != NULL) {
        if (sem_close(semUsed) < 0) {
            ERROR_MSG("Error closing sem_used", strerror(errno));
        }
        if (sem_unlink(SEM_USED) < 0 && errno != ENOENT) {
            // Only print the error if it's not "No such file or directory"
            ERROR_MSG("Error unlinking SEM_USED", strerror(errno));
        }
    }

    if (semFree != NULL) {
        if (sem_close(semFree) < 0) {
            ERROR_MSG("Error closing sem_free", strerror(errno));
        }
        if (sem_unlink(SEM_FREE) < 0 && errno != ENOENT) {
            // Only print the error if it's not "No such file or directory"
            ERROR_MSG("Error unlinking SEM_FREE", strerror(errno));
        }
    }

    if (semMutex != NULL) {
        if (sem_close(semMutex) < 0) {
            ERROR_MSG("Error closing sem_mutex", strerror(errno));
        }
        if (sem_unlink(SEM_MUTEX) < 0 && errno != ENOENT) {
            // Only print the error if it's not "No such file or directory"
            ERROR_MSG("Error unlinking SEM_MUTEX", strerror(errno));
        }
    }

    // Unmap shared memory
    if (buf != NULL) {
        buf->terminate = 1;
        if (munmap(buf, sizeof(*buf)) < 0) {
            ERROR_MSG("Error unmapping shared memory", strerror(errno));
        }
    }

    // Unlink shared memory
    if (shm_unlink(SHM_NAME) < 0) {
        ERROR_MSG("Error unlinking shared memory", strerror(errno));
    }
}

/**
 * @brief Perform startup operations.
 *
 * This function performs startup operations, such as setting a cleanup function
 * using atexit, creating shared memory, mapping shared memory, setting signal
 * handlers, initializing the buffer, and creating semaphores.
 */
static void startup() {
    // The atexit function in C is used to register a function to be called automatically when
    // the program terminates normally. It allows you to specify a function that should be executed
    // just before the program exits.
    if(atexit(shutdown) < 0) {
        ERROR_EXIT("Error setting cleanup function", NULL);
    }

    // create shared memory
    shmFd = shm_open(SHM_NAME, O_RDWR | O_CREAT | O_EXCL, 0600);
    if (shmFd < 0) {
        ERROR_EXIT("Error creating shared memory", strerror(errno));
    }

    // In C programming, the ftruncate function is used to resize a file to a specified length.
    // This function is typically used with file descriptors and is part of the POSIX standard.
    if (ftruncate(shmFd, sizeof(cbuf)) < 0) {
        ERROR_EXIT("Error setting size of shared memory", strerror(errno));
    }

    // map shared memory object
    buf = mmap(NULL, sizeof(*buf), PROT_READ | PROT_WRITE, MAP_SHARED, shmFd, 0);
    if (buf == MAP_FAILED) {
        ERROR_EXIT("Error mapping shared memory", strerror(errno));
    }
    if (close(shmFd) < 0) {
        ERROR_MSG("Error closing shared memory fd", strerror(errno));
    }
    shmFd = -1;

    // set signal handler

    struct sigaction sa = { .sa_handler = handleSignal };
    if (sigaction(SIGINT, &sa, NULL) < 0 || sigaction(SIGTERM, &sa, NULL) < 0) {
        ERROR_EXIT("Error setting signal handler", strerror(errno));
    }

    // initialize buffer
    buf->terminate = 0;
    buf->readPos = 0;
    buf->writePos = 0;
    buf->numOfGenerators = 0;
    buf->numberOfSolutions = 0;


    // create semaphores
    semUsed = sem_open(SEM_USED, O_CREAT | O_EXCL, 0600, 0);
    if (semUsed == SEM_FAILED) {
        ERROR_EXIT("Error creating used_sem", strerror(errno));
    }
    semFree = sem_open(SEM_FREE, O_CREAT | O_EXCL, 0600, BUF_SIZE);
    if (semFree == SEM_FAILED) {
        ERROR_EXIT("Error creating sem_free", strerror(errno));
    }
    semMutex = sem_open(SEM_MUTEX, O_CREAT | O_EXCL, 0600, 1);
    if (semMutex == SEM_FAILED) {
        ERROR_EXIT("Error creating sem_mutex", strerror(errno));
    }
}

/**
 * @brief Wait for a semaphore and check for termination.
 *
 * This function waits for the `semUsed` semaphore, which signals that there is
 * data available in the shared buffer. If the semaphore wait fails, it checks
 * for the EINTR error (interrupted system call) and handles it by retrying the
 * wait. If the buffer is flagged for termination, the program exits.
 */
static void waitAndRead() {
    if (sem_wait(semUsed) < 0) {
        if (errno != EINTR) {
            ERROR_EXIT("Error while sem_wait", strerror(errno));
        }
    }
    if (buf->terminate) {
        exit(EXIT_SUCCESS);
    }
}

/**
 * @brief Signal that a read operation is complete.
 *
 * This function signals that a read operation is complete by posting to the
 * `semFree` semaphore, indicating that there is free space in the shared buffer.
 */
static void readSignal() {
    if (sem_post(semFree) < 0) {
        ERROR_EXIT("Error while sem_post", strerror(errno));
    }
}

/**
 * @brief Read an edge_list from the shared buffer.
 *
 * This function waits for a semaphore, reads an `edge_list` from the shared
 * buffer, signals the completion of the read, and returns the read `edge_list`.
 */
static edge_list readBuffer() {
    waitAndRead();
    edge_list candidate = buf->data[buf->readPos];
    buf->readPos = (buf->readPos + 1) % BUF_SIZE;
    readSignal();
    return candidate;
}

/**
 * @brief Process and print solutions from the shared buffer.
 *
 * This function continuously reads solutions from the shared buffer, increments
 * the count of solutions, and prints information about the best solutions found.
 * The function terminates when the buffer is flagged for termination or the
 * maximum number of solutions is reached.
 *
 * @param maxSolutions The maximum number of solutions to process. Use 0 for no limit.
 */
static void solutions(long maxSolutions) {
    edge_list solution = { .stored = SIZE_MAX };
    while (buf->terminate == 0 && (buf->numberOfSolutions < maxSolutions || maxSolutions == 0)) {
        edge_list candidate = readBuffer();
        buf->numberOfSolutions++;
        if (candidate.stored == 0) {
            printf("The graph is acyclic!\n");
            buf->terminate = 1;
        } else if (candidate.stored < solution.stored) {
            solution = candidate;
            printf("Solution with %zu edges:", solution.stored);
            for (size_t i = 0; i < solution.stored; i++) {
                printf(" %ld-%ld", solution.list[i].u, solution.list[i].v);
            }
            printf("\n");
        }
    }
    if (maxSolutions < buf->numberOfSolutions) {
        printf("The graph might not be acyclic, best solution removes %zu edges.", solution.stored);
    }
}

/**
 * entrypoint
 * @param argc
 * @param argv
 * @return EXIT_SUCCESS if all went well else EXIT_FAILURE
 */
int main(int argc, char *argv[]) {
    PROGRAM_NAME = argv[0];

    long nValue = 0; // Default value for n
    long wValue = 0; // Default value for w

    int opt;
    char *endptr;

    while ((opt = getopt(argc, argv, "hn:w:")) != -1) {
        switch (opt) {
            case 'h':
                USAGE();
            case 'n':
                errno = 0; // Reset errno before calling strtol
                nValue = strtol(optarg, &endptr, 10);

                // Check for conversion errors
                if (errno != 0 || *endptr != '\0') {
                    fprintf(stderr, "Invalid number for -n option\n");
                    exit(EXIT_FAILURE);
                }
                break;
            case 'w':
                errno = 0; // Reset errno before calling strtol
                wValue = strtol(optarg, &endptr, 10);

                // Check for conversion errors
                if (errno != 0 || *endptr != '\0') {
                    fprintf(stderr, "Invalid number for -w option\n");
                    exit(EXIT_FAILURE);
                }
                break;
            default:
                USAGE();
        }
    }

    startup();
    if (wValue < 0)  {
        ERROR_EXIT("value of -w should be greater than or equal to 0", strerror(errno));
    }
    sleep(wValue);

    solutions(nValue);

    return EXIT_SUCCESS;
}