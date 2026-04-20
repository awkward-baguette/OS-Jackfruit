/*
 * io_pulse.c - I/O-oriented workload for scheduler experiments.
 *
 * Usage:
 *   /io_pulse [iterations] [sleep_ms]
 *
 * Writes small bursts to a file and sleeps between writes.
 * Simulates I/O-bound workload.
 */

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define DEFAULT_OUTPUT "/tmp/io_pulse.out"
#define DEFAULT_ITERATIONS 20
#define DEFAULT_SLEEP_MS 200

/* ================= SAFE PARSE ================= */

static unsigned int parse_uint(const char *arg, unsigned int fallback)
{
    char *end = NULL;
    unsigned long value = strtoul(arg, &end, 10);

    if (!arg || *arg == '\0' || (end && *end != '\0') || value == 0)
        return fallback;

    return (unsigned int)value;
}

/* ================= MAIN ================= */

int main(int argc, char *argv[])
{
    unsigned int iterations = (argc > 1) ? parse_uint(argv[1], DEFAULT_ITERATIONS) : DEFAULT_ITERATIONS;
    unsigned int sleep_ms = (argc > 2) ? parse_uint(argv[2], DEFAULT_SLEEP_MS) : DEFAULT_SLEEP_MS;

    int fd = open(DEFAULT_OUTPUT, O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (fd < 0) {
        perror("open failed");
        return 1;
    }

    printf("[io_pulse] Starting I/O workload: iterations=%u sleep=%u ms\n",
           iterations, sleep_ms);

    for (unsigned int i = 0; i < iterations; i++) {

        char buffer[128];
        int len = snprintf(buffer, sizeof(buffer),
                           "io_pulse iteration=%u\n", i + 1);

        if (len < 0 || write(fd, buffer, len) != len) {
            perror("write failed");
            close(fd);
            return 1;
        }

        /* force disk write (simulate real I/O) */
        fsync(fd);

        printf("[io_pulse] wrote iteration=%u\n", i + 1);
        fflush(stdout);

        usleep(sleep_ms * 1000U);
    }

    close(fd);

    printf("[io_pulse] Completed successfully\n");

    return 0;
}
