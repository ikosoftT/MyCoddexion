/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yikoubaz <yikoubaz@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/17 23:19:28 by yikoubaz          #+#    #+#             */
/*   Updated: 2026/08/01 08:34:13 by yikoubaz         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CODEXION_H
# define CODEXION_H

# include <stdio.h>
# include <stdlib.h>
# include <limits.h>
# include <string.h>
# include <unistd.h>
# include <sys/time.h>
# include <time.h>
# include <pthread.h>

# define FIFO 1
# define EDF 0

// === Objects Init STR ====

typedef struct s_data
{
	int		nb_coders;
	long	time_to_burnout;
	long	time_to_compile;
	long	time_to_debug;
	long	time_to_refactor;
	int		nb_compiles_required;
	long	dongle_cooldown;
	int		scheduler;
}	t_data;

typedef struct s_sim
{
	t_data				data;
	long				start_time;
	long				request_counter;
	int					stop;
	pthread_t			monitor;
	pthread_mutex_t		stop_mutex;
	pthread_mutex_t		print_mutex;
	pthread_mutex_t		scheduler_mutex;
	pthread_mutex_t		start_mutex;
	struct s_coder		*coders;
	struct s_dongle		*dongles;
}	t_sim;

typedef struct s_coder
{
	int				id;
	long			last_compile;
	int				compiles_count;
	pthread_mutex_t	state_mutex;
	pthread_t		thread;
	int				left;
	int				right;
	t_sim			*sim;
}	t_coder;

// Request Struct DataModel

typedef struct s_request
{
	t_coder	*coder;
	long	priority;
	long	order;
}	t_request;

typedef struct s_heap
{
	t_request	*array;
	int			size;
	int			capacity;
}	t_heap;

typedef struct s_dongle
{
	pthread_mutex_t	mutex;
	pthread_cond_t	cond;
	long			cooldown_until;
	int				held;
	t_heap			heap;
}	t_dongle;

// === Objects Init END ====

// == Parsing start == 

int			parse_args(int ac, char **av, t_data *data);
int			fill_data(char **av, t_data *data);

// == Parsing end == 

// == threads start == 
int			create_coder_threads(t_sim *sim);
int			create_monitor_thread(t_sim *sim);
int			join_coder_threads(t_sim *sim);
int			join_monitor_thread(t_sim *sim);
// == threads end == 

// == Simulation start == 
int			init_sim(t_sim *sim, t_data *data);
int			init_dongles(t_sim *sim);
int			init_coders(t_sim *sim);

int			simulation_stopped(t_sim *sim);
void		stop_simulation(t_sim *sim);
long		elapsed_time(t_sim *sim);

// == dongles == 
int			take_dongles(t_coder *coder);
void		release_dongles(t_coder *coder);
// == Scheduler Stuffs == 
int			request_dongle(t_coder *coder, t_dongle *dongle);
void		release_dongle(t_coder *coder, t_dongle *dongle);
int			request_dongle_timeout(t_coder *coder, t_dongle *dongle,
				long timeout_ms);

void		log_status(t_coder *coder, char *msg);
// == Simulation end == 

// ===== Heap Data structer str ==== 

// Heap IMPLEMENTATION
// heap-operations
int			heap_push(t_heap *heap, t_request *req);
int			heap_remove(t_heap *heap, t_coder *coder);
t_request	heap_pop(t_heap *heap);
t_request	*heap_peek(t_heap *heap);

// === > Heap initors < === 

int			heap_init(t_heap *heap, int capacity);
void		heap_destroy(t_heap *heap);

// === > Heap Algorithms < ==== 

void		heapify_up(t_heap *heap, int i);
void		heapify_down(t_heap *heap, int i);

// ===== Heap Data structer end ==== 

// Utils functions str
void		smart_sleep(long ms, t_sim *sim);
int			is_space(char c);
long		ft_atol(char *s);
long		get_time(void);
void		get_timeout(struct timespec *ts, long ms);
// Utils functions end

// =========CLEANER==== 
void		clean_all(t_sim *sim);

#endif
