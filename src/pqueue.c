/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   pqueue.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yikoubaz <yikoubaz@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/01 12:30:00 by yikoubaz          #+#    #+#             */
/*   Updated: 2026/07/01 17:21:51 by yikoubaz         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/coddexion.h"


static int	pq_higher(t_request *a, t_request *b)
{
	if (a->priority != b->priority)
		return (a->priority < b->priority);
	return (a->order < b->order);
}

void	pq_init(t_pqueue *pq, int cap)
{
	pq->data = malloc(sizeof(t_request) * cap);
	pq->size = 0;
	pq->cap = cap;
}

static void	swap(t_request *a, t_request *b)
{
	t_request	tmp;

	tmp = *a;
	*a = *b;
	*b = tmp;
}

void	pq_push(t_pqueue *pq, t_request req)
{
	int	i;
	int	p;

	if (pq->size >= pq->cap)
		return ;
	i = pq->size++;
	pq->data[i] = req;
	while (i > 0)
	{
		p = (i - 1) / 2;
		if (pq_higher(&pq->data[i], &pq->data[p]))
		{
			swap(&pq->data[i], &pq->data[p]);
			i = p;
		}
		else
			break ;
	}
}

t_request	*pq_peek(t_pqueue *pq)
{
	if (pq->size == 0)
		return (NULL);
	return (&pq->data[0]);
}

t_request	pq_pop(t_pqueue *pq)
{
	t_request	result;
	int			i;
	int			left;
	int			right;
	int			smallest;

	result = pq->data[0];
	pq->data[0] = pq->data[--pq->size];
	i = 0;
	while (1)
	{
		left = 2 * i + 1;
		right = 2 * i + 2;
		smallest = i;
		if (left < pq->size && pq_higher(&pq->data[left], &pq->data[smallest]))
			smallest = left;
		if (right < pq->size && pq_higher(&pq->data[right],
				&pq->data[smallest]))
			smallest = right;
		if (smallest != i)
		{
			swap(&pq->data[i], &pq->data[smallest]);
			i = smallest;
		}
		else
			break ;
	}
	return (result);
}

void	pq_clear(t_pqueue *pq)
{
	free(pq->data);
	pq->data = NULL;
	pq->size = 0;
	pq->cap = 0;
}

void	pq_remove_coder(t_pqueue *pq, int coder_id)
{
	int	i;
	int	found;
	int	p;

	i = 0;
	found = -1;
	while (i < pq->size)
	{
		if (pq->data[i].coder_id == coder_id)
		{
			found = i;
			break ;
		}
		i++;
	}
	if (found < 0)
		return ;
	pq->data[found] = pq->data[--pq->size];
	i = found;
	while (i > 0)
	{
		p = (i - 1) / 2;
		if (pq_higher(&pq->data[i], &pq->data[p]))
		{
			swap(&pq->data[i], &pq->data[p]);
			i = p;
		}
		else
			break ;
	}
}
