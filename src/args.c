#include "ft_traceroute.h"

void	ft_print_help(const char *prog)
{
	printf("Usage:\n");
	printf("  %s <destination>\n", prog);
	printf("  %s --help\n", prog);
	printf("\n");
	printf("destination: IPv4 address or hostname (IPv4 only)\n");
}

int	ft_parse_args(int argc, char **argv, int *help, char *dest, size_t destsz)
{
	if (!help || !dest || destsz == 0)
		return (1);

	*help = 0;
	dest[0] = '\0';

	if (argc == 2 && ft_strcmp(argv[1], "--help") == 0)
	{
		*help = 1;
		return (0);
	}
	if (argc != 2)
	{
		printf("ft_traceroute: invalid arguments (use --help)\n");
		return (1);
	}
	ft_strlcpy(dest, argv[1], destsz);
	return (0);
}
