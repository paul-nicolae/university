// SPDX-License-Identifier: BSD-3-Clause

#include <string.h>

char *strcpy(char *destination, const char *source)
{
    char* destination_start = destination;
    while(*source != 0){
        *(destination++) = *(source++);
    }
    *destination = 0;

    return destination_start;
}

char *strncpy(char *destination, const char *source, size_t len)
{

    char* destination_start = destination;
    while(*source != 0 && (len--) > 0){
        *(destination++) = *(source++);
    }
    // *destination = 0;

    return destination_start;
}

char *strcat(char *destination, const char *source)
{
    char* destination_start = destination;
    size_t destination_len = strlen(destination);
    destination += destination_len;

    while(*source != 0){
        *(destination++) = *(source++);
    }
    *destination = 0;

    return destination_start;
}

char *strncat(char *destination, const char *source, size_t len)
{
    char* destination_start = destination;
    size_t destination_len = strlen(destination);
    destination += destination_len;

    while(*source != 0 && (len--) > 0){
        *(destination++) = *(source++);
    }
    *destination = 0;

    return destination_start;
}

int strcmp(const char *str1, const char *str2)
{
    for(; *str1 != 0 && *str2 != 0 && *str1 == *str2; str1++, str2++);
    return *str1 - *str2;
}

int strncmp(const char *str1, const char *str2, size_t len)
{
    for(; *str1 != 0 && *str2 != 0 && *str1 == *str2 && len > 1; str1++, str2++, len--);
    return *str1 - *str2;
}

size_t strlen(const char *str)
{
    size_t i = 0;

    for (; *str != '\0'; str++, i++)
        ;

    return i;
}

char *strchr(const char *str, int c)
{
    while(*(str) != 0 && *(str++) != c);
    return *(str - 1) == c ? str - 1 : NULL;
}

char *strrchr(const char *str, int c)
{
    char* last_occurrence = NULL;
    while(*str != 0){
        if(*(str++) == c) last_occurrence = str - 1;
    }

    return last_occurrence;
}

char *strstr(const char *haystack, const char *needle)
{
    char* needle_start = needle;

    while(*haystack != 0){

        char* cursor = haystack;
        while(*(cursor) != 0 && *(needle) != 0 && *(needle++) == *(cursor++));

        if(*needle == 0 && *(cursor - 1) == *(needle - 1)) return haystack;

        needle = needle_start;
        haystack++;
    }

    return NULL;
}

char *strrstr(const char *haystack, const char *needle)
{
    char* needle_start = needle;
    char* last_occurrence = NULL;

    while(*haystack != 0){

        char* cursor = haystack;
        while(*(cursor) != 0 && *(needle) != 0 && *(needle++) == *(cursor++));
        if(*needle == 0 && *(cursor - 1) == *(needle - 1)) last_occurrence = haystack;

        needle = needle_start;
        haystack++;
    }

    return last_occurrence;
}

void *memcpy(void *destination, const void *source, size_t num)
{
    char *casted_dest = (char*)destination;
    char *casted_source = (char*)source;

    for(;num > 0; *(casted_dest++) = *(casted_source++), num--);

    return destination;
}

void *memmove(void *destination, const void *source, size_t num)
{
    char *casted_dest = (char*)destination;
    char *casted_source = (char*)source;

    for(;num > 0; *(casted_dest++) = *(casted_source++), num--);

    return destination;
}

int memcmp(const void *ptr1, const void *ptr2, size_t num)
{
    char *casted_ptr1 = (char*)ptr1;
    char *casted_ptr2 = (char*)ptr2;

    for(; *casted_ptr1 == *casted_ptr2 && num > 1; casted_ptr1++, casted_ptr2++, num--);
    return *casted_ptr1 - *casted_ptr2;
}

void *memset(void *source, int value, size_t num)
{
    int *casted_source = (int*)source;
    for(; num > 0; *(casted_source++) = value, num--);

    return source;
}