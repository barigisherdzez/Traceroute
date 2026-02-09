#include "ft_traceroute.h"

void	ft_bzero(void *p, size_t n)
{
	size_t i;
	unsigned char *b;

	b = (unsigned char *)p;
	i = 0;
	while (i < n)
	{
		b[i] = 0;
		i++;
	}
}

size_t	ft_strlen(const char *s)
{
	size_t i = 0;
	while (s && s[i])
		i++;
	return (i);
}

int	ft_strcmp(const char *a, const char *b)
{
	size_t i = 0;

	if (!a && !b) return 0;
	if (!a) return -1;
	if (!b) return 1;
	while (a[i] && b[i] && a[i] == b[i])
		i++;
	return ((unsigned char)a[i] - (unsigned char)b[i]);
}

void	ft_strlcpy(char *dst, const char *src, size_t dstsz)
{
	size_t i = 0;

	if (!dst || dstsz == 0)
		return;
	if (!src)
	{
		dst[0] = '\0';
		return;
	}
	while (src[i] && i + 1 < dstsz)
	{
		dst[i] = src[i];
		i++;
	}
	dst[i] = '\0';
}

void	ft_memcpy(void *dst, const void *src, size_t n)
{
	size_t i;
	unsigned char *d;
	const unsigned char *s;

	if (!dst || !src)
		return;
	d = (unsigned char *)dst;
	s = (const unsigned char *)src;
	i = 0;
	while (i < n)
	{
		d[i] = s[i];
		i++;
	}
}

void	ft_fatal(const char *msg)
{
	if (msg)
		printf("ft_traceroute: %s\n", msg);
	exit(1);
}

double	ft_time_diff_ms(const struct timeval *a, const struct timeval *b)
{
	long sec = b->tv_sec - a->tv_sec;
	long usec = b->tv_usec - a->tv_usec;
	return (double)sec * 1000.0 + (double)usec / 1000.0;
}
