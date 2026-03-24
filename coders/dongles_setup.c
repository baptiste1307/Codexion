/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongles_setup.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bpasquer <bpasquer@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/25 17:49:30 by bpasquer          #+#    #+#             */
/*   Updated: 2026/03/25 17:49:32 by bpasquer         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/**
 * @brief Destroy the subset of dongles already initialized successfully.
 *
 * @param dongles Array of dongles being initialized.
 * @param count Number of valid mutexes to destroy.
 */
static void	destroy_partial_dongles(t_dongle *dongles, int count)
{
	while (count-- > 0)
		pthread_mutex_destroy(&(dongles[count].mutex));
}

int	init_dongle_mutexes(t_dongle *dongles, int count)
{
	int	i;

	i = 0;
	while (i < count)
	{
		if (pthread_mutex_init(&(dongles[i].mutex), NULL) != 0)
			return (destroy_partial_dongles(dongles, i), 0);
		dongles[i].available_at = 0;
		i++;
	}
	return (1);
}
