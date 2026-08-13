/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yikoubaz <yikoubaz@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/18 00:44:15 by yikoubaz          #+#    #+#             */
/*   Updated: 2026/08/13 22:16:52 by yikoubaz         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	is_valid(char **av)
{
	int	i;

	i = 1;
	while (i <= 6)
	{
		if (ft_atol(av[i]) < 0)
			return (0);
		i++;
	}
	return (1);
}

static int	is_num(char *s)
{
	int	i;

	i = 0;
	while (s[i] && is_space(s[i]))
		i++;
	while (s[i])
	{
		if (s[i] < '0' || s[i] > '9')
			return (0);
		i++;
	}
	return (1);
}

static int	validate_args(int ac, char **av)
{
	int	i;

	if (ac != 9)
		return (0);
	i = 1;
	while (i <= 7)
	{
		if (!is_num(av[i]))
			return (0);
		i++;
	}
	i = 1;
	while (i <= 8)
	{
		if (ft_atol(av[i]) < INT_MIN || ft_atol(av[i]) > INT_MAX)
			return (0);
		i++;
	}
	if (!is_valid(av) || ft_atol(av[8]) < 0)
		return (0);
	return (1);
}

int	fill_data(char **av, t_data *data)
{
	data->nb_coders = (int)ft_atol(av[1]);
	data->time_to_burnout = ft_atol(av[2]);
	data->time_to_compile = ft_atol(av[3]);
	data->time_to_debug = ft_atol(av[4]);
	data->time_to_refactor = ft_atol(av[5]);
	data->nb_compiles_required = (int)ft_atol(av[6]);
	data->dongle_cooldown = ft_atol(av[7]);
	if (strcmp("fifo", av[8]) == 0)
		data->scheduler = FIFO;
	else if (strcmp("edf", av[8]) == 0)
		data->scheduler = EDF;
	else
		return (0);
	return (1);
}

int	parse_args(int ac, char **av, t_data *data)
{
	if (!validate_args(ac, av))
		return (0);
	if (!fill_data(av, data))
		return (0);
	return (1);
}
