/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yikoubaz <yikoubaz@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/18 00:44:23 by yikoubaz          #+#    #+#             */
/*   Updated: 2026/07/28 10:48:35 by yikoubaz         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	is_space(char c)
{
	return ((c >= 9 && c <= 13) || c == 32);
}

long	ft_atol(char *s)
{
	long	r;
	int		i;

	r = 0;
	i = 0;
	while (s[i] && is_space(s[i]))
		i++;
	while (s[i])
		r = r * 10 + (s[i++] - 48);
	return (r);
}

void	get_timeout(struct timespec *ts, long ms)
{
	clock_gettime(CLOCK_REALTIME, ts);
	ts->tv_sec += ms / 1000;
	ts->tv_nsec += (ms % 1000) * 1000000;
	if (ts->tv_nsec >= 1000000000)
	{
		ts->tv_sec++;
		ts->tv_nsec -= 1000000000;
	}
}

long	get_time(void)
{
	struct timeval	tv;

	if (gettimeofday(&tv, NULL) != 0)
		return (0);
	return (tv.tv_sec * 1000L + tv.tv_usec / 1000);
}

void	smart_sleep(long ms, t_sim *sim)
{
	long	start;

	start = get_time();
	while (!simulation_stopped(sim) && (get_time() - start) < ms)
		usleep(500);
}
