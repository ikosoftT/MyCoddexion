/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitor.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yikoubaz <yikoubaz@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/18 00:44:11 by yikoubaz          #+#    #+#             */
/*   Updated: 2026/08/17 09:45:55 by yikoubaz         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	print_burnout(t_sim *sim, int *i)
{
	stop_simulation(sim);
	pthread_mutex_lock(&sim->print_mutex);
	printf("%ld %d %s\n", elapsed_time(sim), sim->coders[*i].id, "burned out");
	pthread_mutex_unlock(&sim->print_mutex);
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
				print_burnout(sim, &i);
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
