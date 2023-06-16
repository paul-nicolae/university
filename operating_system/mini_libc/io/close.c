// SPDX-License-Identifier: BSD-3-Clause

#include <unistd.h>
#include <internal/syscall.h>
#include <stdarg.h>
#include <errno.h>

int close(int fd)
{
	/* TODO: Implement close(). */
	int value = (int)syscall(__NR_close, fd);

	if (value < 0) {
		// setting the errno to indicate the error
		errno = -value;
		return -1;
	}

	// upon successful completion, 0 shall be returned
	return 0;
}
