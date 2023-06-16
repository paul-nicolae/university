// SPDX-License-Identifier: BSD-3-Clause

#include <sys/stat.h>
#include <errno.h>
#include <internal/syscall.h>

int fstat(int fd, struct stat *st)
{
	/* TODO: Implement fstat(). */

	int value = syscall(__NR_fstat, fd, st);

	if (value < 0) {
		// on error, -1 is returned
		// and errno is set to indicate the error
		errno = -value;
		return -1;
	}

	// on success, 0 is returned
	return 0;
}
