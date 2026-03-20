/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   logs.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bpasquer <bpasquer@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/25 17:50:11 by bpasquer          #+#    #+#             */
/*   Updated: 2026/03/25 17:50:13 by bpasquer         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	print_state(t_coder *coder, char *message)
{
	long	timestamp;

	if (pthread_mutex_lock(&(coder->data->log_mutex)) != 0)
		return ;
	if (coder->data->stop)
	{
		pthread_mutex_unlock(&(coder->data->log_mutex));
		return ;
	}
	timestamp = get_time_ms() - coder->data->start_time;
	printf("%ld %d %s\n", timestamp, coder->id, message);
	if (pthread_mutex_unlock(&(coder->data->log_mutex)) != 0)
		return ;
}
