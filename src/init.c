/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yikoubaz <yikoubaz@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/28 10:20:41 by yikoubaz          #+#    #+#             */
/*   Updated: 2026/07/01 17:21:00 by yikoubaz         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/coddexion.h"

static int	init_dongles(t_sim *sim)
{
	int	i;

	sim->dongles = malloc(sizeof(t_dongle) * sim->nb_coders);
	if (!sim->dongles)
		return (0);
	i = 0;
	while (i < sim->nb_coders)
	{
		pthread_mutex_init(&sim->dongles[i].mutex, NULL);
		pthread_cond_init(&sim->dongles[i].cond, NULL);
		sim->dongles[i].held = 0;
		sim->dongles[i].cooldown_until = 0;
		sim->dongles[i].id = i;
		pq_init(&sim->dongles[i].queue, sim->nb_coders);
		i++;
	}
	return (1);
}

static void	setup_coder_dongles(t_sim *sim, int i)
{
	int	nb;

	nb = sim->nb_coders;
	sim->coders[i].left = &sim->dongles[i];
	sim->coders[i].right = &sim->dongles[(i + 1) % nb];
	if (nb == 1)
		sim->coders[i].right = &sim->dongles[0];
}

static int	init_coders(t_sim *sim)
{
	int	i;

	sim->coders = malloc(sizeof(t_coder) * sim->nb_coders);
	if (!sim->coders)
		return (0);
	i = 0;
	while (i < sim->nb_coders)
	{
		sim->coders[i].id = i + 1;
		sim->coders[i].last_compile_start = 0;
		sim->coders[i].compile_count = 0;
		sim->coders[i].finished = 0;
		sim->coders[i].sim = sim;
		setup_coder_dongles(sim, i);
		i++;
	}
	return (1);
}

int	init_sim(t_sim *sim, char **av)
{
	int	i;

	(void)av;
	pthread_mutex_init(&sim->run_mutex, NULL);
	pthread_mutex_init(&sim->print_mutex, NULL);
	pthread_mutex_init(&sim->order_mutex, NULL);
	pthread_cond_init(&sim->run_cond, NULL);
	sim->stopped = 0;
	sim->coders_finished = 0;
	sim->global_order = 0;
	sim->monitor_ready = 0;
	if (!init_dongles(sim))
		return (0);
	if (!init_coders(sim))
		return (0);
	sim->start_time = get_time_ms();
	i = 0;
	while (i < sim->nb_coders)
	{
		sim->coders[i].last_compile_start = sim->start_time;
		i++;
	}
	return (1);
}
