/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yikoubaz <yikoubaz@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/28 10:20:14 by yikoubaz          #+#    #+#             */
/*   Updated: 2026/07/01 17:21:41 by yikoubaz         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/coddexion.h"

int	main(int ac, char **av)
{
	t_sim	*sim;

	sim = malloc(sizeof(t_sim));
	if (!sim)
		return (1);
	memset(sim, 0, sizeof(t_sim));
	if (!parse_args(ac, av, sim))
	{
		printf("Error\n");
		cleanup_sim(sim);
		return (1);
	}
	start_simulation(sim);
	cleanup_sim(sim);
	return (0);
}
