/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitor.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yikoubaz <yikoubaz@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/18 00:44:11 by yikoubaz          #+#    #+#             */
/*   Updated: 2026/08/03 18:21:36 by yikoubaz         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	find_burned_coder(t_sim *sim, long *curr)
{
	int	i;

	i = 0;
	while (i < sim->data.nb_coders)
	{
		pthread_mutex_lock(&sim->coders[i].state_mutex);
		*curr = get_time() - sim->coders[i].last_compile;
		pthread_mutex_unlock(&sim->coders[i].state_mutex);
		if (*curr >= sim->data.time_to_burnout)
			return (i);
		i++;
	}
	return (-1);
}

static int	all_coders_done(t_sim *sim)
{
	int	i;
	int	all_done;

	i = 0;
	all_done = 1;
	while (i < sim->data.nb_coders)
	{
		pthread_mutex_lock(&sim->coders[i].state_mutex);
		if (sim->coders[i].compiles_count < sim->data.nb_compiles_required)
			all_done = 0;
		pthread_mutex_unlock(&sim->coders[i].state_mutex);
		i++;
	}
	return (all_done);
}

void	*monitor_routine(void *arg)
{
	t_sim	*sim;
	long	curr;
	int		i;

	sim = (t_sim *)arg;
	curr = 0;
	while (!simulation_stopped(sim))
	{
		i = find_burned_coder(sim, &curr);
		if (i >= 0 || all_coders_done(sim))
		{
			if (i >= 0)
			{
				stop_simulation(sim);
				pthread_mutex_lock(&sim->print_mutex);
				printf("%ld %d %s\n", elapsed_time(sim), sim->coders[i].id, "burned out");
				pthread_mutex_unlock(&sim->print_mutex);
			}
			else
				stop_simulation(sim);
			return (NULL);
		}
		smart_sleep(1, sim);
	}
	return (NULL);
}

int	create_monitor_thread(t_sim *sim)
{
	if (pthread_create(&sim->monitor, NULL, monitor_routine, sim) != 0)
		return (0);
	return (1);
}

int	join_monitor_thread(t_sim *sim)
{
	if (pthread_join(sim->monitor, NULL) != 0)
		return (0);
	return (1);
}
