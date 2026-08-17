/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heap.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yikoubaz <yikoubaz@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/23 10:39:56 by yikoubaz          #+#    #+#             */
/*   Updated: 2026/08/17 13:00:24 by yikoubaz         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	heap_init(t_heap *heap, int capacity)
{
	heap->array = malloc(capacity * sizeof(t_request));
	if (!heap->array)
		return (0);
	heap->size = 0;
	heap->capacity = capacity;
	return (1);
}

// Push item to heap

int	heap_push(t_heap *heap, t_request *req)
{
	if (heap->size == heap->capacity)
		return (0);
	heap->array[heap->size] = *req;
	heap->size++;
	heapify_up(heap, heap->size - 1);
	return (1);
}

t_request	*heap_peek(t_heap *heap)
{
	if (!heap->size)
		return (NULL);
	return (&heap->array[0]);
}

t_request	heap_pop(t_heap *heap)
{
	t_request	root;

	if (heap->size == 0)
	{
		root.coder = NULL;
		root.priority = 0;
		root.order = 0;
		return (root);
	}
	root = heap->array[0];
	heap->array[0] = heap->array[heap->size - 1];
	heap->size--;
	if (heap->size > 0)
		heapify_down(heap, 0);
	return (root);
}

void	heap_destroy(t_heap *heap)
{
	if (!heap->array)
		return ;
	free(heap->array);
	heap->array = NULL;
	heap->size = 0;
	heap->capacity = 0;
}
