/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitor.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bpasquer <bpasquer@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/25 17:50:37 by bpasquer          #+#    #+#             */
/*   Updated: 2026/03/25 17:50:39 by bpasquer         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/**
 * @brief Check whether every coder reached the required compile count.
 *
 * @param coders Array of coders monitored by the simulation.
 * @return 1 if all coders are done, 0 otherwise.
 */
static int	all_coders_done(t_coder *coders)
{
	int	i;
	int	required;

	required = coders[0].data->number_of_compiles_required;
	if (required <= 0)
		return (0);
	i = 0;
	while (i < coders[0].data->number_of_coders)
	{
		pthread_mutex_lock(&coders[i].last_compile_mutex);
		if (coders[i].compile_count < required)
		{
			pthread_mutex_unlock(&coders[i].last_compile_mutex);
			return (0);
		}
		pthread_mutex_unlock(&coders[i].last_compile_mutex);
		i++;
	}
	return (1);
}

void	set_stop_flag(t_coder *coder)
{
	pthread_mutex_lock(&coder->data->stop_mutex);
	if (coder->data->stop)
	{
		pthread_mutex_unlock(&coder->data->stop_mutex);
		return ;
	}
	coder->data->stop = 1;
	pthread_mutex_unlock(&coder->data->stop_mutex);
}

void	monitor_routine_2(t_coder *coders, int i)
{
	pthread_mutex_lock(&coders[0].data->stop_mutex);
	if (!coders[0].data->stop)
	{
		print_state(&coders[i], "burned out");
		coders[0].data->stop = 1;
	}
	pthread_mutex_unlock(&coders[0].data->stop_mutex);
}

void	*monitor_routine(void *arg)
{
	int			i;
	long		last;
	t_coder		*coders;

	coders = (t_coder *)arg;
	while (1)
	{
		i = 0;
		while (i < coders[0].data->number_of_coders)
		{
			pthread_mutex_lock(&coders[i].last_compile_mutex);
			last = coders[i].last_compile_time;
			pthread_mutex_unlock(&coders[i].last_compile_mutex);
			if ((get_time_ms() - last) > coders[0].data->time_to_burnout)
				return (monitor_routine_2(coders, i), NULL);
			i++;
		}
		if (all_coders_done(coders))
			return (set_stop_flag(&coders[0]), NULL);
		usleep(1000);
	}
}
