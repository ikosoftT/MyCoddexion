*This project has been created as part of the 42 curriculum by yikoubaz.*

## Description

Codexion is a concurrency simulation that models multiple coders competing for limited USB dongles in a shared co-working hub. Each coder needs two dongles to compile quantum code, and must coordinate with others to avoid deadlocks, starvation, and burnout. The project implements POSIX threads, mutexes, condition variables, and priority scheduling (FIFO/EDF) to orchestrate resource sharing.

The simulation stops when all coders have compiled a required number of times, or immediately when any coder burns out from missing their compilation deadline.

## Instructions

### Compilation

```sh
make          # Build the codexion executable
make clean    # Remove object files
make fclean   # Remove object files and executable
make re       # Rebuild from scratch
```

### Usage

```sh
./codexion <number_of_coders> <time_to_burnout> <time_to_compile> \
           <time_to_debug> <time_to_refactor> <number_of_compiles_required> \
           <dongle_cooldown> <scheduler>
```

**Arguments:**

| Argument | Description |
|---|---|
| `number_of_coders` | Number of coders (and dongles) |
| `time_to_burnout` | Max ms without compiling before burnout |
| `time_to_compile` | Duration of compile phase (ms) |
| `time_to_debug` | Duration of debug phase (ms) |
| `time_to_refactor` | Duration of refactor phase (ms) |
| `number_of_compiles_required` | Compiles needed to end simulation |
| `dongle_cooldown` | Cooldown after releasing a dongle (ms) |
| `scheduler` | `fifo` or `edf` |

### Examples

```sh
./codexion 4 2000 100 100 100 3 10 fifo
./codexion 5 3000 200 150 100 5 15 edf
./codexion 1 2000 100 100 100 3 10 fifo
```

## Blocking cases handled

### Deadlock prevention (Coffman's conditions)

Deadlock is prevented by enforcing a **resource hierarchy** (Coffman's third condition): coders always acquire dongles in ascending index order. This breaks the circular wait condition that would otherwise occur in the ring topology.

### Starvation prevention

The **priority queue (heap)** ensures fair arbitration:
- **FIFO**: Requests are served in arrival order, preventing newer requests from bypassing older ones.
- **EDF (Earliest Deadline First)**: The coder with the closest burnout deadline is served first, with request order as tie-breaker.

### Cooldown handling

After a coder releases a dongle, the dongle enters a cooldown period during which it cannot be re-acquired. This prevents immediate re-acquisition that could starve other waiting coders and adds realism to resource contention.

### Burnout detection

A dedicated monitor thread continuously checks each coder's elapsed time since their last compile start against the burnout threshold. When burnout is detected, the simulation stops and a burnout message is printed within 10ms.

### Log serialization

All log output is protected by a global print mutex, preventing interleaved messages from concurrent coders.

## Thread synchronization mechanisms

### Mutexes (`pthread_mutex_t`)

- **Per-dongle mutex**: Protects each dongle's state (held flag, cooldown timer) and its associated heap. All operations on a dongle's request queue are serialized through this mutex.
- **Print mutex**: Serializes all log output to prevent message interleaving.
- **Stop mutex**: Protects the simulation stop flag for safe cross-thread access.
- **Scheduler mutex**: Serializes request ordering across dongles.
- **Per-coder state mutex**: Protects each coder's `last_compile` time and `compiles_count`.

### Condition variables (`pthread_cond_t`)

Each dongle has an associated condition variable. Coders waiting for a dongle call `pthread_cond_wait` on the dongle's condition variable, releasing the dongle mutex and blocking until signaled. When a dongle is released (`held = 0`), `pthread_cond_broadcast` wakes all waiters, who then re-check the scheduling priority.

When the simulation stops, all dongle condition variables are broadcast to ensure no coder thread remains blocked.

### Priority queue (heap)

Each dongle maintains a **min-heap** of requests. The scheduling policy determines the priority:
- **FIFO**: Priority = request arrival order (lower = earlier)
- **EDF**: Priority = absolute deadline (`last_compile_start + time_to_burnout`)

Tie-breaking uses request order, ensuring deterministic behavior even with identical priorities.

### Race condition prevention

- Resource acquisition follows a strict ordering (ascending dongle index) to prevent deadlock.
- The heap pop operation is only performed by the thread that acquires the dongle, under the dongle mutex.
- Thread-safe communication between coders and the monitor is achieved through shared state protected by the state_mutex.

## Resources

### References

- POSIX Threads Programming (Lawrence Livermore National Laboratory)
- Dining Philosophers Problem — Edsger Dijkstra
- "Operating Systems: Three Easy Pieces" — Remzi H. Arpaci-Dusseau
- Coffman, E.G., Elphick, M.J., Shoshani, A. "System Deadlocks" (1971)

### AI usage

AI used for Understanding the problems of (Deadlocks, race, etc..).

- Optimizing My Code Base Always Getting Bettre Flow
- Norm the Code and Style it. 
