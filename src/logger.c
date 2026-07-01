/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   logger.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yikoubaz <yikoubaz@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/28 10:21:27 by yikoubaz          #+#    #+#             */
/*   Updated: 2026/07/01 17:21:23 by yikoubaz         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/coddexion.h"

void	log_action(t_sim *sim, int coder_id, char *msg)
{
	long	ts;

	pthread_mutex_lock(&sim->print_mutex);
	if (sim->stopped && strcmp(msg, "burned out"))
	{
		pthread_mutex_unlock(&sim->print_mutex);
		return ;
	}
	ts = get_time_ms() - sim->start_time;
	printf("%ld %d %s\n", ts, coder_id, msg);
	pthread_mutex_unlock(&sim->print_mutex);
}
