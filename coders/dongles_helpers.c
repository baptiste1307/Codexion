/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongles_helpers.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bpasquer <bpasquer@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/25 17:48:58 by bpasquer          #+#    #+#             */
/*   Updated: 2026/03/25 17:49:07 by bpasquer         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/**
 * @brief Check whether two dongles are both past their cooldown timestamp.
 *
 * @param first First dongle to inspect.
 * @param second Second dongle to inspect.
 * @param now Current timestamp used as comparison reference.
 * @return 1 if both dongles are available now, 0 otherwise.
 */
static int	dongles_ready(t_dongle *first, t_dongle *second, long now)
{
	if (first->available_at > now || second->available_at > now)
		return (0);
	return (1);
}

int	pair_is_available(t_dongle *first, t_dongle *second)
{
	long	now;

	if (pthread_mutex_trylock(&(first->mutex)) != 0)
		return (0);
	if (pthread_mutex_trylock(&(second->mutex)) != 0)
		return (pthread_mutex_unlock(&(first->mutex)), 0);
	now = get_time_ms();
	if (!dongles_ready(first, second, now))
	{
		pthread_mutex_unlock(&(second->mutex));
		pthread_mutex_unlock(&(first->mutex));
		return (0);
	}
	pthread_mutex_unlock(&(second->mutex));
	pthread_mutex_unlock(&(first->mutex));
	return (1);
}

int	try_take_pair(t_dongle *first, t_dongle *second)
{
	long	now;

	if (pthread_mutex_trylock(&(first->mutex)) != 0)
		return (0);
	if (pthread_mutex_trylock(&(second->mutex)) != 0)
		return (pthread_mutex_unlock(&(first->mutex)), 0);
	now = get_time_ms();
	if (!dongles_ready(first, second, now))
	{
		pthread_mutex_unlock(&(second->mutex));
		pthread_mutex_unlock(&(first->mutex));
		return (0);
	}
	return (1);
}

void	set_compile_start(t_coder *coder)
{
	pthread_mutex_lock(&coder->last_compile_mutex);
	coder->last_compile_time = get_time_ms();
	pthread_mutex_unlock(&coder->last_compile_mutex);
}

int	wait_single_dongle(t_coder *coder)
{
	long	now;

	while (!has_burned_out(coder))
	{
		if (pthread_mutex_lock(&coder->left_dgl->mutex) != 0)
			return (1);
		now = get_time_ms();
		if (coder->left_dgl->available_at <= now)
			return (set_compile_start(coder),
				print_state(coder, "has taken a dongle"), 0);
		pthread_mutex_unlock(&coder->left_dgl->mutex);
		usleep(1000);
	}
	return (1);
}
