#include <malloc.h>
#include "libufdt_sysdeps.h"

void dto_qsort(void *base, size_t nmemb, size_t size,
		int (*compar)(const void *, const void *))
{
	qsort(base, nmemb, size, compar);
}

void *dto_malloc(size_t size)
{
	return malloc(size);
}

void dto_free(void *ptr)
{
	free(ptr);
}

char *dto_strdup(const char *s)
{
	return strdup(s);
}

char *dto_strchr(const char *s, int c)
{
	return strchr(s, c);
}

unsigned long int dto_strtoul(const char *nptr, char **endptr, int base)
{
	return simple_strtoul(nptr, endptr, base);
}

size_t dto_strlen(const char *s)
{
	return strlen(s);
}

int dto_memcmp(const void *lhs, const void *rhs, size_t n)
{
	return memcmp(lhs, rhs, n);
}

void *dto_memcpy(void *dest, const void *src, size_t n)
{
	return memcpy(dest, src, n);
}

int dto_strcmp(const char *s1, const char *s2)
{
	return strcmp(s1, s2);
}

int dto_strncmp(const char *s1, const char *s2, size_t n)
{
	return strncmp(s1, s2, n);
}

void *dto_memchr(const void *s, int c, size_t n)
{
	return memchr(s, c, n);
}

void *dto_memset(void *s, int c, size_t n)
{
	return memset(s, c, n);
}
