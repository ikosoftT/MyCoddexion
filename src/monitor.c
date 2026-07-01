/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitor.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yikoubaz <yikoubaz@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/28 10:21:09 by yikoubaz          #+#    #+#             */
/*   Updated: 2026/07/01 17:21:46 by yikoubaz         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/coddexion.h"


static int	check_burnout(t_sim *sim)
{
	int		i;
	long	now;
	long	deadline;

	now = get_time_ms();
	i = 0;
	while (i < sim->nb_coders)
	{
		pthread_mutex_lock(&sim->run_mutex);
		deadline = sim->coders[i].last_compile_start + sim->time_to_burnout;
		if (!sim->coders[i].finished
			&& now > deadline
			&& sim->coders[i].compile_count < sim->nb_compiles_required)
		{
			sim->stopped = 1;
			pthread_mutex_unlock(&sim->run_mutex);
			log_action(sim, sim->coders[i].id, "burned out");
			wake_all_dongles(sim);
			return (1);
		}
		pthread_mutex_unlock(&sim->run_mutex);
		i++;
	}
	return (0);
}

void	*monitor_routine(void *arg)
{
	t_sim	*sim;

	sim = (t_sim *)arg;
	sim->monitor_ready = 1;
	while (1)
	{
		pthread_mutex_lock(&sim->run_mutex);
		if (sim->stopped)
		{
			pthread_mutex_unlock(&sim->run_mutex);
			break ;
		}
		pthread_mutex_unlock(&sim->run_mutex);
		if (check_burnout(sim))
			break ;
		pthread_mutex_lock(&sim->run_mutex);
		if (sim->coders_finished >= sim->nb_coders)
		{
			sim->stopped = 1;
			pthread_cond_broadcast(&sim->run_cond);
			pthread_mutex_unlock(&sim->run_mutex);
			wake_all_dongles(sim);
			break ;
		}
		pthread_mutex_unlock(&sim->run_mutex);
		usleep(500);
	}
	return (NULL);
}
