/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cleanup.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yikoubaz <yikoubaz@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/18 03:24:59 by yikoubaz          #+#    #+#             */
/*   Updated: 2026/08/01 08:24:53 by yikoubaz         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	destroy_coder(t_sim *sim)
{
	int	i;

	if (!sim->coders)
		return ;
	i = 0;
	while (i < sim->data.nb_coders)
		pthread_mutex_destroy(&sim->coders[i++].state_mutex);
	free(sim->coders);
	sim->coders = NULL;
}

static void	destroy_dongles(t_sim *sim)
{
	int	i;

	if (!sim->dongles)
		return ;
	i = 0;
	while (i < sim->data.nb_coders)
	{
		pthread_mutex_destroy(&sim->dongles[i].mutex);
		pthread_cond_destroy(&sim->dongles[i].cond);
		heap_destroy(&sim->dongles[i].heap);
		i++;
	}
	free(sim->dongles);
	sim->dongles = NULL;
}

void	clean_all(t_sim *sim)
{
	if (!sim)
		return ;
	destroy_coder(sim);
	destroy_dongles(sim);
	pthread_mutex_destroy(&sim->print_mutex);
	pthread_mutex_destroy(&sim->stop_mutex);
	pthread_mutex_destroy(&sim->grant_mutex);
	pthread_mutex_destroy(&sim->start_mutex);
	free(sim);
}
