/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongles.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yikoubaz <yikoubaz@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/18 00:44:07 by yikoubaz          #+#    #+#             */
/*   Updated: 2026/08/13 22:29:33 by yikoubaz         ###   ########.fr       */
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

// Add Take Dongles

int	take_dongles(t_coder *coder)
{
	t_sim	*sim;
	int		left;
	int		right;

	sim = coder->sim;
	if (sim->data.nb_coders == 1)
		return (request_dongle(coder, &sim->dongles[0]));
	left = coder->left;
	right = coder->right;
	pthread_mutex_lock(&sim->grant_mutex);
	push_request(coder, &sim->dongles[left]);
	push_request(coder, &sim->dongles[right]);
	pthread_mutex_unlock(&sim->grant_mutex);
	while (!simulation_stopped(sim))
	{
		pthread_mutex_lock(&sim->grant_mutex);
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
			log_status(coder, "has taken a dongle\nhas taken a dongle");
			return (1);
		}
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
