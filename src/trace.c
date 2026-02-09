#include "ft_traceroute.h"

/* ---------- helpers ---------- */

static void	set_timeout(struct timeval *tv, int timeout_ms)
{
	tv->tv_sec = timeout_ms / 1000;
	tv->tv_usec = (timeout_ms % 1000) * 1000;
}

static int	compute_port(int ttl, int seq)
{
	return 33434 + (ttl * FT_PROBES_PER_HOP) + seq;
}

static int	send_udp_probe(int udp_fd, const struct sockaddr_in *dst, int ttl, int seq)
{
	int port;
	struct sockaddr_in to;
	char payload[1];

	port = compute_port(ttl, seq);
	to = *dst;
	to.sin_port = htons((unsigned short)port);
	payload[0] = 0;

	if (setsockopt(udp_fd, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl)) < 0)
		return (-1);
	if (sendto(udp_fd, payload, sizeof(payload), 0,
			(const struct sockaddr *)&to, sizeof(to)) < 0)
		return (-1);
	return (0);
}

/* drain any queued ICMP so we don't match stale packets */
static void	drain_icmp(int icmp_fd)
{
	while (1)
	{
		fd_set rfds;
		struct timeval tv;
		unsigned char buf[2048];
		struct sockaddr_in from;
		socklen_t fromlen;
		int n;

		FD_ZERO(&rfds);
		FD_SET(icmp_fd, &rfds);
		tv.tv_sec = 0;
		tv.tv_usec = 0;

		n = select(icmp_fd + 1, &rfds, NULL, NULL, &tv);
		if (n <= 0)
			return;

		fromlen = (socklen_t)sizeof(from);
		(void)recvfrom(icmp_fd, buf, sizeof(buf), 0, (struct sockaddr *)&from, &fromlen);
	}
}

/*
Strict match:
- Parse outer IP if present
- Parse ICMP header
- Parse embedded (inner) IP header
- Parse embedded UDP header and compare dst port
Return 1 only when it matches expected_port.
Set *reached_dest only for ICMP DestUnreach PortUnreach (3/3).
*/
static int	match_icmp_packet(const unsigned char *buf, int len, int expected_port, int *reached_dest)
{
	int off = 0;
	const struct ip *outer_ip;
	int outer_hlen;
	const struct icmphdr *ic;
	const unsigned char *inner;
	const struct ip *inner_ip;
	int inner_hlen;
	const struct udphdr *udp;
	unsigned short dport;

	*reached_dest = 0;

	if (!buf || len < (int)sizeof(struct icmphdr))
		return (0);

	/* If packet starts with IPv4 header */
	if (len >= (int)sizeof(struct ip))
	{
		outer_ip = (const struct ip *)buf;
		if (outer_ip->ip_v == 4)
		{
			outer_hlen = outer_ip->ip_hl * 4;
			if (outer_hlen >= (int)sizeof(struct ip) && len >= outer_hlen + (int)sizeof(struct icmphdr))
				off = outer_hlen;
		}
	}

	/* ICMP header */
	if (len < off + (int)sizeof(struct icmphdr))
		return (0);
	ic = (const struct icmphdr *)(buf + off);

	if (!(ic->type == ICMP_TIME_EXCEEDED || ic->type == ICMP_DEST_UNREACH))
		return (0);

	/* inner IP begins after ICMP header (8 bytes) */
	inner = (const unsigned char *)(buf + off + 8);
	if (buf + len < inner + (int)sizeof(struct ip))
		return (0);

	inner_ip = (const struct ip *)inner;
	if (inner_ip->ip_v != 4)
		return (0);

	inner_hlen = inner_ip->ip_hl * 4;
	if (inner_hlen < (int)sizeof(struct ip))
		return (0);

	/* embedded UDP header must exist (first 8 bytes included) */
	if (buf + len < inner + inner_hlen + (int)sizeof(struct udphdr))
		return (0);

	udp = (const struct udphdr *)(inner + inner_hlen);
	dport = ntohs(udp->uh_dport);

	if ((int)dport != expected_port)
		return (0);

	/* destination reached: port unreachable */
	if (ic->type == ICMP_DEST_UNREACH && ic->code == ICMP_PORT_UNREACH)
		*reached_dest = 1;

	return (1);
}

static int	wait_icmp_for_port(int icmp_fd, int expected_port,
							struct sockaddr_in *from, int *reached_dest)
{
	struct timeval t0, now;
	int elapsed_ms;

	while (1)
	{
		fd_set rfds;
		struct timeval tv;
		unsigned char buf[2048];
		socklen_t fromlen;
		int n;
		int is_dest;

		if (gettimeofday(&t0, NULL) != 0)
			return (0);

		/* loop until timeout */
		while (1)
		{
			if (gettimeofday(&now, NULL) != 0)
				return (0);

			elapsed_ms = (int)ft_time_diff_ms(&t0, &now);
			if (elapsed_ms >= FT_TIMEOUT_MS)
				return (0);

			FD_ZERO(&rfds);
			FD_SET(icmp_fd, &rfds);
			set_timeout(&tv, FT_TIMEOUT_MS - elapsed_ms);

			n = select(icmp_fd + 1, &rfds, NULL, NULL, &tv);
			if (n == 0)
				return (0);
			if (n < 0)
			{
				if (errno == EINTR)
					continue;
				return (0);
			}

			fromlen = (socklen_t)sizeof(*from);
			n = recvfrom(icmp_fd, buf, sizeof(buf), 0, (struct sockaddr *)from, &fromlen);
			if (n <= 0)
				continue;

			if (match_icmp_packet(buf, n, expected_port, &is_dest))
			{
				*reached_dest = is_dest;
				return (1);
			}
			/* else unrelated ICMP, keep waiting until timeout */
		}
	}
}

/* ---------- main traceroute ---------- */

int	ft_traceroute_run(const t_target *t)
{
	int udp_fd;
	int icmp_fd;
	int ttl;
	int i;

	t_probe_result pr[FT_PROBES_PER_HOP];
	char first_hop_ip[INET_ADDRSTRLEN];
	struct timeval start, end;

	udp_fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (udp_fd < 0)
	{
		printf("ft_traceroute: socket UDP: %s\n", strerror(errno));
		return (1);
	}

	icmp_fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (icmp_fd < 0)
	{
		printf("ft_traceroute: socket ICMP (need sudo): %s\n", strerror(errno));
		close(udp_fd);
		return (1);
	}

	for (ttl = 1; ttl <= FT_MAX_HOPS; ttl++)
	{
		ft_bzero(pr, sizeof(pr));
		first_hop_ip[0] = '\0';

		for (i = 0; i < FT_PROBES_PER_HOP; i++)
		{
			struct sockaddr_in from;
			int reached_dest;
			int expected_port;

			expected_port = compute_port(ttl, i);

			/* prevent stale matching */
			drain_icmp(icmp_fd);

			if (gettimeofday(&start, NULL) != 0)
				continue;

			if (send_udp_probe(udp_fd, &t->addr, ttl, i) != 0)
				continue;

			ft_bzero(&from, sizeof(from));
			if (!wait_icmp_for_port(icmp_fd, expected_port, &from, &reached_dest))
				continue;

			if (gettimeofday(&end, NULL) != 0)
				continue;

			pr[i].received = 1;
			pr[i].rtt_ms = ft_time_diff_ms(&start, &end);
			ft_strlcpy(pr[i].hop_ip, inet_ntoa(from.sin_addr), sizeof(pr[i].hop_ip));

			if (first_hop_ip[0] == '\0')
				ft_strlcpy(first_hop_ip, pr[i].hop_ip, sizeof(first_hop_ip));

			if (reached_dest)
				pr[i].reached_dest = 1;
		}

		ft_print_hop_line(ttl, first_hop_ip, pr);

		for (i = 0; i < FT_PROBES_PER_HOP; i++)
		{
			if (pr[i].reached_dest)
			{
				close(icmp_fd);
				close(udp_fd);
				return (0);
			}
		}
	}

	close(icmp_fd);
	close(udp_fd);
	return (0);
}
