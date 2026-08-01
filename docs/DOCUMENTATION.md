# Codexion — Full Technical Documentation & 42 Evaluation Guide

This document explains **everything** you need to defend your project: the overall
flow, the logic of every single function, the concurrency design decisions, and
the answers to the questions an evaluator is most likely to ask.

---

## 1. Project Overview

Codexion is a **multi-threaded concurrency simulation** (the "philosophers" style
project but themed around coders competing for USB dongles).

**The world:**
- `N` coders and `N` dongles in a ring topology.
- Each coder sits between two dongles: `left = id`, `right = (id+1) % N`.
- Each coder **needs two dongles at the same time** to compile.
- Compiling is followed by debugging and refactoring.
- Every coder must compile a required number of times.
- If a coder does not compile for longer than `time_to_burnout`, it burns out.

**The simulation ends when either:**
1. Every coder reached `number_of_compiles_required` compiles (success), **or**
2. Any coder burns out (failure).

**The challenges solved:**
- Deadlock (circular wait on dongles).
- Starvation (fair arbitration with a priority queue).
- Race conditions (mutexes everywhere).
- Burnout detection (a dedicated monitor thread).

---

## 2. Running the Program

```sh
make                       # build
./codexion <n_coders> <time_to_burnout> <time_to_compile> \
           <time_to_debug> <time_to_refactor> <n_compiles_required> \
           <dongle_cooldown> <fifo|edf>
```

Example from the README:

```sh
./codexion 4 2000 100 100 100 3 10 fifo
```

| Argument | Meaning |
|---|---|
| `number_of_coders` | Number of coders **and** dongles |
| `time_to_burnout` | Max ms a coder may go without compiling before burning out |
| `time_to_compile` | How long the compile phase lasts (ms) |
| `time_to_debug` | How long the debug phase lasts (ms) |
| `time_to_refactor` | How long the refactor phase lasts (ms) |
| `number_of_compiles_required` | Compiles needed from each coder to win |
| `dongle_cooldown` | Cooldown after a dongle is released (ms) |
| `scheduler` | `fifo` or `edf` |

---

## 3. Architecture Overview

```
                +-----------------+
                |      main()     |
                +--------+--------+
                         |
              +----------+----------+
              |  ft_setup (parse)   |
              |  ft_initor (init)   |
              |  run_simulation     |
              +----------+----------+
                         |
          +--------------+----------------------+
          |                                     |
   +------v--------+                     +------v--------+
   |   N coder     |                     |   monitor     |
   |   threads     |                     |   thread      |
   +------+--------+                     +------+--------+
          |                                     |
          |  take_dongles -> request_dongle    |  find_burned_coder
          |       +                           |  all_coders_done
          |       v                           |
          |   per-dongle HEAP (priority)      |      | stop flag
          |  +-----------------------------+  |      v
          +->| dongle mutex + cond + heap  |--+----> stop_simulation
             +-----------------------------+
```

**Threads:**
- `main` — orchestrates everything, then joins.
- `N` **coder threads** (`coder_routine`) — the workers.
- **1 monitor thread** (`monitor_routine`) — checks burnout / completion.

---

## 4. Data Structures

### `t_data` (parsed arguments)
```c
typedef struct s_data
{
	int		nb_coders;              // number of coders & dongles
	long	time_to_burnout;        // deadline slack
	long	time_to_compile;        // ms of compile
	long	time_to_debug;          // ms of debug
	long	time_to_refactor;       // ms of refactor
	int		nb_compiles_required;   // win condition
	long	dongle_cooldown;        // ms a dongle is locked after release
	int		scheduler;              // FIFO (1) or EDF (0)
}	t_data;
```

### `t_sim` (the shared simulation context)
```c
typedef struct s_sim
{
	t_data				data;           // parsed config (copied)
	long				start_time;     // epoch ms at init (for elapsed time)
	long				request_counter; // global, monotonic, for FIFO ordering
	int					stop;           // 1 == simulation over
	pthread_t			monitor;        // monitor thread id
	pthread_mutex_t		stop_mutex;     // protects `stop`
	pthread_mutex_t		print_mutex;    // serializes log lines
	pthread_mutex_t		scheduler_mutex;// serializes request_counter
	pthread_barrier_t	start_barrier;  // starts all threads together
	struct s_coder		*coders;        // array of N coders
	struct s_dongle		*dongles;       // array of N dongles
}	t_sim;
```

### `t_coder` (one worker)
```c
typedef struct s_coder
{
	int				id;             // 1..N
	long			last_compile;   // epoch ms when compile started (deadline base)
	int				compiles_count; // number of compiles finished
	pthread_mutex_t	state_mutex;    // protects last_compile & compiles_count
	pthread_t		thread;         // the coder's thread id
	int				left;           // left dongle index (id)
	int				right;          // right dongle index ((id+1) % N)
	t_sim			*sim;           // back-pointer to the sim
}	t_coder;
```

### `t_request` (a pending claim on one dongle)
```c
typedef struct s_request
{
	t_coder	*coder;     // who wants the dongle
	long	priority;   // FIFO: arrival order ; EDF: absolute deadline
	long	order;      // global counter, tie-breaker
}	t_request;
```

### `t_heap` (min-heap of requests, per dongle)
```c
typedef struct s_heap
{
	t_request	*array;   // dynamic array, root = next to serve
	int			size;
	int			capacity; // == nb_coders (a coder can wait at most once per dongle)
}	t_heap;
```

### `t_dongle` (the contested resource)
```c
typedef struct s_dongle
{
	pthread_mutex_t	mutex;          // guards everything about this dongle
	pthread_cond_t	cond;           // waiters block here
	long			cooldown_until; // epoch ms before the dongle can be used again
	int				held;           // 1 == currently held by some coder
	t_heap			heap;           // priority queue of requests
}	t_dongle;
```

---

## 5. Global Execution Flow

```
main
 ├─ malloc t_sim, t_data
 ├─ ft_setup()          → parse & validate the 8 arguments
 ├─ ft_initor()
 │    ├─ init_sim()     → copy data, init sim mutexes + barrier + start_time
 │    ├─ init_dongles() → malloc N dongles, init mutex/cond/heap each
 │    └─ init_coders()  → malloc N coders, set ids, left/right, state_mutex
 ├─ run_simulation()
 │    ├─ create_coder_threads()   → spawn N coder_routine threads
 │    ├─ barrier_wait            → all threads start at the same moment
 │    ├─ create_monitor_thread() → spawn monitor_routine
 │    ├─ join_monitor_thread()   → main waits until sim ends (win or burnout)
 │    └─ join_coder_threads()    → then reaps every coder thread
 ├─ free(data)
 └─ clean_all(sim)     → destroy every mutex/cond/heap, free memory
```

Everything is freed at the end — no leaks. `main` returns `0` on a clean run.

---

## 6. File-by-File Function Reference

### `main.c`

**`static int ft_err(char *err)`**
Writes `err` character by character to **stderr** and returns `2`. Used for every
error path so the exit code signals failure.

**`static int ft_setup(int ac, char **av, t_data *data, t_sim *sim)`**
The argument-parsing stage.
- If `data` allocation failed → free `sim`, print `"Fail at data alloc\n"`.
- If `parse_args` fails → free both, print `"Parsing error\n"`.
- Otherwise returns `1` (continue).
Returns `2` on any error — `main` returns that code immediately.

**`static int ft_initor(t_data *data, t_sim *sim)`**
Calls the three initializers in order: `init_sim` → `init_dongles` → `init_coders`.
Returns `0` if any step fails.

**`static int run_simulation(t_sim *sim)`**
The heart of the orchestration:
1. `create_coder_threads` — if it fails, report and return.
2. `pthread_barrier_wait(&sim->start_barrier)` — holds `main` here until **all N
   coder threads also reach the barrier**, so every coder starts "at the same time".
   This is what makes the `last_compile = start_time` trick fair at boot.
3. `create_monitor_thread` — on failure, set `stop`, join coders (so they don't
   spin forever), report, return.
4. `join_monitor_thread` — **blocking wait for the simulation to finish**. The
   monitor returns only after `stop` was set (win or burnout).
5. `join_coder_threads` — reap all workers, which by then have exited their loops
   because `simulation_stopped` is true.
Returns `0` on success.

**`int main(int ac, char **av)`**
- Allocates `sim`, then `data`.
- `ft_setup` → if not `1`, return that code (2) after freeing appropriately.
- `ft_initor` → on failure, free data, `clean_all(sim)`, print `"Fail at initor func\n"`.
- `run_simulation` → store result in `code`.
- `free(data)`, `clean_all(sim)` → **always** run the full cleanup.
- `return (code)`.

### `parser.c`

**`static int is_num(char *s)`**
Returns `1` if the string is only digits (after optional leading whitespace).
Rejects negatives, signs, and letters.

**`static int is_valid(char **av)`**
Checks `av[1]..av[6]` are all `> 0` (strictly positive — no zero coders, zero
times, etc.).

**`static int validate_args(int ac, char **av)`**
- Requires exactly `ac == 9` (program + 8 arguments).
- `av[1]..av[7]` must be numeric.
- `av[1]..av[8]` must fit in `INT_MAX`.
- `is_valid` (all but the scheduler `> 0`) and the scheduler argument must be `>= 0`.
- `av[8]` must be `fifo` or `edf` (checked later in `fill_data`).

**`int fill_data(char **av, t_data *data)`**
Assigns each argument into `data` via `ft_atol`. For the scheduler:
- `"fifo"` → `data->scheduler = FIFO` (1)
- `"edf"`  → `data->scheduler = EDF` (0)
- anything else → returns `0` (invalid scheduler → parsing error).

**`int parse_args(int ac, char **av, t_data *data)`**
Runs `validate_args` then `fill_data`. Returns `1` only if both succeed.

### `utils.c`

**`int is_space(char c)`**
Returns `1` for ASCII whitespace: `\t \n \v \f \r` and space.

**`long ft_atol(char *s)`**
Hand-rolled `atoi` (positive only): skips leading whitespace, then accumulates
`r = r * 10 + digit`. Returns a `long`. (The parser already guaranteed digits.)

**`long get_time(void)`**
Returns the current **epoch time in milliseconds** via `gettimeofday`.
`tv_sec * 1000 + tv_usec / 1000`. All times in the program are ms.

**`void get_timeout(struct timespec *ts, long ms)`**
Fills a `timespec` that is `ms` in the **future** (absolute time, required by
`pthread_cond_timedwait`). Handles the nanosecond overflow across the second
boundary.

**`void smart_sleep(long ms, t_sim *sim)`**
A sleep that can be interrupted by the simulation stopping. Loops: while the
simulation is **not** stopped AND less than `ms` has elapsed, `usleep(500)`.
This guarantees a coder blocked in a long compile/debug phase still notices
`stop` within ~0.5 ms and exits cleanly. It also prevents a "usleep overflow"
with huge arguments.

### `init.c`

**`int init_sim(t_sim *sim, t_data *data)`**
- Copies `*data` into `sim->data` (so the config lives inside the sim).
- Inits `print_mutex` and `stop_mutex`.
- Records `start_time = get_time()`.
- `stop = 0`, `coders = NULL`, `dongles = NULL`, `request_counter = 0`.
- Inits `scheduler_mutex`.
- Inits the **start barrier** with a count of `nb_coders + 1` (N coders + main).
Returns `0` if any init fails.

**`static int init_dongle_mutex(t_sim *sim, int i)`**
Initializes dongle `i`:
- `pthread_mutex_init(&dongles[i].mutex)` — on failure, destroys all previous
  dongle mutexes, frees the array, returns `0`.
- `held = 0`, `cooldown_until = 0`.
- `pthread_cond_init(&dongles[i].cond)` — on failure, destroys all previous conds.
- `heap_init(&dongles[i].heap, nb_coders)` — capacity is `N` because no coder can
  be queued twice on the same dongle at once.

**`int init_dongles(t_sim *sim)`**
Allocates the `t_dongle` array (N elements). On alloc failure returns `0`.
Loops calling `init_dongle_mutex` for each index; returns `0` on first failure,
`1` if all succeeded. (Note: partial failures may leave earlier dongles
allocated — this mirrors the project's cleanup contract in `clean_all`.)

**`int init_coders(t_sim *sim)`**
Allocates the `t_coder` array (N elements). For each coder:
- `sim = sim` (back-pointer).
- `id = i + 1` (1-based).
- `compiles_count = 0`.
- `last_compile = start_time` → **the deadline clock starts at boot**, so the
  monitor can start checking immediately.
- `left = i`, `right = (i + 1) % N` → the ring adjacency.
- Inits `state_mutex`.
Returns `1`.

### `thread.c`

**`static int one_coder(t_coder *coder)`**
Special case for `N == 1`:
- If `nb_coders != 1` → returns `0` (not handled here).
- Waits on the start barrier (so it still synchronizes with main).
- Tries `request_dongle(coder, &dongles[0])`; if it fails (sim stopped) → return `1`.
- Once it has the single dongle, it just waits while the simulation runs
  (`usleep(500)` loop), then `release_dongle` and returns `1`.
Why? With one coder there are no neighbors to contend with, and the acquire-two-
dongles logic (`left == right == 0`) would try to take the *same* dongle twice —
so this path sidesteps that entirely.

**`static int do_work(t_coder *coder)`**
One full work cycle after a coder holds both dongles:
1. Log `"is compiling"`.
2. Lock `state_mutex` → `last_compile = get_time()` → unlock. **This refresh is
   what saves the coder from burnout.**
3. `smart_sleep(time_to_compile)`.
4. Lock → `compiles_count++` → unlock. (This is the only place the count grows.)
5. `release_dongles(coder)` → the two dongles are freed BEFORE the next phase.
6. If simulation stopped → return `1` (exit).
7. Log `"is debugging"`, `smart_sleep(time_to_debug)`; if stopped → return `1`.
8. Log `"is refactoring"`, `smart_sleep(time_to_refactor)`.
9. Return whether the simulation has stopped.
Note: after compiling, the coder no longer needs the dongles — the dongles are
released before debug/refactor, allowing other coders to use them.

**`void *coder_routine(void *arg)`**
The entry point of every coder thread:
1. Cast `arg` to `t_coder *`.
2. If `one_coder` handled it → return.
3. Main loop `while (!simulation_stopped(sim))`:
   - `take_dongles(coder)` → try to grab both dongles (blocking, ordered). If it
     returns `0`, the sim stopped while waiting → exit.
   - If the sim stopped right after acquiring (e.g., monitor fired mid-acquire) →
     release the dongles and exit.
   - `do_work(coder)` → compile/debug/refactor. Returns `1` if we must stop.
4. Return `NULL`.

**`int create_coder_threads(t_sim *sim)`**
Creates N threads running `coder_routine` with `&sim->coders[i]` as argument.
If `pthread_create` fails at index `i`, it joins all already-created threads and
returns `0` (so no orphan threads leak into `run_simulation`).

**`int join_coder_threads(t_sim *sim)`**
Joins all N threads; returns `0` if any join fails.

### `dongles.c`

**`static int acquire_dongles(t_coder *coder, int first, int second)`**
The blocking acquire loop (used when N > 1):
```
while (!simulation_stopped(sim)):
    if !request_dongle(first)   → return 0      (sim stopped)
    if request_dongle_timeout(second, 2) → return 1   (got both)
    release_dongle(first)                       (give up first)
    smart_sleep(2)                              (back off, avoid thundering herd)
return 0
```
The key anti-deadlock trick is in `take_dongles`: `first`/`second` are passed in
**ascending index order** so no two coders ever wait on each other in a cycle.

**`int take_dongles(t_coder *coder)`**
- If `N == 1` → just `request_dongle` on dongle 0.
- Computes `first = min(left, right)`, `second = max(left, right)`.
- Calls `acquire_dongles(coder, first, second)`.
This enforces the **resource hierarchy** (always acquire the smaller index first).

**`void release_dongles(t_coder *coder)`**
Releases both dongles (left then right) via `release_dongle`. Order does not
matter for deadlock here because releasing can never deadlock.

### `scheduler.c`

**`static t_request build_request(t_coder *coder)`**
Creates a request for one dongle:
1. `sim = coder->sim`.
2. Lock `scheduler_mutex` → `req.order = request_counter++` → unlock. The global
   counter gives every request a unique, increasing arrival number across all
   dongles.
3. **FIFO**: `req.priority = req.order` (earlier arrival = smaller priority).
4. **EDF**: lock the coder's `state_mutex`, `req.priority = last_compile +
   time_to_burnout` (absolute deadline), then `req.order = ((long)coder->id << 48)
   | req.order`. The id shifted into the high bits guarantees each coder has a
   unique order, and ties in priority are broken deterministically by id then
   arrival.
5. `req.coder = coder` and return.

**`static int wait_for_acquire(t_coder *coder, t_dongle *dongle, long end_time, long poll_ms)`**
Called with the dongle mutex **already held**. Loop:
- Peek `heap_peek(&dongle->heap)`.
- If `top->coder == coder && !dongle->held && get_time() >= cooldown_until` →
  **this coder is the highest priority waiter and the dongle is free and
  cooled-down** → `heap_pop`, set `held = 1`, log `"has taken a dongle"`, return `1`.
- If `end_time >= 0` and `now >= end_time` → timeout, break.
- `get_timeout(&timeout, poll_ms)` then `pthread_cond_timedwait` — this atomically
  unlocks the dongle mutex and sleeps up to `poll_ms`, waking early on broadcast.
- Loop until `simulation_stopped`.
Returns `0` (did not acquire).

**`int request_dongle(t_coder *coder, t_dongle *dongle)`**
Blocking request (no timeout):
1. `req = build_request(coder)`.
2. Lock the dongle mutex.
3. `heap_push(&dongle->heap, &req)` — if the heap is full (`size == capacity`),
   unlock and return `0`. (Only possible if a bug made two identical coder
   requests for the same dongle — it never happens in practice.)
4. `wait_for_acquire(coder, dongle, -1, 10)` — the `-1` means "no deadline",
   poll every 10 ms so it can still notice `simulation_stopped` promptly.
5. If acquired → unlock, return `1`.
6. Otherwise (sim stopped) → `heap_remove` this coder's request, broadcast the
   cond (wake everyone so they re-check), unlock, return `0`.

**`int request_dongle_timeout(t_coder *coder, t_dongle *dongle, long timeout_ms)`**
Same as `request_dongle` but with a deadline: `wait_for_acquire(..., get_time() +
timeout_ms, 1)` — polls every 1 ms and bails out after `timeout_ms`. Used by
`acquire_dongles` so a coder that can't get the *second* dongle gives up and
releases the first, preventing deadlock.

**`void release_dongle(t_coder *coder, t_dongle *dongle)`**
- Lock dongle mutex.
- `held = 0`, `cooldown_until = get_time() + dongle_cooldown`.
- `pthread_cond_broadcast(&dongle->cond)` → all waiters re-evaluate priority.
- Unlock.
The cooldown makes the dongle unusable for a while even after release, so a
released dongle can't be instantly grabbed by the same coder again.

### `heap.c`

**`int heap_init(t_heap *heap, int capacity)`**
Mallocates `capacity * sizeof(t_request)`, sets `size = 0`, `capacity = capacity`.
Returns `0` on alloc failure.

**`int heap_push(t_heap *heap, t_request *req)`**
- If full → return `0`.
- Copies `*req` into `array[size]` (a copy, so the caller's stack `req` is fine),
  increments size, then `heapify_up` at the new last position. Returns `1`.

**`t_request *heap_peek(t_heap *heap)`**
Returns a pointer to the root (`array[0]`) if non-empty, else `NULL`.

**`t_request heap_pop(t_heap *heap)`**
- Empty → returns a zeroed request (coder = NULL) — the caller checks this.
- Saves root, moves the last element to the root, decrements size, `heapify_down`,
  returns the saved root.

**`void heap_destroy(t_heap *heap)`**
Frees the array and zeroes size/capacity.

### `heap_utils.c`

**`static void ft_swap(t_request *a, t_request *b)`**
Swaps two requests by value.

**`static int less(t_request a, t_request b)`**
The ordering predicate that makes it a **min-heap**:
- If priorities differ → smaller priority is "less".
- Else → smaller `order` is "less" (deterministic tie-break).
With FIFO, priority *is* the order, so the heap is a true arrival queue. With
EDF, priority is the deadline, so the nearest-deadline coder is served first.

**`int heap_remove(t_heap *heap, t_coder *coder)`**
Removes *the* request belonging to `coder`:
- Linear scan for `array[i].coder == coder`. If not found → `0`.
- Replace it with the last element, decrement size.
- If the replacement is out of order with its parent, `heapify_up`; otherwise
  `heapify_down`. (The classic remove-by-value fix-up for a binary heap.)
- Returns `1`.
Used when a coder abandons a dongle (sim stopped or timeout).

**`void heapify_up(t_heap *heap, int i)`**
Bubbles element `i` toward the root while it is "less" than its parent
(`parent = (i-1)/2`). Called after push.

**`void heapify_down(t_heap *heap, int i)`**
Sinks element `i` down while any child is "less". Compares both children
(`left = 2i+1`, `right = 2i+2`), swaps with the smallest, repeats. Called after
pop/remove.

### `monitor.c`

**`static int find_burned_coder(t_sim *sim, long *curr)`**
Scans all coders:
- Lock coder's `state_mutex` → `*curr = get_time() - last_compile` → unlock.
- If `*curr > time_to_burnout` → **this coder burned out**, return its index.
Returns `-1` if nobody burned out.

**`static int all_coders_done(t_sim *sim)`**
Scans all coders under their `state_mutex`; if any has `compiles_count <
nb_compiles_required`, sets `all_done = 0`. Returns `1` only when every coder
finished the required compiles.

**`void *monitor_routine(void *arg)`**
The ref's loop:
```
while (!simulation_stopped(sim)):
    i = find_burned_coder(sim, &curr)
    if (i >= 0 || all_coders_done(sim)):
        if (i >= 0): log_status(&coders[i], "burned out")
        stop_simulation(sim)
        return NULL
    smart_sleep(1, sim)     # re-check every ~1 ms
```
It exits as soon as it sets the stop flag. After that, all coder threads see
`simulation_stopped == 1` and unwind (their `smart_sleep`/`wait_for_acquire`/cond
waits all bail within ~1-10 ms).

**`int create_monitor_thread(t_sim *sim)`** / **`int join_monitor_thread(t_sim *sim)`**
Thin wrappers around `pthread_create` / `pthread_join` returning `0` on failure.

### `simulation.c`

**`int simulation_stopped(t_sim *sim)`**
Reads `sim->stop` under `stop_mutex`. Used by every thread in every loop condition.

**`void stop_simulation(t_sim *sim)`**
1. Lock `stop_mutex`, set `stop = 1`, unlock.
2. For every dongle: lock its mutex, `pthread_cond_broadcast`, unlock.
The broadcast is essential: it wakes any coder blocked in
`pthread_cond_timedwait`, who then re-checks `simulation_stopped` and exits
instead of sleeping for up to `poll_ms` more.

**`long elapsed_time(t_sim *sim)`**
`get_time() - start_time` in ms. Used only for logging the timestamp.

### `logger.c`

**`void log_status(t_coder *coder, char *msg)`**
- Locks `print_mutex` (so two threads can never interleave a line).
- Prints `"<elapsed_ms> <coder_id> <msg>\n"` only if the simulation is not stopped
  (so the "burned out" line is not interleaved after other final lines, and no
  extra lines appear after the simulation ends).

### `cleanup.c`

**`static void destroy_coder(t_sim *sim)`**
Destroys every coder `state_mutex`, frees the array, NULLs the pointer.

**`static void destroy_dongles(t_sim *sim)`**
For each dongle: destroy mutex, cond, and heap; then free the array, NULL it.

**`void clean_all(t_sim *sim)`**
Destroys all coders, dongles, then `print_mutex`, `stop_mutex`,
`scheduler_mutex`, the barrier, and finally `free(sim)`. Safe to call with NULL.
**The last thing `main` does — every resource is released, no leaks.**

---

## 7. The Lifecycle of a Coder (Step by Step)

```
      coder_routine
            │
            ├─ one_coder? (N==1) ──────────────► grab single dongle, wait, release
            │
            ▼
      ┌─────────────┐   take_dongles   ┌────────────────────┐
      │  COMPILE    │◄─────────────────│   ACQUIRE 2 DONGLE │
      │  phase      │                  │  (ascending order, │
      └─────┬───────┘                  │   FIFO/EDF heap)   │
            │  last_compile refreshed  └────────────────────┘
            ▼  compiles_count++                 │
      RELEASE DONGLE (before debug!)           ▲
            │                                   │ release → cooldown → broadcast
            ▼                                   │
      ┌─────────────┐                     waits in dongle cond
      │    DEBUG    │ ─────────────────────────►  (blocked until priority reached)
      └─────┬───────┘
            ▼
      ┌─────────────┐
      │  REFACTOR   │
      └─────┬───────┘
            ▼
      loop while !simulation_stopped
            │
            ▼ (monitor set stop = 1)
      return NULL ─────► main joins thread
```

1. All coders start together via the barrier.
2. Each coder computes its two dongles `(left, right)` and requests them in
   ascending order so the ring can't deadlock.
3. While waiting, the coder is parked on the dongle's condition variable, parked
   in the dongle's priority heap. The heap decides who wins when the dongle
   becomes free.
4. On success the coder logs `"has taken a dongle"` (twice — once per dongle).
5. Compile: refreshes `last_compile`, sleeps `time_to_compile`, increments
   `compiles_count`.
6. Dongles released. From here on the coder needs nothing shared — debug and
   refactor are pure sleeps, interruptible by the stop flag.
7. If the monitor finds anyone burned out, it stops the sim; the running coder
   exits its sleep within ~0.5 ms and unwinds.
8. `main` joins the monitor and all coder threads, then frees everything.

---

## 8. The Scheduler in Depth

Each dongle has its own **min-heap of requests**. The root is the next coder to
get that dongle.

**FIFO (`fifo`)**
- `priority = order` where `order = request_counter++` under `scheduler_mutex`.
- So whoever asked first is served first. New requests can never jump the queue.
- This is literally a fair queue.

**EDF (`edf`)**
- `priority = coder->last_compile + time_to_burnout` = absolute deadline.
- The coder closest to burning out is served first.
- `order = (id << 48) | global_counter` makes every coder's order unique and
  deterministic, so two equal deadlines are resolved by id, then arrival.

**Why not `pthread_mutex_trylock`?** — Because we need *fairness* (a queue), not
just mutual exclusion. A raw trylock loop would let fast coders starve slow ones.
The heap turns the dongle into a **prioritized waiting room**.

---

## 9. Deadlock Prevention (the "why can't it freeze?" argument)

Deadlock requires 4 conditions (Coffman). We break **circular wait**:

- Coders are arranged in a ring. Coder `i` needs dongles `i` and `(i+1)%N`.
- Without ordering, coder 0 might hold dongle 0 and want dongle 1 while coder 1
  holds dongle 1 and wants dongle 0 → classic circular wait → freeze.
- `take_dongles` always requests the **lower index first** and, crucially, uses
  `request_dongle_timeout` for the second one. If it can't grab the second within
  2 ms, it **releases the first and retries**.
- Therefore no thread ever holds one dongle while waiting forever on another in a
  cycle — the wait always has a deadline and the hold always gets dropped.

**You can demonstrate this:** run with `N` coders and tiny compile times, e.g.
`./codexion 4 2000 100 100 100 3 10 fifo`. It never hangs.

**N == 1** is special-cased in `one_coder` because `left == right == 0` — a single
coder must not "take dongle 0 twice".

---

## 10. Starvation Prevention

Starvation would be: a low-priority coder never gets dongles while others cycle
through them forever.

- The **heap** guarantees the coder at the root is served when the dongle frees.
- **FIFO** makes this literal: arrival order is strictly respected.
- **EDF** could in theory starve someone far from their deadline while others keep
  getting closer deadlines... but:
  - Every compile **refreshes** `last_compile`, so a well-fed coder's deadline
    moves forward (its priority worsens), letting the starving coder rise to the
    root.
  - The **cooldown** prevents the same coder from instantly re-taking a dongle.
  - If a starving coder's deadline does arrive, it rises to the root and gets the
    dongle — or it burns out, which is a *visible, correct* outcome (the monitor
    reports it).

---

## 11. Burnout Detection

- `last_compile` is refreshed **at the start** of each compile under `state_mutex`.
- The monitor reads `get_time() - last_compile` under the same mutex and compares
  with `time_to_burnout` (`>`).
- If the gap exceeds the budget, the coder is reported as `"burned out"` and the
  simulation stops.
- `smart_sleep(1)` in the monitor means detection latency ≤ ~1 ms.

**Timing caveat to know for the defense:** burnout is only detected while the
coder is *not* compiling. During a compile phase `last_compile` is fresh, so a
very short `time_to_burnout` (e.g. less than `time_to_compile`) will simply never
trip burnout — the coder is "safe" the whole time it compiles. This matches the
spirit of the project (a coder that keeps meeting its schedule is not burned out).

---

## 12. The Cooldown Mechanism

- `release_dongle` sets `held = 0` and `cooldown_until = now + dongle_cooldown`.
- A waiting coder may only acquire if `now >= cooldown_until` (`wait_for_acquire`
  checks it before popping the heap).
- This prevents the pattern where coder A releases a dongle and instantly
  re-takes it before coder B ever gets a turn, and it models a real "USB port
  needs a second to reset" behavior.

---

## 13. Synchronization Inventory

| Shared data | Protected by | Used by |
|---|---|---|
| `sim->stop` | `stop_mutex` | everyone |
| `sim->request_counter` | `scheduler_mutex` | request builders |
| stdout | `print_mutex` | `log_status` |
| coder `last_compile`, `compiles_count` | per-coder `state_mutex` | coder thread & monitor |
| dongle `held`, `cooldown_until`, heap | per-dongle `mutex` | requester/releaser |
| waiters on a dongle | per-dongle `cond` | waiters + releaser + stopper |
| thread start | `start_barrier` (N+1) | all coder threads + main |

**Lock ordering** (no cycle): a thread holds at most **one** dongle mutex at a
time (each `request_dongle` uses a single dongle mutex, and the two dongles are
requested sequentially, never held in a nested lock). Coder state locks and dongle
locks are never held together. This is what makes the program deadlock-free.

---

## 14. Time & Timing

- `get_time()` = epoch ms (`gettimeofday`). All durations are ms.
- `smart_sleep` = poll-based sleep (500 µs) that aborts when the sim stops.
- `get_timeout` builds the absolute `timespec` needed by `pthread_cond_timedwait`
  (so spurious wakeups and broadcasts still respect the deadline).
- `elapsed_time` = `get_time() - start_time` only for log timestamps.

**Why polling instead of `usleep`?** A plain `usleep(100000)` would take 100 ms to
notice a stop; the monitor wants < ~10 ms reaction. Polling is the price of a
responsive, leak-free shutdown.

---

## 15. Edge Cases & Why They're Safe

| Case | Handling |
|---|---|
| `N == 1` | `one_coder` path — never double-requests dongle 0 |
| Heap full in `request_dongle` | `heap_push` fails → unlock, return `0` (never happens in practice) |
| Sim stops while a coder waits | `wait_for_acquire` loop condition → returns `0` → `heap_remove` + broadcast |
| Sim stops during compile/debug/refactor | `smart_sleep` aborts → coder returns → main joins it |
| Sim stops between acquiring dongles | `coder_routine` checks `simulation_stopped` after `take_dongles`, releases both |
| Monitor thread creation fails | `run_simulation` sets stop, joins coders, reports error |
| Coder thread creation fails partway | created threads are joined immediately |
| Bad arguments | `validate_args`/`fill_data` reject → `"Parsing error"` exit code 2 |
| malloc fails | every alloc is checked; errors exit cleanly (no crash, no UB) |
| Very large durations | `smart_sleep`/`get_timeout` loop in chunks — no integer overflow |
| Spurious cond wakeups | loops always re-check the condition (`top`, `held`, `cooldown`) |

---

## 16. Evaluation Q&A (likely questions + suggested answers)

**Q: Why a heap and not a mutex + flag per dongle?**
A: Fairness. A plain mutex has no queue — the OS scheduler decides who wakes, so a
fast coder could starve a slow one. The per-dongle min-heap is an explicit,
policy-driven waiting room: FIFO or EDF.

**Q: How do you prevent deadlock?**
A: Ascending-order acquisition (`take_dongles` picks `min(left,right)` first) plus
a timeout on the second dongle so a coder never sits holding one dongle forever.
Circular wait is impossible.

**Q: How do you prevent starvation?**
A: The heap always serves the root; FIFO is pure arrival order; EDF pulls in the
nearest deadline; the cooldown stops one coder from instantly re-taking; and a
coder whose deadline never gets served is *detected* (burnout) rather than silently
starved.

**Q: What does the monitor do?**
A: Every ~1 ms it checks each coder's `now - last_compile`. If it exceeds
`time_to_burnout`, it logs `"burned out"` and stops everything. It also detects
completion (all coders reached `nb_compiles_required`) and stops on success.

**Q: Why is the dongle released before debugging/refactoring?**
A: Because the coder only needs dongles to compile. Releasing early frees
resources for others and is more realistic (and makes the sim less contended).

**Q: Why do you poll with `usleep` instead of using `pthread_cond_timedwait` for the phases?**
A: The phases are pure time delays with no signal to wait on; a cond wait would
offer nothing over polling here. Polling with `smart_sleep` gives an immediate,
clean response to the stop flag.

**Q: How do you guarantee the printed output is not garbled?**
A: Every `log_status` locks `print_mutex` around the whole `printf`.

**Q: Why `start_barrier`?**
A: So every coder begins at the same instant and `last_compile = start_time` is
fair from t=0. Without it, early-started threads would have an advantage.

**Q: Can a coder take the same dongle twice?**
A: Only the `N == 1` case has `left == right`, and it's special-cased. For N > 1,
`left != right` always.

**Q: Memory leaks?**
A: `clean_all` frees coders, dongles, each heap array, and `sim`; `main` frees
`data`. Every malloc is checked. Use `valgrind --leak-check=full ./codexion ...`
to show zero leaks if asked.

**Q: What happens if you run `./codexion 5 3000 200 150 100 5 15 edf`?**
A: 5 coders, 3000 ms burnout, 200/150/100 ms phases, 5 compiles each to win,
15 ms dongle cooldown, EDF scheduling. It completes when all 5 coders have
compiled 5 times, or stops with `"burned out"` if any misses its 3 s deadline.

---

## 17. Quick Cheat Sheet (flow, in 6 lines)

1. `main` parses args → builds `t_data`.
2. `init_sim` → `init_dongles` → `init_coders` → all mutexes/heaps/barrier ready.
3. `create_coder_threads` spawn N coders; barrier releases them together.
4. Monitor spawns; coders loop: acquire 2 dongles (ordered, heap-prioritized,
   time-limited) → compile → release → debug → refactor.
5. Monitor stops the sim on **burnout** or **all compiles done**, broadcasting all
   dongle conds so waiters wake and exit.
6. `main` joins all threads, `clean_all` frees everything, exit code 0.
