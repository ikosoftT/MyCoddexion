/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yikoubaz <yikoubaz@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/28 10:20:30 by yikoubaz          #+#    #+#             */
/*   Updated: 2026/07/01 17:21:48 by yikoubaz         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/coddexion.h"

static int	fill_sim_from_args(t_sim *sim, char **av)
{
	if (ft_atol(av[1]) > INT_MAX || ft_atol(av[2]) > INT_MAX
		|| ft_atol(av[3]) > INT_MAX || ft_atol(av[4]) > INT_MAX
		|| ft_atol(av[5]) > INT_MAX || ft_atol(av[6]) > INT_MAX
		|| ft_atol(av[7]) > INT_MAX)
		return (0);
	if (!is_valid_int(av))
		return (0);
	sim->nb_coders = (int)ft_atol(av[1]);
	sim->time_to_burnout = ft_atol(av[2]);
	sim->time_to_compile = ft_atol(av[3]);
	sim->time_to_debug = ft_atol(av[4]);
	sim->time_to_refactor = ft_atol(av[5]);
	sim->nb_compiles_required = (int)ft_atol(av[6]);
	sim->dongle_cooldown = ft_atol(av[7]);
	if (!strcmp(av[8], "fifo"))
		sim->scheduler = FIFO;
	else
		sim->scheduler = EDF;
	return (1);
}

int	parse_args(int ac, char **av, t_sim *sim)
{
	if (ac != 9)
		return (0);
	if (!validate_args(ac, av))
		return (0);
	if (parse_scheduler(av[8]))
		return (0);
	if (!fill_sim_from_args(sim, av))
		return (0);
	if (!init_sim(sim, av))
		return (0);
	return (1);
}

int	validate_args(int ac, char **av)
{
	int	i;

	(void)ac;
	i = 1;
	while (i <= 7)
	{
		if (!is_num(av[i]))
			return (0);
		i++;
	}
	return (1);
}

int	parse_scheduler(char *str)
{
	if (!strcmp("fifo", str))
		return (0);
	if (!strcmp("edf", str))
		return (0);
	return (1);
}
