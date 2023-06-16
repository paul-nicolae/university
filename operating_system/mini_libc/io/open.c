// SPDX-License-Identifier: BSD-3-Clause

#include <fcntl.h>
#include <internal/syscall.h>
#include <stdarg.h>
#include <errno.h>

int open(const char *filename, int flags, ...)
{
	/* TODO: Implement open system call. */

	// taking the third possible parameter of open syscall
	va_list valist;
	va_start(valist, flags);
	mode_t mode = va_arg(valist, mode_t);
	va_end(valist);
	
	int value = syscall(__NR_open, filename, flags, mode);

	if (value < 0) {
		// setting the errno to indicate the error
		errno = -value;
		return -1;
	}
	
	// on success, return the new file descriptor (a nonnegative integer)
	return value;
}