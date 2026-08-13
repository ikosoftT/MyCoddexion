/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitor_util.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yikoubaz <yikoubaz@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/13 22:06:09 by yikoubaz          #+#    #+#             */
/*   Updated: 2026/08/13 22:12:12 by yikoubaz         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "./codexion.h"

int	find_burned_coder(t_sim *sim, long *curr)
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

int	all_coders_done(t_sim *sim)
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
