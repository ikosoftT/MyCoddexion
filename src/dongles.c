/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongles.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yikoubaz <yikoubaz@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/18 00:44:07 by yikoubaz          #+#    #+#             */
/*   Updated: 2026/07/28 10:47:36 by yikoubaz         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	acquire_dongles(t_coder *coder, int first, int second)
{
	while (!simulation_stopped(coder->sim))
	{
		if (!request_dongle(coder, &coder->sim->dongles[first]))
			return (0);
		if (request_dongle_timeout(coder, &coder->sim->dongles[second], 2))
			return (1);
		release_dongle(coder, &coder->sim->dongles[first]);
		smart_sleep(2, coder->sim);
	}
	return (0);
}

int	take_dongles(t_coder *coder)
{
	int	left;
	int	right;
	int	first;
	int	second;

	if (coder->sim->data.nb_coders == 1)
		return (request_dongle(coder, &coder->sim->dongles[0]));
	left = coder->left;
	right = coder->right;
	if (left < right)
	{
		first = left;
		second = right;
	}
	else
	{
		first = right;
		second = left;
	}
	return (acquire_dongles(coder, first, second));
}

void	release_dongles(t_coder *coder)
{
	release_dongle(coder, &coder->sim->dongles[coder->left]);
	release_dongle(coder, &coder->sim->dongles[coder->right]);
}
