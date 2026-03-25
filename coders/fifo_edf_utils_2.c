/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   fifo_edf_utils_2.c                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bpasquer <bpasquer@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/25 17:49:49 by bpasquer          #+#    #+#             */
/*   Updated: 2026/03/25 17:49:52 by bpasquer         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	heap_push_locked(t_heap *heap, t_coder *c)
{
	heap->arr[heap->size] = c;
	go_up_in_heap(heap, heap->size);
	heap->size++;
}

void	heap_push(t_heap *heap, t_coder *c)
{
	pthread_mutex_lock(&heap->mutex);
	heap_push_locked(heap, c);
	pthread_mutex_unlock(&heap->mutex);
}

t_coder	*heap_peek(t_heap *heap)
{
	if (heap->size == 0)
		return (NULL);
	return (heap->arr[0]);
}

void	heap_pop_locked(t_heap *heap)
{
	if (heap->size == 0)
		return ;
	heap->arr[0] = heap->arr[heap->size - 1];
	heap->size--;
	go_down_in_heap(heap, 0);
}

void	heap_pop(t_heap *heap)
{
	pthread_mutex_lock(&heap->mutex);
	if (heap->size == 0)
	{
		pthread_mutex_unlock(&heap->mutex);
		return ;
	}
	heap_pop_locked(heap);
	pthread_mutex_unlock(&heap->mutex);
}
