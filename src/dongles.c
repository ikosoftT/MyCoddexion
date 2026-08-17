/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongles.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yikoubaz <yikoubaz@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/18 00:44:07 by yikoubaz          #+#    #+#             */
/*   Updated: 2026/08/17 12:48:52 by yikoubaz         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	release_dongle(t_coder *coder, t_dongle *dongle)
{
	pthread_mutex_lock(&coder->sim->grant_mutex);
	pthread_mutex_lock(&dongle->mutex);
	dongle->held = 0;
	dongle->cooldown_until = get_time() + coder->sim->data.dongle_cooldown;
	pthread_mutex_unlock(&dongle->mutex);
	pthread_cond_broadcast(&dongle->cond);
	pthread_mutex_unlock(&coder->sim->grant_mutex);
}

static int	check_and_use(t_sim *sim, t_coder *coder, int left, int right)
{
	if (dongle_state(&sim->dongles[left])
		&& dongle_state(&sim->dongles[right])
		&& is_eligible(sim, &sim->dongles[left], coder)
		&& is_eligible(sim, &sim->dongles[right], coder))
	{
		heap_remove(&sim->dongles[left].heap, coder);
		pthread_mutex_lock(&sim->dongles[left].mutex);
		sim->dongles[left].held = 1;
		pthread_mutex_unlock(&sim->dongles[left].mutex);
		heap_remove(&sim->dongles[right].heap, coder);
		pthread_mutex_lock(&sim->dongles[right].mutex);
		sim->dongles[right].held = 1;
		pthread_mutex_unlock(&sim->dongles[right].mutex);
		pthread_mutex_unlock(&sim->grant_mutex);
		log_status(coder, "has taken a dongle");
		log_status(coder, "has taken a dongle");
		return (1);
	}
	return (0);
}

static void	push_in_heap(t_sim *sim, t_coder *coder, int left, int right)
{
	pthread_mutex_lock(&sim->grant_mutex);
	push_request(coder, &sim->dongles[left]);
	push_request(coder, &sim->dongles[right]);
	pthread_mutex_unlock(&sim->grant_mutex);
}

int	take_dongles(t_coder *coder)
{
	t_sim	*sim;
	int		left;
	int		right;

	sim = coder->sim;
	left = coder->left;
	right = coder->right;
	push_in_heap(sim, coder, left, right);
	while (!simulation_stopped(sim))
	{
		pthread_mutex_lock(&sim->grant_mutex);
		if (check_and_use(sim, coder, left, right))
			return (1);
		pthread_mutex_unlock(&sim->grant_mutex);
		if (simulation_stopped(sim))
			break ;
		usleep(200);
	}
	pthread_mutex_lock(&sim->grant_mutex);
	heap_remove(&sim->dongles[left].heap, coder);
	heap_remove(&sim->dongles[right].heap, coder);
	pthread_mutex_unlock(&sim->grant_mutex);
	return (0);
}

void	release_dongles(t_coder *coder)
{
	release_dongle(coder, &coder->sim->dongles[coder->left]);
	release_dongle(coder, &coder->sim->dongles[coder->right]);
}
