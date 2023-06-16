#ifndef __TIME_H__
#define __TIME_H__	1

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned int time_t;

// the structure timespec is used to specify
// intervals of time with nanosecond precision
struct timespec
{
    time_t tv_sec;    // seconds
    long   tv_nsec;   // nanoseconds
};

int nanosleep(const struct timespec *req, struct timespec *rem);

#ifdef __cplusplus
}
#endif

#endif
