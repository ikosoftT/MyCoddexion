/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   logger.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yikoubaz <yikoubaz@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/18 03:53:26 by yikoubaz          #+#    #+#             */
/*   Updated: 2026/08/03 18:22:31 by yikoubaz         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void log_status(t_coder *coder, char *msg)
{
	pthread_mutex_lock(&coder->sim->print_mutex);
	
	printf("%ld %d %s\n", elapsed_time(coder->sim), coder->id, msg);
	pthread_mutex_unlock(&coder->sim->print_mutex);
}
