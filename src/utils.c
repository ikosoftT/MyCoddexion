/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yikoubaz <yikoubaz@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/28 11:56:30 by yikoubaz          #+#    #+#             */
/*   Updated: 2026/07/01 17:21:58 by yikoubaz         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/coddexion.h"


static int	is_space(char c)
{
	return ((c >= 9 && c <= 13) || c == 32);
}

long	ft_atol(char *s)
{
	long	r;
	short	i;

	r = 0;
	i = 0;
	while (s[i] && is_space(s[i]))
		i++;
	while (s[i] && s[i] >= '0' && s[i] <= '9')
		r = r * 10 + (s[i++] - 48);
	return (r);
}

int	is_num(char *arg)
{
	int	i;

	i = 0;
	while (arg[i] && is_space(arg[i]))
		i++;
	while (arg[i])
	{
		if (arg[i] < '0' || arg[i] > '9')
			return (0);
		i++;
	}
	return (1);
}

int	is_valid_int(char **av)
{
	int	i;

	i = 1;
	while (i <= 6)
	{
		if (ft_atol(av[i]) <= 0)
			return (0);
		i++;
	}
	if (ft_atol(av[7]) < 0)
		return (0);
	return (1);
}

long	get_time_ms(void)
{
	struct timeval	tv;

	gettimeofday(&tv, NULL);
	return ((long)tv.tv_sec * 1000 + (long)tv.tv_usec / 1000);
}

void	precise_usleep(long us)
{
	long	start;
	long	elapsed;

	start = get_time_ms();
	while (1)
	{
		elapsed = get_time_ms() - start;
		if (elapsed * 1000 >= us)
			break ;
		if (us - elapsed * 1000 > 1000)
			usleep(1000);
		else
			usleep(100);
	}
}
