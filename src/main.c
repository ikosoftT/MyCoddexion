/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yikoubaz <yikoubaz@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/18 00:43:52 by yikoubaz          #+#    #+#             */
/*   Updated: 2026/08/03 16:13:11 by yikoubaz         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	ft_err(char *err)
{
	while (*err)
		write(2, err++, 1);
	return (2);
}

static int	ft_setup(int ac, char **av, t_data *data, t_sim *sim)
{
	if (!data)
	{
		free(sim);
		return (ft_err("Fail at data alloc\n"));
	}
	if (!parse_args(ac, av, data))
	{
		free(data);
		free(sim);
		return (ft_err("Parsing error\n"));
	}
	return (1);
}

static int	ft_initor(t_data *data, t_sim *sim)
{
	if (!init_sim(sim, data))
		return (0);
	if (!init_dongles(sim))
		return (0);
	if (!init_coders(sim))
		return (0);
	return (1);
}

static int	run_simulation(t_sim *sim)
{
	if (!create_coder_threads(sim))
		return (ft_err("fail at creating threads\n"));
	if (!create_monitor_thread(sim))
	{
		stop_simulation(sim);
		join_coder_threads(sim);
		return (ft_err("Fail at create monitor thread\n"));
	}
	if (!join_monitor_thread(sim))
	{
		join_coder_threads(sim);
		return (ft_err("Fail at join monitor thread\n"));
	}
	if (!join_coder_threads(sim))
		return (ft_err("Fail at Join coder thread\n"));
	return (0);
}

int	main(int ac, char **av)
{
	t_data	*data;
	t_sim	*sim;
	int		code;

	sim = malloc(sizeof(t_sim));
	if (!sim)
		return (ft_err("Error: Failing at sim allocation\n"));
	data = malloc(sizeof(t_data));
	code = ft_setup(ac, av, data, sim);
	if (code != 1)
		return (code);
	if (!ft_initor(data, sim))
	{
		free(data);
		clean_all(sim);
		return (ft_err("Fail at initor func\n"));
	}
	code = run_simulation(sim);
	free(data);
	clean_all(sim);
	return (code);
}
