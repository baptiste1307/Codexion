/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   fifo_edf_utils.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bpasquer <bpasquer@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/25 17:50:04 by bpasquer          #+#    #+#             */
/*   Updated: 2026/03/25 17:50:07 by bpasquer         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	heap_swap(t_coder **a, t_coder **b)
{
	t_coder	*tmp;

	tmp = *a;
	*a = *b;
	*b = tmp;
}

long	get_deadline(t_coder *c)
{
	long	value;

	pthread_mutex_lock(&c->last_compile_mutex);
	value = c->last_compile_time;
	pthread_mutex_unlock(&c->last_compile_mutex);
	return (value + c->data->time_to_burnout);
}

long	get_priority_value(t_heap *heap, t_coder *c)
{
	if (ft_strcmp(c->data->scheduler, "fifo") == 0)
		return (c->request_order);
	(void)heap;
	return (get_deadline(c));
}

void	go_up_in_heap(t_heap *heap, int i)
{
	int	parent;

	while (i > 0)
	{
		parent = (i - 1) / 2;
		if (get_priority_value(heap, heap->arr[parent])
			<= get_priority_value(heap, heap->arr[i]))
			break ;
		heap_swap(&heap->arr[parent], &heap->arr[i]);
		i = parent;
	}
}

void	go_down_in_heap(t_heap *heap, int i)
{
	int	left;
	int	right;
	int	smallest;

	while (1)
	{
		left = 2 * i + 1;
		right = 2 * i + 2;
		smallest = i;
		if (left < heap->size
			&& (get_priority_value(heap, heap->arr[left])
				< get_priority_value(heap, heap->arr[smallest])))
			smallest = left;
		if (right < heap->size
			&& (get_priority_value(heap, heap->arr[right])
				< get_priority_value(heap, heap->arr[smallest])))
			smallest = right;
		if (smallest == i)
			break ;
		heap_swap(&heap->arr[i], &heap->arr[smallest]);
		i = smallest;
	}
}
