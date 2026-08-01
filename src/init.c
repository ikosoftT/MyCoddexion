/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yikoubaz <yikoubaz@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/18 02:51:59 by yikoubaz          #+#    #+#             */
/*   Updated: 2026/07/27 08:54:25 by yikoubaz         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	init_sim(t_sim *sim, t_data *data)
{
	sim->data = *data;
	if (pthread_mutex_init(&sim->print_mutex, NULL) != 0
		|| pthread_mutex_init(&sim->stop_mutex, NULL) != 0)
		return (0);
	sim->start_time = get_time();
	sim->stop = 0;
	sim->coders = NULL;
	sim->dongles = NULL;
	sim->request_counter = 0;
	if (pthread_mutex_init(&sim->scheduler_mutex, NULL) != 0)
		return (0);
	if (pthread_barrier_init(&sim->start_barrier, NULL,
			sim->data.nb_coders + 1) != 0)
		return (0);
	return (1);
}

static int	init_dongle_mutex(t_sim *sim, int i)
{
	if (pthread_mutex_init(&sim->dongles[i].mutex, NULL) != 0)
	{
		while (--i >= 0)
			pthread_mutex_destroy(&sim->dongles[i].mutex);
		free(sim->dongles);
		return (0);
	}
	sim->dongles[i].held = 0;
	sim->dongles[i].cooldown_until = 0;
	if (pthread_cond_init(&sim->dongles[i].cond, NULL) != 0)
	{
		while (--i >= 0)
			pthread_cond_destroy(&sim->dongles[i].cond);
		return (0);
	}
	return (heap_init(&sim->dongles[i].heap, sim->data.nb_coders));
}

int	init_dongles(t_sim *sim)
{
	int	i;

	sim->dongles = malloc(sizeof(t_dongle) * sim->data.nb_coders);
	if (!sim->dongles)
		return (0);
	i = 0;
	while (i < sim->data.nb_coders)
	{
		if (!init_dongle_mutex(sim, i))
			return (0);
		i++;
	}
	return (1);
}

int	init_coders(t_sim *sim)
{
	int	i;

	sim->coders = malloc(sizeof(t_coder) * sim->data.nb_coders);
	if (!sim->coders)
		return (0);
	i = 0;
	while (i < sim->data.nb_coders)
	{
		sim->coders[i].sim = sim;
		sim->coders[i].id = i + 1;
		sim->coders[i].compiles_count = 0;
		sim->coders[i].last_compile = sim->start_time;
		sim->coders[i].left = i;
		sim->coders[i].right = (i + 1) % sim->data.nb_coders;
		pthread_mutex_init(&sim->coders[i].state_mutex, NULL);
		i++;
	}
	return (1);
}
