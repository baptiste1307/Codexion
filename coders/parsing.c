/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parsing.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bpasquer <bpasquer@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/25 15:30:00 by Baptiste          #+#    #+#             */
/*   Updated: 2026/03/25 17:50:45 by bpasquer         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/**
 * @brief Parse one non-negative decimal string into an int.
 *
 * @param str Input string to validate and convert.
 * @param value Output pointer receiving the parsed integer.
 * @return 1 on success, 0 if the string is invalid.
 */
static int	parse_int_arg(const char *str, int *value)
{
	long	result;
	int		i;

	if (!str || !str[0])
		return (0);
	i = 0;
	result = 0;
	while (str[i])
	{
		if (str[i] < '0' || str[i] > '9')
			return (0);
		result = (result * 10) + (str[i] - '0');
		if (result > 2147483647)
			return (0);
		i++;
	}
	*value = (int)result;
	return (1);
}

int	ft_strcmp(const char *s1, const char *s2)
{
	int	_i;

	_i = 0;
	while (s1[_i] == s2[_i] && s1[_i] != '\0')
		_i++;
	return ((unsigned char)s1[_i] - (unsigned char)s2[_i]);
}

int	set_data_scheduler(int ac, char **av, t_data *data)
{
	if (ac != 9 || (ft_strcmp(av[8], "fifo") && ft_strcmp(av[8], "edf")))
		return (0);
	data->scheduler = av[8];
	return (1);
}

/**
 * @brief Copy parsed numeric arguments into the shared data structure.
 *
 * @param data Shared simulation data to fill.
 * @param values Parsed numeric values in subject argument order.
 */
static void	store_values(t_data *data, int values[7])
{
	data->number_of_coders = values[0];
	data->time_to_burnout = values[1];
	data->time_to_compile = values[2];
	data->time_to_debug = values[3];
	data->time_to_refactor = values[4];
	data->number_of_compiles_required = values[5];
	data->dongle_cooldown = values[6];
	data->stop = 0;
	data->next_request_order = 0;
}

t_data	*set_data(int ac, char **av, t_data *data)
{
	int	values[7];
	int	i;

	i = 0;
	while (i < 7)
	{
		values[i] = 0;
		i++;
	}
	i = 1;
	while (i < ac && i <= 7)
	{
		if (!parse_int_arg(av[i], &values[i - 1]))
			return (NULL);
		i++;
	}
	store_values(data, values);
	return (data);
}
