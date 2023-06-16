// SPDX-License-Identifier: BSD-3-Clause

#include <internal/io.h>
#include <string.h>

int puts(const char *str)
{
    int value = write(1, str, strlen(str));
    write(1, "\n", 1);

    return value;
}