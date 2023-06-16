// SPDX-License-Identifier: BSD-3-Clause

#include <unistd.h>
#include <internal/syscall.h>
#include <errno.h>

off_t lseek(int fd, off_t offset, int whence)
{
	/* TODO: Implement lseek(). */

	off_t value = (off_t)syscall(__NR_lseek, fd, offset, whence);

	if (value < 0) {
		// on error, the value -1 is returned and
		// the errno is set to indicate the error
		errno = -value;
		return -1;
	}

	// upon successful completion, returns the resulting offset
	return value;
}
