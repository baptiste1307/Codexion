/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   fifo_edf_utils_3.c                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bpasquer <bpasquer@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/25 17:49:57 by bpasquer          #+#    #+#             */
/*   Updated: 2026/03/25 17:49:59 by bpasquer         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	heap_remove_at_locked(t_heap *heap, int index)
{
	if (index < 0 || index >= heap->size)
		return ;
	heap->arr[index] = heap->arr[heap->size - 1];
	heap->size--;
	if (index < heap->size)
	{
		go_down_in_heap(heap, index);
		go_up_in_heap(heap, index);
	}
}
