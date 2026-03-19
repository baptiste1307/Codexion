/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bpasquer <bpasquer@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/25 15:30:00 by Baptiste          #+#    #+#             */
/*   Updated: 2026/03/25 17:50:31 by bpasquer         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/**
 * @brief Create simulation resources that depend on parsed runtime data.
 *
 * This includes dongles and the coder array stored back into the shared data
 * structure.
 *
 * @param data_ptr Shared simulation data.
 * @return Pointer to the initialized coder array, or NULL on failure.
 */
static t_coder	*init_resources(t_data *data_ptr)
{
	t_coder	*coders;

	if (set_dongles(data_ptr) != 0)
		return (NULL);
	coders = init_coders(data_ptr);
	if (!coders)
		return (NULL);
	data_ptr->coders = coders;
	return (coders);
}

/**
 * @brief Start worker threads and the dedicated monitor thread.
 *
 * @param coders Array of initialized coders.
 * @param data_ptr Shared simulation data.
 * @param monitor Output pointer receiving the monitor thread handle.
 * @return 0 on success, 1 if thread creation fails.
 */
static int	start_threads(t_coder *coders, t_data *data_ptr,
	pthread_t *monitor)
{
	if (create_threads(coders, data_ptr) != 0)
		return (1);
	pthread_create(monitor, NULL, monitor_routine, coders);
	return (0);
}

/**
 * @brief Join threads and free every runtime resource before exit.
 *
 * @param coders Array of coder structures.
 * @param data_ptr Shared simulation data.
 * @param monitor Monitor thread handle to join.
 */
static void	cleanup(t_coder *coders, t_data *data_ptr,
	pthread_t monitor)
{
	join_threads(coders, data_ptr);
	pthread_join(monitor, NULL);
	destroy_dongles(data_ptr);
	pthread_mutex_destroy(&data_ptr->log_mutex);
	pthread_mutex_destroy(&data_ptr->stop_mutex);
	free(data_ptr->heap);
	free(coders);
}

int	main(int ac, char **av)
{
	t_data		data;
	t_data		*data_ptr;
	t_coder		*coders;
	pthread_t	monitor;

	data_ptr = init_data(ac, av, &data);
	if (!data_ptr)
		return (1);
	coders = init_resources(data_ptr);
	if (!coders)
		return (pthread_mutex_destroy(&data_ptr->log_mutex),
			pthread_mutex_destroy(&data_ptr->stop_mutex),
			free(data_ptr->heap), 1);
	if (start_threads(coders, data_ptr, &monitor))
		return (free(coders), destroy_dongles(data_ptr),
			pthread_mutex_destroy(&data_ptr->log_mutex),
			pthread_mutex_destroy(&data_ptr->stop_mutex),
			free(data_ptr->heap), 1);
	cleanup(coders, data_ptr, monitor);
	return (0);
}
