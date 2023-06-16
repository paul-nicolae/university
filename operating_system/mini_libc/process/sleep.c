#include <unistd.h>
#include <time.h>
#include <errno.h>

unsigned int sleep(unsigned int seconds)
{
    struct timespec req, rem;

    req.tv_sec = seconds;
    req.tv_nsec = 0;

    nanosleep(&req, &rem);

    // return the number of seconds left to sleep
    return rem.tv_sec;
}