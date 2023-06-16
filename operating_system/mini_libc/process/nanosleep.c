#include <unistd.h>
#include <internal/syscall.h>
#include <errno.h>
#include <time.h>

int nanosleep(const struct timespec *req, struct timespec *rem)
{
    int value = syscall(__NR_nanosleep, req, rem);

    if (value < 0) {
        // if the call is interrupted or encounters an error, returns -1
        // and the errno is set to indicate the error
        errno = -value;
        return -1;
    }

    // on successfully sleeping for the requested interval, returns 0 
    return 0;
}