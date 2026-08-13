/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   thread_time_init.c                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yikoubaz <yikoubaz@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/13 21:17:38 by yikoubaz          #+#    #+#             */
/*   Updated: 2026/08/13 22:18:31 by yikoubaz         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "./codexion.h"

void	thread_time_init(t_sim *sim)
{
	int	i;

	sim->start_time = get_time();
	i = 0;
	while (i < sim->data.nb_coders)
	{
		pthread_mutex_lock(&sim->coders[i].state_mutex);
		sim->coders[i].last_compile = get_time();
		pthread_mutex_unlock(&sim->coders[i].state_mutex);
		i++;
	}
}
