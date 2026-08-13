/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   scheduler_utils.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yikoubaz <yikoubaz@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/13 21:33:18 by yikoubaz          #+#    #+#             */
/*   Updated: 2026/08/13 22:23:27 by yikoubaz         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "./codexion.h"

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

void	push_request(t_coder *coder, t_dongle *dongle)
{
	t_request	req;

	req = build_request(coder);
	heap_push(&dongle->heap, &req);
}

int	dongle_state(t_dongle *dongle)
{
	int	free;

	pthread_mutex_lock(&dongle->mutex);
	free = (!dongle->held && get_time() >= dongle->cooldown_until);
	pthread_mutex_unlock(&dongle->mutex);
	return (free);
}

t_dongle	*get_other(t_sim *sim, t_coder *coder, t_dongle *dongle)
{
	if (&sim->dongles[coder->left] == dongle)
		return (&sim->dongles[coder->right]);
	return (&sim->dongles[coder->left]);
}

int	is_eligible(t_sim *sim, t_dongle *dongle, t_coder *coder)
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
