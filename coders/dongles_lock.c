/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongles_lock.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bpasquer <bpasquer@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/25 17:49:22 by bpasquer          #+#    #+#             */
/*   Updated: 2026/03/25 17:49:24 by bpasquer         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/**
 * @brief Check whether a coder can use its two neighboring dongles right now.
 *
 * The order is normalized before probing the pair so the same physical dongles
 * are always checked in a consistent order.
 *
 * @param coder Coder whose pair availability is tested.
 * @return 1 if both dongles are currently usable, 0 otherwise.
 */
static int	coder_pair_available(t_coder *coder)
{
	t_dongle	*first;
	t_dongle	*second;

	first = coder->left_dgl;
	second = coder->right_dgl;
	if (first > second)
		return (pair_is_available(second, first));
	return (pair_is_available(first, second));
}

/**
 * @brief Compare two heap candidates using the active scheduling policy.
 *
 * FIFO compares request order, EDF compares computed deadline, and request
 * order is reused as a tie-breaker for deterministic selection.
 *
 * @param heap Scheduling heap containing the candidates.
 * @param candidate Index of the newly inspected candidate.
 * @param best Index of the current best candidate, or -1 if none exists.
 * @return 1 if candidate is better than best, 0 otherwise.
 */
static int	is_higher_priority(t_heap *heap, int candidate, int best)
{
	long	candidate_value;
	long	best_value;

	if (best < 0)
		return (1);
	candidate_value = get_priority_value(heap, heap->arr[candidate]);
	best_value = get_priority_value(heap, heap->arr[best]);
	if (candidate_value < best_value)
		return (1);
	if (candidate_value == best_value
		&& heap->arr[candidate]->request_order
		< heap->arr[best]->request_order)
		return (1);
	return (0);
}

/**
 * @brief Find the best waiting coder whose pair is actually available.
 *
 * The scheduler only selects requests that can take their two dongles
 * immediately so one blocked pair does not freeze unrelated pairs.
 *
 * @param heap Heap of pending coder requests.
 * @return Index of the best feasible coder, or -1 if none exists.
 */
static int	find_best_available_coder(t_heap *heap)
{
	int	i;
	int	best;

	i = 0;
	best = -1;
	while (i < heap->size)
	{
		if (coder_pair_available(heap->arr[i])
			&& is_higher_priority(heap, i, best))
			best = i;
		i++;
	}
	return (best);
}

/**
 * @brief Insert a coder request into the heap with a monotonic request id.
 *
 * @param heap Shared scheduling heap.
 * @param coder Coder being registered as waiting for a compile slot.
 */
static void	register_request(t_heap *heap, t_coder *coder)
{
	pthread_mutex_lock(&heap->mutex);
	coder->request_order = coder->data->next_request_order++;
	heap_push_locked(heap, coder);
	pthread_mutex_unlock(&heap->mutex);
}

int	wait_turn_and_take(t_coder *coder, t_dongle *first, t_dongle *second)
{
	t_heap	*heap;
	int		best;

	heap = coder->data->heap;
	register_request(heap, coder);
	while (!has_burned_out(coder))
	{
		pthread_mutex_lock(&heap->mutex);
		best = find_best_available_coder(heap);
		if (best >= 0 && heap->arr[best] == coder
			&& try_take_pair(first, second))
		{
			heap_remove_at_locked(heap, best);
			pthread_mutex_unlock(&heap->mutex);
			set_compile_start(coder);
			print_state(coder, "has taken a dongle");
			print_state(coder, "has taken a dongle");
			return (0);
		}
		pthread_mutex_unlock(&heap->mutex);
		usleep(1000);
	}
	return (1);
}
