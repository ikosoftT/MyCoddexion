/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   scheduler.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yikoubaz <yikoubaz@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/23 08:28:35 by yikoubaz          #+#    #+#             */
/*   Updated: 2026/08/03 17:55:07 by yikoubaz         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static t_request	build_request(t_coder *coder)
{
	t_sim		*sim;
	t_request	req;

	sim = coder->sim;
	pthread_mutex_lock(&sim->scheduler_mutex);
	req.order = sim->request_counter++;
	pthread_mutex_unlock(&sim->scheduler_mutex);
	if (sim->data.scheduler == FIFO)
		req.priority = req.order;
	else
	{
		pthread_mutex_lock(&coder->state_mutex);
		req.priority = coder->last_compile + sim->data.time_to_burnout;
		pthread_mutex_unlock(&coder->state_mutex);
		req.order = sim->request_counter++;
	}
	req.coder = coder;
	return (req);
}

static int	wait_for_acquire(t_coder *coder, t_dongle *dongle,
		long end_time, long poll_ms)
{
	t_request		*top;
	struct timespec	timeout;

	while (!simulation_stopped(coder->sim))
	{
		top = heap_peek(&dongle->heap);
		if (top && top->coder == coder && !dongle->held
			&& get_time() >= dongle->cooldown_until)
		{
			heap_pop(&dongle->heap);
			dongle->held = 1;
			log_status(coder, "has taken a dongle");
			return (1);
		}
		if (end_time >= 0 && get_time() >= end_time)
			break ;
		get_timeout(&timeout, poll_ms);
		pthread_cond_timedwait(&dongle->cond, &dongle->mutex, &timeout);
	}
	return (0);
}

int	request_dongle(t_coder *coder, t_dongle *dongle)
{
	t_request	req;

	req = build_request(coder);
	pthread_mutex_lock(&dongle->mutex);
	if (!heap_push(&dongle->heap, &req))
	{
		pthread_mutex_unlock(&dongle->mutex);
		return (0);
	}
	if (wait_for_acquire(coder, dongle, -1, 10))
	{
		pthread_mutex_unlock(&dongle->mutex);
		return (1);
	}
	heap_remove(&dongle->heap, coder);
	pthread_cond_broadcast(&dongle->cond);
	pthread_mutex_unlock(&dongle->mutex);
	return (0);
}

void	release_dongle(t_coder *coder, t_dongle *dongle)
{
	pthread_mutex_lock(&dongle->mutex);
	dongle->held = 0;
	dongle->cooldown_until = get_time() + coder->sim->data.dongle_cooldown;
	pthread_cond_broadcast(&dongle->cond);
	pthread_mutex_unlock(&dongle->mutex);
}

int	request_dongle_timeout(t_coder *coder, t_dongle *dongle, long timeout_ms)
{
	t_request	req;

	req = build_request(coder);
	pthread_mutex_lock(&dongle->mutex);
	if (!heap_push(&dongle->heap, &req))
	{
		pthread_mutex_unlock(&dongle->mutex);
		return (0);
	}
	if (wait_for_acquire(coder, dongle, get_time() + timeout_ms, 1))
	{
		pthread_mutex_unlock(&dongle->mutex);
		return (1);
	}
	heap_remove(&dongle->heap, coder);
	pthread_cond_broadcast(&dongle->cond);
	pthread_mutex_unlock(&dongle->mutex);
	return (0);
}
