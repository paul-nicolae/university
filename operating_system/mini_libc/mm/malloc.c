// SPDX-License-Identifier: BSD-3-Clause

#include <internal/mm/mem_list.h>
#include <internal/types.h>
#include <internal/essentials.h>
#include <sys/mman.h>
#include <string.h>
#include <stdlib.h>

void *malloc(size_t size)
{
	/* TODO: Implement malloc(). */
	void *start = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (start == MAP_FAILED) return MAP_FAILED;

	mem_list_add(start, size);

	return start;
}

void *calloc(size_t nmemb, size_t size)
{
	/* TODO: Implement calloc(). */
	size_t length = nmemb * size;

	void *start = mmap(NULL, length, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (start == MAP_FAILED) return MAP_FAILED;

	memset(start, 0, length);

	mem_list_add(start, length);

	return start;
}

void free(void *ptr)
{
	/* TODO: Implement free(). */
	struct mem_list *item = mem_list_find(ptr);

	// delete the allocated memory
	munmap(item->start, item->len);

	// extract and delete the mem_list struct from doubly-linked list
	mem_list_del(ptr);
}

void *realloc(void *ptr, size_t size)
{
	/* TODO: Implement realloc(). */
	struct mem_list *item = mem_list_find(ptr);
	void *start = mremap(item->start, item->len, size, MREMAP_MAYMOVE);
	if (start == MAP_FAILED) return MAP_FAILED;

	mem_list_add(start, size);

	return start;
}

void *reallocarray(void *ptr, size_t nmemb, size_t size)
{
	/* TODO: Implement reallocarray(). */
	size_t length = nmemb * size;
	
	struct mem_list *item = mem_list_find(ptr);
	void *start = mremap(item->start, item->len, length, MREMAP_MAYMOVE);
	if (start == MAP_FAILED) return MAP_FAILED;

	mem_list_add(start, size);

	return start;
}
