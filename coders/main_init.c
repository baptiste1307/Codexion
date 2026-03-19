/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main_init.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bpasquer <bpasquer@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/25 17:50:19 by bpasquer          #+#    #+#             */
/*   Updated: 2026/03/25 17:50:21 by bpasquer         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/**
 * @brief Validate the numeric values parsed from the command line.
 *
 * @param data_ptr Parsed shared data structure.
 * @return 1 if all values are acceptable, 0 otherwise.
 */
static int	has_valid_values(t_data *data_ptr)
{
	if (data_ptr->number_of_coders <= 0)
		return (0);
	if (data_ptr->time_to_burnout < 0 || data_ptr->time_to_compile < 0)
		return (0);
	if (data_ptr->time_to_debug < 0 || data_ptr->time_to_refactor < 0)
		return (0);
	if (data_ptr->number_of_compiles_required < 0)
		return (0);
	if (data_ptr->dongle_cooldown < 0)
		return (0);
	return (1);
}

/**
 * @brief Validate the scheduler string and emit the parsing error if needed.
 *
 * @param ac Number of command-line arguments.
 * @param av Command-line argument vector.
 * @param data_ptr Shared data receiving the scheduler.
 * @return 1 if scheduler is valid, 0 otherwise.
 */
static int	setup_scheduler(int ac, char **av, t_data *data_ptr)
{
	if (set_data_scheduler(ac, av, data_ptr))
		return (1);
	fprintf(stderr,
		"Error while parsing arguments: "
		"last one should be 'fifo' or 'edf'.\n");
	return (0);
}

/**
 * @brief Initialize runtime-only fields after parsing succeeds.
 *
 * @param data_ptr Shared data structure to prepare.
 * @param data Original stack object holding mutex fields.
 * @return 1 on success, 0 if allocation fails.
 */
static int	setup_runtime_data(t_data *data_ptr, t_data *data)
{
	data_ptr->dongles = NULL;
	data_ptr->coders = NULL;
	data_ptr->heap = malloc(sizeof(t_heap));
	if (!data_ptr->heap)
		return (0);
	data_ptr->heap->arr = NULL;
	data_ptr->heap->size = 0;
	data_ptr->heap->capacity = 0;
	data_ptr->start_time = get_time_ms();
	pthread_mutex_init(&data->log_mutex, NULL);
	pthread_mutex_init(&data->stop_mutex, NULL);
	return (1);
}

t_data	*init_data(int ac, char **av, t_data *data)
{
	t_data	*data_ptr;

	if (ac < 2)
		return (NULL);
	data_ptr = set_data(ac, av, data);
	if (!data_ptr || !has_valid_values(data_ptr))
		return (NULL);
	if (!setup_scheduler(ac, av, data_ptr))
		return (NULL);
	if (!setup_runtime_data(data_ptr, data))
		return (NULL);
	return (data_ptr);
}
