/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   scheduler.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yikoubaz <yikoubaz@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/23 08:28:35 by yikoubaz          #+#    #+#             */
/*   Updated: 2026/08/13 21:59:08 by yikoubaz         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

t_request	build_request(t_coder *coder)
{
	t_sim		*sim;
	t_request	req;

	sim = coder->sim;
	req.coder = coder;
	req.order = sim->request_counter++;
	if (sim->data.scheduler == FIFO)
		req.priority = req.order;
	else
	{
		pthread_mutex_lock(&coder->state_mutex);
		req.priority = coder->last_compile + sim->data.time_to_burnout;
		pthread_mutex_unlock(&coder->state_mutex);
	}
	return (req);
}

static int	get_it(t_coder *coder, t_dongle *dongle, t_sim *sim)
{
	t_request	*top;

	top = heap_peek(&dongle->heap);
	if (top && top->coder == coder && dongle_state(dongle))
	{
		heap_pop(&dongle->heap);
		pthread_mutex_lock(&dongle->mutex);
		dongle->held = 1;
		pthread_mutex_unlock(&dongle->mutex);
		pthread_mutex_unlock(&sim->grant_mutex);
		log_status(coder, "has taken a dongle");
		return (1);
	}
	return (0);
}

int	request_dongle(t_coder *coder, t_dongle *dongle)
{
	t_sim	*sim;

	sim = coder->sim;
	pthread_mutex_lock(&sim->grant_mutex);
	push_request(coder, dongle);
	pthread_mutex_unlock(&sim->grant_mutex);
	while (!simulation_stopped(sim))
	{
		pthread_mutex_lock(&sim->grant_mutex);
		if (get_it(coder, dongle, sim))
			return (1);
		pthread_mutex_unlock(&sim->grant_mutex);
		if (simulation_stopped(sim))
			break ;
		usleep(200);
	}
	pthread_mutex_lock(&sim->grant_mutex);
	heap_remove(&dongle->heap, coder);
	pthread_mutex_unlock(&sim->grant_mutex);
	return (0);
}
