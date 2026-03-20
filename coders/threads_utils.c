/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   threads_utils.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bpasquer <bpasquer@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/25 15:30:00 by Baptiste          #+#    #+#             */
/*   Updated: 2026/03/25 17:53:25 by bpasquer         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	has_burned_out(t_coder *coder)
{
	pthread_mutex_lock(&coder->data->stop_mutex);
	if (coder->data->stop)
	{
		pthread_mutex_unlock(&coder->data->stop_mutex);
		return (1);
	}
	pthread_mutex_unlock(&coder->data->stop_mutex);
	return (0);
}

int	has_reach_max_compiles(t_coder *coder)
{
	pthread_mutex_lock(&coder->last_compile_mutex);
	if (coder->compile_count >= coder->data->number_of_compiles_required)
	{
		pthread_mutex_unlock(&coder->last_compile_mutex);
		return (1);
	}
	pthread_mutex_unlock(&coder->last_compile_mutex);
	return (0);
}

int	routine_2(t_coder *coder)
{
	print_state(coder, "is compiling");
	usleep((useconds_t)(coder->data->time_to_compile * 1000));
	pthread_mutex_lock(&coder->last_compile_mutex);
	coder->compile_count++;
	pthread_mutex_unlock(&coder->last_compile_mutex);
	if (has_reach_max_compiles(coder))
		return (0);
	print_state(coder, "is debugging");
	usleep((useconds_t)(coder->data->time_to_debug * 1000));
	print_state(coder, "is refactoring");
	usleep((useconds_t)(coder->data->time_to_refactor * 1000));
	return (1);
}
