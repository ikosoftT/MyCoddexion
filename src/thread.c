/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   thread.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yikoubaz <yikoubaz@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/18 00:44:20 by yikoubaz          #+#    #+#             */
/*   Updated: 2026/08/03 14:29:37 by yikoubaz         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	one_coder(t_coder *coder)
{
	t_dongle	*dongle;

	if (coder->sim->data.nb_coders != 1)
		return (0);
	dongle = &coder->sim->dongles[0];
	if (!request_dongle(coder, dongle))
		return (1);
	while (!simulation_stopped(coder->sim))
		usleep(500);
	release_dongle(coder, dongle);
	return (1);
}

static int	do_work(t_coder *coder)
{
	log_status(coder, "is compiling");
	pthread_mutex_lock(&coder->state_mutex);
	coder->last_compile = get_time();
	pthread_mutex_unlock(&coder->state_mutex);
	smart_sleep(coder->sim->data.time_to_compile, coder->sim);
	pthread_mutex_lock(&coder->state_mutex);
	coder->compiles_count++;
	pthread_mutex_unlock(&coder->state_mutex);
	release_dongles(coder);
	if (simulation_stopped(coder->sim))
		return (1);
	log_status(coder, "is debugging");
	smart_sleep(coder->sim->data.time_to_debug, coder->sim);
	if (simulation_stopped(coder->sim))
		return (1);
	log_status(coder, "is refactoring");
	smart_sleep(coder->sim->data.time_to_refactor, coder->sim);
	return (simulation_stopped(coder->sim));
}

void	*coder_routine(void *arg)
{
	t_coder	*coder;

	coder = (t_coder *)arg;
	pthread_mutex_lock(&coder->sim->start_mutex);
	pthread_mutex_unlock(&coder->sim->start_mutex);
	if (one_coder(coder))
		return (NULL);
	while (!simulation_stopped(coder->sim))
	{
		if (!take_dongles(coder))
			return (NULL);
		if (simulation_stopped(coder->sim))
		{
			release_dongles(coder);
			return (NULL);
		}
		if (do_work(coder))
			return (NULL);
	}
	return (NULL);
}

int	create_coder_threads(t_sim *sim)
{
	int	i;

	pthread_mutex_lock(&sim->start_mutex);
	i = 0;
	while (i < sim->data.nb_coders)
	{
		if (pthread_create(&sim->coders[i].thread, NULL, coder_routine,
				&sim->coders[i]) != 0)
		{
			pthread_mutex_unlock(&sim->start_mutex);
			while (--i >= 0)
				pthread_join(sim->coders[i].thread, NULL);
			return (0);
		}
		i++;
	}
	pthread_mutex_unlock(&sim->start_mutex);
	return (1);
}

int	join_coder_threads(t_sim *sim)
{
	int	i;

	i = 0;
	while (i < sim->data.nb_coders)
	{
		if (pthread_join(sim->coders[i].thread, NULL) != 0)
			return (0);
		i++;
	}
	return (1);
}
