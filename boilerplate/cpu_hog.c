/*
 * cpu_hog.c - CPU-bound workload for scheduler experiments.
 *
 * Usage:
 *   /cpu_hog [seconds]
 *
 * Burns CPU continuously and reports progress every second.
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define DEFAULT_DURATION 10

/* ================= SAFE PARSE ================= */

static unsigned int parse_seconds(const char *arg, unsigned int fallback)
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
    unsigned int duration = (argc > 1)
                                ? parse_seconds(argv[1], DEFAULT_DURATION)
                                : DEFAULT_DURATION;

    time_t start = time(NULL);
    time_t last_report = start;

    volatile unsigned long long accumulator = 0;

    printf("[cpu_hog] Starting CPU workload for %u seconds\n", duration);

    while ((unsigned int)(time(NULL) - start) < duration) {

        /* heavy CPU computation */
        accumulator = accumulator * 1664525ULL + 1013904223ULL;

        time_t now = time(NULL);

        if (now != last_report) {
            last_report = now;

            printf("[cpu_hog] alive elapsed=%ld accumulator=%llu\n",
                   (long)(now - start),
                   accumulator);

            fflush(stdout);
        }
    }

    printf("[cpu_hog] Completed duration=%u accumulator=%llu\n",
           duration, accumulator);

    return 0;
}
