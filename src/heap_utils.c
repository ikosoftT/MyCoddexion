/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heap_utils.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yikoubaz <yikoubaz@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/22 09:56:15 by yikoubaz          #+#    #+#             */
/*   Updated: 2026/07/29 11:54:44 by yikoubaz         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	ft_swap(t_request *a, t_request *b)
{
	t_request	t;

	t = *a;
	*a = *b;
	*b = t;
}

static int	less(t_request a, t_request b)
{
	if (a.priority != b.priority)
		return (a.priority < b.priority);
	return (a.order < b.order);
}

// Removes An Itemfrom Heap then Re Sort it
int	heap_remove(t_heap *heap, t_coder *coder)
{
	int	i;

	i = 0;
	while (i < heap->size)
	{
		if (heap->array[i].coder == coder)
			break ;
		i++;
	}
	if (i == heap->size)
		return (0);
	heap->array[i] = heap->array[heap->size - 1];
	heap->size--;
	if (i > 0)
	{
		if (i < heap->size && less(heap->array[i], heap->array[(i - 1) / 2]))
			heapify_up(heap, i);
		else
			heapify_down(heap, i);
	}
	return (1);
}

void	heapify_up(t_heap *heap, int i)
{
	int	parent;

	while (i > 0)
	{
		parent = (i - 1) / 2;
		if (!less(heap->array[i], heap->array[parent]))
			break ;
		ft_swap(&heap->array[parent], &heap->array[i]);
		i = parent;
	}
}

void	heapify_down(t_heap *heap, int i)
{
	int	smallest;
	int	left;
	int	right;

	while (1)
	{
		smallest = i;
		left = 2 * i + 1;
		right = 2 * i + 2;
		if (left < heap->size && less(heap->array[left], heap->array[smallest]))
			smallest = left;
		if (right < heap->size && less(heap->array[right],
				heap->array[smallest]))
			smallest = right;
		if (smallest == i)
			break ;
		ft_swap(&heap->array[i], &heap->array[smallest]);
		i = smallest;
	}
}
