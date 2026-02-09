#include "ft_traceroute.h"

int	ft_resolve_target(const char *input, t_target *t)
{
	struct addrinfo hints;
	struct addrinfo *res;
	int gai;
	char *ip_tmp;

	if (!input || !t)
		return (1);

	ft_bzero(t, sizeof(*t));
	ft_strlcpy(t->input, input, sizeof(t->input));

	ft_bzero(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;

	res = NULL;
	gai = getaddrinfo(input, NULL, &hints, &res);
	if (gai != 0)
	{
		printf("ft_traceroute: %s\n", gai_strerror(gai));
		return (1);
	}

	ft_memcpy(&t->addr, res->ai_addr, sizeof(struct sockaddr_in));

	ip_tmp = inet_ntoa(t->addr.sin_addr);
	if (!ip_tmp)
	{
		printf("ft_traceroute: inet_ntoa failed\n");
		freeaddrinfo(res);
		return (1);
	}
	ft_strlcpy(t->ip, ip_tmp, sizeof(t->ip));

	/* mandatory: manage FQDN but no per-hop DNS: keep input as "fqdn" */
	ft_strlcpy(t->fqdn, input, sizeof(t->fqdn));

	freeaddrinfo(res);
	return (0);
}
