/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coddexion.h                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yikoubaz <yikoubaz@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/28 10:29:04 by yikoubaz          #+#    #+#             */
/*   Updated: 2026/07/01 12:30:00 by yikoubaz         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CODDEXION_H
# define CODDEXION_H

# include <pthread.h>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <unistd.h>
# include <sys/time.h>
# include <limits.h>

# define FIFO 0
# define EDF 1

typedef struct s_request
{
	int				coder_id;
	long			priority;
	int				order;
}	t_request;

typedef struct s_pqueue
{
	t_request		*data;
	int				size;
	int				cap;
}	t_pqueue;

typedef struct s_dongle
{
	pthread_mutex_t	mutex;
	pthread_cond_t	cond;
	int				held;
	long			cooldown_until;
	t_pqueue		queue;
	int				id;
}	t_dongle;

typedef struct s_sim	t_sim;

typedef struct s_coder
{
	int				id;
	pthread_t		thread;
	long			last_compile_start;
	int				compile_count;
	int				finished;
	t_dongle		*left;
	t_dongle		*right;
	t_sim			*sim;
}	t_coder;

struct s_sim
{
	int				nb_coders;
	long			time_to_burnout;
	long			time_to_compile;
	long			time_to_debug;
	long			time_to_refactor;
	int				nb_compiles_required;
	long			dongle_cooldown;
	int				scheduler;
	t_coder			*coders;
	t_dongle		*dongles;
	long			start_time;
	int				stopped;
	int				coders_finished;
	int				global_order;
	pthread_mutex_t	run_mutex;
	pthread_mutex_t	print_mutex;
	pthread_mutex_t	order_mutex;
	pthread_cond_t	run_cond;
	pthread_t		monitor;
	int				monitor_ready;
};

// Parsing
int		parse_args(int ac, char **av, t_sim *sim);
int		validate_args(int ac, char **av);
int		parse_scheduler(char *str);
int		is_valid_int(char **av);

// Init
int		init_sim(t_sim *sim, char **av);

// Simulation
int		start_simulation(t_sim *sim);
void	*coder_routine(void *arg);
void	*monitor_routine(void *arg);
int		sim_stopped(t_sim *sim);

// Dongle
void	take_dongle(t_coder *coder, t_dongle *dongle);
void	release_dongle(t_coder *coder, t_dongle *dongle);
long	get_priority(t_coder *coder);
void	wake_all_dongles(t_sim *sim);

// Logger
void	log_action(t_sim *sim, int coder_id, char *msg);

// Priority queue
void	pq_init(t_pqueue *pq, int cap);
void	pq_push(t_pqueue *pq, t_request req);
t_request	pq_pop(t_pqueue *pq);
t_request	*pq_peek(t_pqueue *pq);
void	pq_clear(t_pqueue *pq);
void	pq_remove_coder(t_pqueue *pq, int coder_id);

// Utils
long	get_time_ms(void);
void	precise_usleep(long us);
int		is_num(char *arg);
long	ft_atol(char *s);

// Cleanup
void	cleanup_sim(t_sim *sim);

#endif
