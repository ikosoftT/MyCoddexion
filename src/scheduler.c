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

static void	push_request(t_coder *coder, t_dongle *dongle)
{
	t_request	req;

	req = build_request(coder);
	heap_push(&dongle->heap, &req);
}

static int	dongle_state(t_dongle *dongle)
{
	int	free;

	pthread_mutex_lock(&dongle->mutex);
	free = (!dongle->held && get_time() >= dongle->cooldown_until);
	pthread_mutex_unlock(&dongle->mutex);
	return (free);
}

static t_dongle	*get_other(t_sim *sim, t_coder *coder, t_dongle *dongle)
{
	if (&sim->dongles[coder->left] == dongle)
		return (&sim->dongles[coder->right]);
	return (&sim->dongles[coder->left]);
}

static int	other_blocked(t_sim *sim, t_coder *coder, t_dongle *dongle)
{
	t_dongle	*other;
	int			blocked;

	other = get_other(sim, coder, dongle);
	pthread_mutex_lock(&other->mutex);
	blocked = other->held || get_time() < other->cooldown_until;
	pthread_mutex_unlock(&other->mutex);
	return (blocked);
}

static int	is_eligible(t_sim *sim, t_dongle *dongle, t_coder *coder)
{
	t_request	mine;
	t_request	*r;
	int			i;
	int			found;

	mine.coder = NULL;
	found = 0;
	i = 0;
	while (i < dongle->heap.size)
	{
		if (dongle->heap.array[i].coder == coder)
		{
			mine = dongle->heap.array[i];
			found = 1;
		}
		i++;
	}
	if (!found)
		return (0);
	i = 0;
	while (i < dongle->heap.size)
	{
		r = &dongle->heap.array[i];
		if (r->coder != coder
			&& (r->priority < mine.priority
				|| (r->priority == mine.priority
					&& r->coder->id < coder->id)))
		{
			if (!other_blocked(sim, r->coder, dongle))
				return (0);
		}
		i++;
	}
	return (1);
}

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
			log_status(coder, "has taken a dongle");
			log_status(coder, "has taken a dongle");
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

int	request_dongle(t_coder *coder, t_dongle *dongle)
{
	t_sim		*sim;
	t_request	*top;

	sim = coder->sim;
	pthread_mutex_lock(&sim->grant_mutex);
	push_request(coder, dongle);
	pthread_mutex_unlock(&sim->grant_mutex);
	while (!simulation_stopped(sim))
	{
		pthread_mutex_lock(&sim->grant_mutex);
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
