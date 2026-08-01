/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   simulation.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yikoubaz <yikoubaz@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/18 05:27:36 by yikoubaz          #+#    #+#             */
/*   Updated: 2026/07/22 07:53:15 by yikoubaz         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	simulation_stopped(t_sim *sim)
{
	int	stop;

	pthread_mutex_lock(&sim->stop_mutex);
	stop = sim->stop;
	pthread_mutex_unlock(&sim->stop_mutex);
	return (stop);
}

void	stop_simulation(t_sim *sim)
{
	int	i;

	pthread_mutex_lock(&sim->stop_mutex);
	sim->stop = 1;
	pthread_mutex_unlock(&sim->stop_mutex);
	if (sim->dongles)
	{
		i = 0;
		while (i < sim->data.nb_coders)
		{
			pthread_mutex_lock(&sim->dongles[i].mutex);
			pthread_cond_broadcast(&sim->dongles[i].cond);
			pthread_mutex_unlock(&sim->dongles[i].mutex);
			i++;
		}
	}
}

long	elapsed_time(t_sim *sim)
{
	long	elapsed;

	elapsed = get_time() - sim->start_time;
	return (elapsed);
}
