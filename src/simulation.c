/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   simulation.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yikoubaz <yikoubaz@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/28 10:20:53 by yikoubaz          #+#    #+#             */
/*   Updated: 2026/07/01 17:21:55 by yikoubaz         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/coddexion.h"


int	sim_stopped(t_sim *sim)
{
	int	s;

	pthread_mutex_lock(&sim->run_mutex);
	s = sim->stopped;
	pthread_mutex_unlock(&sim->run_mutex);
	return (s);
}

long	get_priority(t_coder *coder)
{
	if (coder->sim->scheduler == FIFO)
		return (get_time_ms());
	return (coder->last_compile_start + coder->sim->time_to_burnout);
}

static int	get_order(t_sim *sim)
{
	int	order;

	pthread_mutex_lock(&sim->order_mutex);
	order = sim->global_order++;
	pthread_mutex_unlock(&sim->order_mutex);
	return (order);
}

static void	wait_on_dongle(t_dongle *dongle, t_request *top, t_coder *coder)
{
	struct timeval	tv;
	struct timespec	ts;
	long			wait;

	if (top && top->coder_id == coder->id && !dongle->held
		&& get_time_ms() < dongle->cooldown_until)
	{
		wait = dongle->cooldown_until - get_time_ms();
		gettimeofday(&tv, NULL);
		ts.tv_sec = tv.tv_sec + wait / 1000;
		ts.tv_nsec = tv.tv_usec * 1000 + (wait % 1000) * 1000000;
		if (ts.tv_nsec >= 1000000000)
		{
			ts.tv_sec++;
			ts.tv_nsec -= 1000000000;
		}
		pthread_cond_timedwait(&dongle->cond, &dongle->mutex, &ts);
	}
	else
		pthread_cond_wait(&dongle->cond, &dongle->mutex);
}

void	take_dongle(t_coder *coder, t_dongle *dongle)
{
	t_request	req;
	t_sim		*sim;
	t_request	*top;

	sim = coder->sim;
	pthread_mutex_lock(&dongle->mutex);
	req.coder_id = coder->id;
	req.priority = get_priority(coder);
	req.order = get_order(sim);
	pq_push(&dongle->queue, req);
	while (1)
	{
		top = pq_peek(&dongle->queue);
		if (top && top->coder_id == coder->id && !dongle->held
			&& get_time_ms() >= dongle->cooldown_until)
		{
			pq_pop(&dongle->queue);
			dongle->held = 1;
			break ;
		}
		pthread_mutex_lock(&sim->run_mutex);
		if (sim->stopped)
		{
			pthread_mutex_unlock(&sim->run_mutex);
			pq_remove_coder(&dongle->queue, coder->id);
			pthread_mutex_unlock(&dongle->mutex);
			return ;
		}
		pthread_mutex_unlock(&sim->run_mutex);
		wait_on_dongle(dongle, top, coder);
	}
	pthread_mutex_unlock(&dongle->mutex);
}

void	release_dongle(t_coder *coder, t_dongle *dongle)
{
	pthread_mutex_lock(&dongle->mutex);
	dongle->held = 0;
	dongle->cooldown_until = get_time_ms() + coder->sim->dongle_cooldown;
	pthread_cond_broadcast(&dongle->cond);
	pthread_mutex_unlock(&dongle->mutex);
}

void	wake_all_dongles(t_sim *sim)
{
	int	i;

	i = 0;
	while (i < sim->nb_coders)
	{
		pthread_mutex_lock(&sim->dongles[i].mutex);
		pthread_cond_broadcast(&sim->dongles[i].cond);
		pthread_mutex_unlock(&sim->dongles[i].mutex);
		i++;
	}
}

static void	check_all_finished(t_sim *sim)
{
	pthread_mutex_lock(&sim->run_mutex);
	if (sim->coders_finished >= sim->nb_coders)
	{
		sim->stopped = 1;
		pthread_cond_broadcast(&sim->run_cond);
	}
	pthread_mutex_unlock(&sim->run_mutex);
}

static void	perform_compile(t_coder *coder, t_sim *sim)
{
	t_dongle	*first;
	t_dongle	*second;

	first = coder->left;
	second = coder->right;
	if (first->id > second->id)
	{
		first = coder->right;
		second = coder->left;
	}
	if (first == second)
	{
		take_dongle(coder, first);
		if (sim_stopped(sim))
			return ;
		log_action(sim, coder->id, "has taken a dongle");
		log_action(sim, coder->id, "has taken a dongle");
	}
	else
	{
		take_dongle(coder, first);
		if (sim_stopped(sim))
			return ;
		log_action(sim, coder->id, "has taken a dongle");
		take_dongle(coder, second);
		if (sim_stopped(sim))
			return ;
		log_action(sim, coder->id, "has taken a dongle");
	}
	pthread_mutex_lock(&sim->run_mutex);
	coder->last_compile_start = get_time_ms();
	pthread_mutex_unlock(&sim->run_mutex);
	log_action(sim, coder->id, "is compiling");
	precise_usleep(sim->time_to_compile * 1000);
	coder->compile_count++;
	if (coder->compile_count >= sim->nb_compiles_required && !coder->finished)
	{
		coder->finished = 1;
		pthread_mutex_lock(&sim->run_mutex);
		sim->coders_finished++;
		pthread_mutex_unlock(&sim->run_mutex);
		check_all_finished(sim);
	}
	release_dongle(coder, coder->left);
	if (first != second)
		release_dongle(coder, coder->right);
}

void	*coder_routine(void *arg)
{
	t_coder	*coder;
	t_sim	*sim;

	coder = (t_coder *)arg;
	sim = coder->sim;
	while (!sim_stopped(sim))
	{
		if (coder->compile_count >= sim->nb_compiles_required)
		{
			usleep(1000);
			continue ;
		}
		perform_compile(coder, sim);
		if (sim_stopped(sim))
			break ;
		log_action(sim, coder->id, "is debugging");
		precise_usleep(sim->time_to_debug * 1000);
		if (sim_stopped(sim))
			break ;
		log_action(sim, coder->id, "is refactoring");
		precise_usleep(sim->time_to_refactor * 1000);
	}
	return (NULL);
}

int	start_simulation(t_sim *sim)
{
	int	i;

	sim->start_time = get_time_ms();
	if (pthread_create(&sim->monitor, NULL, monitor_routine, sim) != 0)
		return (0);
	while (!sim->monitor_ready)
		usleep(100);
	i = 0;
	while (i < sim->nb_coders)
	{
		if (pthread_create(&sim->coders[i].thread, NULL,
				coder_routine, &sim->coders[i]) != 0)
		{
			pthread_mutex_lock(&sim->run_mutex);
			sim->stopped = 1;
			pthread_mutex_unlock(&sim->run_mutex);
			wake_all_dongles(sim);
			break ;
		}
		i++;
	}
	i = 0;
	while (i < sim->nb_coders)
	{
		pthread_join(sim->coders[i].thread, NULL);
		i++;
	}
	pthread_join(sim->monitor, NULL);
	return (1);
}
