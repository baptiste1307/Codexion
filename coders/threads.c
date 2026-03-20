/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   threads.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bpasquer <bpasquer@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/25 15:30:00 by Baptiste          #+#    #+#             */
/*   Updated: 2026/03/25 17:51:01 by bpasquer         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	choose_direction(t_coder *coder, t_dongle **first, t_dongle **second)
{
	if ((coder->id % 2) == 0)
	{
		*first = coder->left_dgl;
		*second = coder->right_dgl;
	}
	else
	{
		*first = coder->right_dgl;
		*second = coder->left_dgl;
	}
}

void	*routine(void *arg)
{
	t_coder		*coder;
	t_dongle	*first;
	t_dongle	*second;

	coder = (t_coder *)arg;
	while (1)
	{
		if (has_burned_out(coder))
			break ;
		choose_direction(coder, &first, &second);
		if (lock_dongles(coder, first, second) != 0)
			break ;
		if (!routine_2(coder))
		{
			unlock_dongles(coder, first, second);
			break ;
		}
		if (unlock_dongles(coder, first, second) != 0)
			break ;
	}
	return (NULL);
}

t_coder	*init_coders(t_data *data_ptr)
{
	t_coder	*coders;
	int		coders_count;
	int		i;

	coders_count = data_ptr->number_of_coders;
	if (coders_count == 1
		&& data_ptr->time_to_burnout < data_ptr->time_to_compile)
		data_ptr->time_to_burnout = data_ptr->time_to_compile;
	coders = malloc(sizeof(t_coder) * coders_count);
	if (!coders)
		return (NULL);
	i = 0;
	while (i < coders_count)
	{
		coders[i].last_compile_time = data_ptr->start_time;
		coders[i].compile_count = 0;
		coders[i].id = i + 1;
		coders[i].data = data_ptr;
		coders[i].left_dgl = &(data_ptr->dongles[i]);
		coders[i].right_dgl = &(data_ptr->dongles[(i + 1) % coders_count]);
		coders[i].request_order = 0;
		pthread_mutex_init(&coders[i].last_compile_mutex, NULL);
		i++;
	}
	return (coders);
}

int	create_threads(t_coder *coders, t_data *data_ptr)
{
	int	i;

	i = 0;
	while (i < data_ptr->number_of_coders)
	{
		if (pthread_create(&coders[i].thread, NULL, routine, &coders[i]) != 0)
		{
			while (i-- > 0)
				pthread_join(coders[i].thread, NULL);
			return (1);
		}
		i++;
	}
	return (0);
}

void	join_threads(t_coder *coders, t_data *data_ptr)
{
	int	i;

	i = 0;
	while (i < data_ptr->number_of_coders)
	{
		pthread_join(coders[i].thread, NULL);
		pthread_mutex_destroy(&coders[i].last_compile_mutex);
		i++;
	}
}
