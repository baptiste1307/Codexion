/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bpasquer <bpasquer@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/25 15:30:00 by Baptiste          #+#    #+#             */
/*   Updated: 2026/03/25 17:48:43 by bpasquer         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CODEXION_H
# define CODEXION_H

# include <stdio.h>
# include <unistd.h>
# include <stdlib.h>
# include <pthread.h>
# include <sys/time.h>

typedef struct s_coder	t_coder;

typedef struct s_dongle
{
	pthread_mutex_t	mutex;
	long			available_at;
}	t_dongle;

typedef struct s_heap
{
	t_coder			**arr;
	int				size;
	int				capacity;
	pthread_mutex_t	mutex;
}	t_heap;

typedef struct s_data
{
	int				number_of_coders;
	int				time_to_burnout;
	int				time_to_compile;
	int				time_to_debug;
	int				time_to_refactor;
	int				number_of_compiles_required;
	int				dongle_cooldown;
	int				stop;
	long			start_time;
	char			*scheduler;
	t_coder			*coders;
	t_heap			*heap;
	pthread_mutex_t	log_mutex;
	pthread_mutex_t	stop_mutex;
	t_dongle		*dongles;
	long			next_request_order;
}	t_data;

typedef struct s_coder
{
	int				id;
	int				compile_count;
	long			last_compile_time;
	pthread_t		thread;
	pthread_mutex_t	last_compile_mutex;
	t_data			*data;
	t_dongle		*left_dgl;
	t_dongle		*right_dgl;
	long			request_order;
}	t_coder;

/**
 * @brief Main routine executed by each coder thread.
 * @param arg Generic pointer cast to a @c t_coder instance.
 * @return Always returns @c NULL when the thread terminates.
 */
void		*routine(void *arg);

/**
 * @brief Parse arguments and initialize the shared runtime data.
 * @param ac Number of command-line arguments.
 * @param av Command-line argument vector.
 * @param data Stack-allocated shared structure filled during init.
 * @return Pointer to initialized shared data, or @c NULL on failure.
 */
t_data		*init_data(int ac, char **av, t_data *data);

/**
 * @brief Parse the seven numeric arguments of the program.
 * @param ac Number of command-line arguments.
 * @param av Command-line argument vector.
 * @param data Shared structure receiving parsed values.
 * @return @p data on success, or @c NULL if one argument is invalid.
 */
t_data		*set_data(int ac, char **av, t_data *data);

/**
 * @brief Allocate and initialize every coder structure.
 * @param data_ptr Shared simulation data used to configure coders.
 * @return Array of initialized coders, or @c NULL on allocation failure.
 */
t_coder		*init_coders(t_data *data_ptr);

/**
 * @brief Spawn one pthread per coder.
 * @param coders Array of initialized coders.
 * @param data_ptr Shared simulation data used for sizing.
 * @return 0 on success, 1 if thread creation fails.
 */
int			create_threads(t_coder *coders, t_data *data_ptr);

/**
 * @brief Join all coder threads and destroy their private mutexes.
 * @param coders Array of coders previously started.
 * @param data_ptr Shared simulation data used for sizing.
 */
void		join_threads(t_coder *coders, t_data *data_ptr);

/**
 * @brief Allocate dongles and initialize the scheduling heap.
 * @param data_ptr Shared simulation data that will own these resources.
 * @return 0 on success, 1 on initialization failure.
 */
int			set_dongles(t_data *data_ptr);

/**
 * @brief Initialize a range of dongles with one mutex each.
 * @param dongles Array of dongles to initialize.
 * @param count Number of dongles contained in the array.
 * @return 1 on success, 0 if one mutex initialization fails.
 */
int			init_dongle_mutexes(t_dongle *dongles, int count);

/**
 * @brief Acquire the dongles needed by a coder before compiling.
 * @param coder Coder requesting the compile resources.
 * @param first First dongle considered for locking.
 * @param second Second dongle considered for locking.
 * @return 0 when both resources are acquired, 1 otherwise.
 */
int			lock_dongles(t_coder *coder, t_dongle *first, t_dongle *second);

/**
 * @brief Release the dongles held by a coder and apply cooldown.
 * @param coder Coder releasing its resources.
 * @param first First dongle to release.
 * @param second Second dongle to release.
 * @return 0 on success, 1 if an unlock fails.
 */
int			unlock_dongles(t_coder *coder, t_dongle *first, t_dongle *second);

/**
 * @brief Destroy all dongle-related resources.
 * @param data_ptr Shared simulation data owning dongles and heap.
 * @return 0 on success, 1 if the input is invalid.
 */
int			destroy_dongles(t_data *data_ptr);

/**
 * @brief Check whether two dongles are both unlocked and out of cooldown.
 * @param first First dongle to inspect.
 * @param second Second dongle to inspect.
 * @return 1 if the pair can be used immediately, 0 otherwise.
 */
int			pair_is_available(t_dongle *first, t_dongle *second);

/**
 * @brief Try to lock a full dongle pair for a compile.
 * @param first First dongle to lock.
 * @param second Second dongle to lock.
 * @return 1 if the pair was locked successfully, 0 otherwise.
 */
int			try_take_pair(t_dongle *first, t_dongle *second);

/**
 * @brief Handle the special case where only one dongle exists.
 * @param coder Single coder of the simulation.
 * @return 0 when the dongle is acquired, 1 if the wait aborts.
 */
int			wait_single_dongle(t_coder *coder);

/**
 * @brief Wait until the scheduler authorizes a coder to take both dongles.
 * @param coder Coder requesting a compile slot.
 * @param first First dongle to lock once selected.
 * @param second Second dongle to lock once selected.
 * @return 0 on success, 1 if the wait stops first.
 */
int			wait_turn_and_take(t_coder *coder, t_dongle *first,
				t_dongle *second);

/**
 * @brief Store the timestamp corresponding to the start of a compile.
 * @param coder Coder whose compile start time must be updated.
 */
void		set_compile_start(t_coder *coder);

/**
 * @brief Print one serialized state transition for a coder.
 * @param coder Coder whose state is being reported.
 * @param message Human-readable state string to print.
 */
void		print_state(t_coder *coder, char *message);

/**
 * @brief Return the current time in milliseconds.
 * @return Absolute time expressed in milliseconds.
 */
long		get_time_ms(void);

/**
 * @brief Monitor thread routine responsible for burnout detection.
 * @param arg Generic pointer cast to the coder array.
 * @return Always returns @c NULL when monitoring ends.
 */
void		*monitor_routine(void *arg);

/**
 * @brief Read the global stop flag of the simulation.
 * @param coder Coder used to access shared stop state.
 * @return 1 if the simulation is stopping, 0 otherwise.
 */
int			has_burned_out(t_coder *coder);

/**
 * @brief Execute compile, debug and refactor phases after locking resources.
 * @param coder Coder currently holding the required dongles.
 * @return 0 if the caller should stop after this cycle, 1 otherwise.
 */
int			routine_2(t_coder *coder);

/**
 * @brief Validate and store the scheduler argument.
 * @param ac Number of command-line arguments.
 * @param av Command-line argument vector.
 * @param data Shared structure receiving the scheduler pointer.
 * @return 1 when the scheduler is valid, 0 otherwise.
 */
int			set_data_scheduler(int ac, char **av, t_data *data);

/**
 * @brief Compare two C strings like @c strcmp.
 * @param s1 First string to compare.
 * @param s2 Second string to compare.
 * @return Negative, zero or positive depending on lexicographic order.
 */
int			ft_strcmp(const char *s1, const char *s2);

/**
 * @brief Check whether a coder reached the compile target.
 * @param coder Coder whose compile counter must be inspected.
 * @return 1 if the required count is reached, 0 otherwise.
 */
int			has_reach_max_compiles(t_coder *coder);

/**
 * @brief Swap two entries inside the scheduling heap.
 * @param a First heap entry.
 * @param b Second heap entry.
 */
void		heap_swap(t_coder **a, t_coder **b);

/**
 * @brief Compute the EDF deadline of one coder.
 * @param c Coder whose deadline must be evaluated.
 * @return Absolute burnout deadline timestamp.
 */
long		get_deadline(t_coder *c);

/**
 * @brief Return the scheduler key associated with a coder.
 * @param heap Heap in which the coder is ranked.
 * @param c Coder whose priority value is requested.
 * @return FIFO request order or EDF deadline depending on scheduler mode.
 */
long		get_priority_value(t_heap *heap, t_coder *c);

/**
 * @brief Rebalance the heap upward after an insertion.
 * @param heap Heap to rebalance.
 * @param i Index of the node to bubble up.
 */
void		go_up_in_heap(t_heap *heap, int i);

/**
 * @brief Rebalance the heap downward after a removal.
 * @param heap Heap to rebalance.
 * @param i Index of the node to push downward.
 */
void		go_down_in_heap(t_heap *heap, int i);

/**
 * @brief Push a coder into the heap with internal locking.
 * @param heap Heap receiving the coder.
 * @param c Coder to insert.
 */
void		heap_push(t_heap *heap, t_coder *c);

/**
 * @brief Return the current top of the heap.
 * @param heap Heap to inspect.
 * @return Highest-priority coder, or @c NULL if the heap is empty.
 */
t_coder		*heap_peek(t_heap *heap);

/**
 * @brief Remove the heap root with internal locking.
 * @param heap Heap whose top entry must be removed.
 */
void		heap_pop(t_heap *heap);

/**
 * @brief Push a coder into the heap while the heap is already locked.
 * @param heap Locked heap receiving the coder.
 * @param c Coder to insert.
 */
void		heap_push_locked(t_heap *heap, t_coder *c);

/**
 * @brief Remove the heap root while the heap is already locked.
 * @param heap Locked heap whose root must be removed.
 */
void		heap_pop_locked(t_heap *heap);

/**
 * @brief Remove an arbitrary entry from a locked heap.
 * @param heap Locked heap from which an element is removed.
 * @param index Index of the element to erase.
 */
void		heap_remove_at_locked(t_heap *heap, int index);

#endif
