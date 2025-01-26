#include <stdint.h>
#include <time.h>

#define BILLION 1000000000L
#define MILLION 1000000L

uint64_t system_time_ns()
{
    struct timespec spec;
    clock_gettime(CLOCK_REALTIME, &spec);

    return BILLION * spec.tv_sec + spec.tv_nsec;
}

void sleep_ns(uint64_t ns)
{
    struct timespec spec = {
        .tv_sec = 0,
        .tv_nsec = ns
    };
    nanosleep(&spec, NULL);
}
