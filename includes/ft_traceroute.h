#ifndef FT_TRACEROUTE_H
#define FT_TRACEROUTE_H

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>

#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>

/* for parsing ICMP payload correctly */
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <netinet/ip_icmp.h>

#define FT_MAX_HOPS 30
#define FT_PROBES_PER_HOP 3
#define FT_TIMEOUT_MS 1000

typedef struct s_target {
	char                input[256];
	char                fqdn[NI_MAXHOST];
	char                ip[INET_ADDRSTRLEN];
	struct sockaddr_in  addr;
}	t_target;

typedef struct s_probe_result {
	int     received;
	double  rtt_ms;
	char    hop_ip[INET_ADDRSTRLEN];
	int     reached_dest;
}	t_probe_result;

/* mini helpers */
void    ft_bzero(void *p, size_t n);
size_t  ft_strlen(const char *s);
int     ft_strcmp(const char *a, const char *b);
void    ft_strlcpy(char *dst, const char *src, size_t dstsz);
void    ft_memcpy(void *dst, const void *src, size_t n);

/* args */
int  ft_parse_args(int argc, char **argv, int *help, char *dest, size_t destsz);
void ft_print_help(const char *prog);

/* resolve */
int  ft_resolve_target(const char *input, t_target *t);

/* tracing */
int  ft_traceroute_run(const t_target *t);

/* printing */
void ft_print_header(const t_target *t);
void ft_print_hop_line(int ttl, const char *hop_ip, t_probe_result pr[FT_PROBES_PER_HOP]);

/* util */
void ft_fatal(const char *msg);
double ft_time_diff_ms(const struct timeval *a, const struct timeval *b);

#endif
