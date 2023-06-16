// SPDX-License-Identifier: BSD-3-Clause

#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <internal/syscall.h>

int stat(const char *restrict path, struct stat *restrict buf)
{
	/* TODO: Implement stat(). */
	int value = syscall(__NR_stat, path, buf);

	if (value < 0) {
		// on error, -1 is returned
		// and errno is set to indicate the error
		errno = -value;
		return -1;
	}

	// on success, 0 is returned
	return 0;
}
