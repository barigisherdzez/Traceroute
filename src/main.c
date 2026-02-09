#include "ft_traceroute.h"

int	main(int argc, char **argv)
{
	int     help;
	char    dest[256];
	t_target t;

	help = 0;
	if (ft_parse_args(argc, argv, &help, dest, sizeof(dest)) != 0)
		return (1);
	if (help)
	{
		ft_print_help(argv[0]);
		return (0);
	}

	ft_bzero(&t, sizeof(t));
	if (ft_resolve_target(dest, &t) != 0)
		return (1);

	ft_print_header(&t);
	if (ft_traceroute_run(&t) != 0)
		return (1);

	return (0);
}
