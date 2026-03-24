/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongles.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bpasquer <bpasquer@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/25 17:49:40 by bpasquer          #+#    #+#             */
/*   Updated: 2026/03/25 17:49:42 by bpasquer         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/**
 * @brief Destroy a given number of initialized dongle mutexes.
 *
 * This helper is used on partial failure paths when the dongle array exists but
 * the initialization sequence did not complete.
 *
 * @param dongles Array of dongles to clean up.
 * @param count Number of dongles whose mutexes must be destroyed.
 */
static void	cleanup_dongles(t_dongle *dongles, int count)
{
	while (count-- > 0)
		pthread_mutex_destroy(&(dongles[count].mutex));
}

int	set_dongles(t_data *data_ptr)
{
	t_dongle	*dongles;

	if (!data_ptr || !data_ptr->heap)
		return (1);
	dongles = malloc(sizeof (t_dongle) * data_ptr->number_of_coders);
	if (!dongles)
		return (1);
	data_ptr->dongles = dongles;
	if (!init_dongle_mutexes(dongles, data_ptr->number_of_coders))
		return (free(dongles), data_ptr->dongles = NULL, 1);
	data_ptr->heap->arr = malloc(sizeof(t_coder *)
			* data_ptr->number_of_coders);
	if (!data_ptr->heap->arr)
	{
		cleanup_dongles(dongles, data_ptr->number_of_coders);
		free(dongles);
		data_ptr->dongles = NULL;
		return (1);
	}
	data_ptr->heap->size = 0;
	data_ptr->heap->capacity = data_ptr->number_of_coders;
	pthread_mutex_init(&data_ptr->heap->mutex, NULL);
	return (0);
}

int	lock_dongles(t_coder *coder, t_dongle *first, t_dongle *second)
{
	if (coder->data->number_of_coders == 1)
		return (wait_single_dongle(coder));
	if (first > second)
		return (wait_turn_and_take(coder, second, first));
	return (wait_turn_and_take(coder, first, second));
}

int	unlock_dongles(t_coder *coder, t_dongle *first, t_dongle *second)
{
	long	release_time;

	release_time = get_time_ms() + coder->data->dongle_cooldown;
	if (coder->data->number_of_coders == 1)
	{
		coder->left_dgl->available_at = release_time;
		if (pthread_mutex_unlock(&coder->left_dgl->mutex) != 0)
			return (1);
		return (0);
	}
	second->available_at = release_time;
	if (pthread_mutex_unlock(&(second->mutex)) != 0)
		return (1);
	first->available_at = release_time;
	if (pthread_mutex_unlock(&(first->mutex)) != 0)
		return (1);
	return (0);
}

int	destroy_dongles(t_data *data_ptr)
{
	int	i;

	i = 0;
	if (! data_ptr || !data_ptr->dongles)
		return (1);
	while (i < data_ptr->number_of_coders)
	{
		pthread_mutex_destroy(&(data_ptr->dongles[i].mutex));
		i++;
	}
	free(data_ptr->dongles);
	data_ptr->dongles = NULL;
	free(data_ptr->heap->arr);
	pthread_mutex_destroy(&data_ptr->heap->mutex);
	return (0);
}
