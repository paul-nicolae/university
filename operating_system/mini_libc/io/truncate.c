// SPDX-License-Identifier: BSD-3-Clause

#include <unistd.h>
#include <internal/syscall.h>
#include <errno.h>

int truncate(const char *path, off_t length)
{
	/* TODO: Implement truncate(). */

	int value = syscall(__NR_truncate, path, length);

	if (value < 0) {
		// on error, -1 is returned and
		// the errno is set to indicate the error
		errno = -value;
		return -1;
	}
	
	// on success, 0 is returned
	return 0;
}
