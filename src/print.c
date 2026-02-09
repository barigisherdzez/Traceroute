#include "ft_traceroute.h"

void	ft_print_header(const t_target *t)
{
	printf("traceroute to %s (%s), %d hops max, 60 byte packets\n",
		t->fqdn, t->ip, FT_MAX_HOPS);
}

static int	same_ip(const char *a, const char *b)
{
	if (!a || !b || !a[0] || !b[0])
		return (0);
	return (ft_strcmp(a, b) == 0);
}

/*
Print like traceroute:
- If hop IP exists, print it once at start.
- If later probes return different hop IP (load balancing), print that IP before that RTT.
- No DNS in mandatory (IP only).
*/
void	ft_print_hop_line(int ttl, const char *hop_ip, t_probe_result pr[FT_PROBES_PER_HOP])
{
	int i;
	int any_ip = (hop_ip && hop_ip[0]);

	printf("%2d  ", ttl);

	if (any_ip)
		printf("%s", hop_ip);

	for (i = 0; i < FT_PROBES_PER_HOP; i++)
	{
		printf("%s", (i == 0) ? (any_ip ? "  " : "") : "  ");

		if (!pr[i].received)
		{
			printf("*");
			continue;
		}

		if (any_ip && pr[i].hop_ip[0] && !same_ip(pr[i].hop_ip, hop_ip))
			printf("%s  ", pr[i].hop_ip);

		printf("%.3f ms", pr[i].rtt_ms);
	}
	printf("\n");
}
