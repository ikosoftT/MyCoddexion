*This project has been created as part of the 42 curriculum by yikoubaz.*

## Description

**Codexion** is a concurrent programming simulation inspired by the classic Dining Philosophers problem. It models a co-working hub where multiple coders compete for limited USB dongles to compile their quantum code. The simulation uses POSIX threads (pthreads), mutexes, condition variables, and custom priority queue scheduling to coordinate resource access.

Each coder cycles through three states — **compiling**, **debugging**, and **refactoring** — requiring two dongles simultaneously to compile. A monitor thread detects burnout (missing a compile deadline) and terminates the simulation. Two scheduling policies are supported: **FIFO** (First In, First Out) and **EDF** (Earliest Deadline First).

## Instructions

### Compilation

```bash
make
```

This produces the `codexion` executable.

### Usage

```bash
./codexion <nb_coders> <time_to_burnout> <time_to_compile> <time_to_debug> \
           <time_to_refactor> <nb_compiles_required> <dongle_cooldown> <scheduler>
```

| Argument | Description |
|---|---|
| `nb_coders` | Number of coders (and dongles) |
| `time_to_burnout` | Max ms without compiling before burnout |
| `time_to_compile` | Duration of a compile session (ms) |
| `time_to_debug` | Duration of debugging (ms) |
| `time_to_refactor` | Duration of refactoring (ms) |
| `nb_compiles_required` | Compiles per coder needed to end simulation |
| `dongle_cooldown` | Cooldown after a dongle is released (ms) |
| `scheduler` | `fifo` or `edf` |

### Cleanup

```bash
make clean    # remove object files
make fclean   # remove objects and binary
make re       # rebuild from scratch
```

### Example

```bash
./codexion 3 800 200 300 200 2 50 fifo
```

## Blocking Cases Handled

### Deadlock Prevention

Deadlock is prevented using **resource ordering**: each coder always acquires the lower-indexed dongle first. This breaks the circular wait condition (Coffman's fourth condition) because at least one coder will acquire dongles in a different order than the others.

- Coder N: left = dongle N, right = dongle (N+1) % nb_coders
- If `left->id > right->id`, the coder takes the right dongle first

For a single coder (one dongle), left and right point to the same dongle and it is acquired only once.

### Starvation Prevention

Both schedulers maintain a **priority queue (min-heap)** at each dongle. When a coder requests a dongle, its request is enqueued with a priority value. The dongle is granted to the highest-priority requester when it becomes available, ensuring fairness:

- **FIFO**: priority = request arrival timestamp
- **EDF**: priority = `last_compile_start + time_to_burnout` (earliest deadline)
- Tie-breaking: global request order (monotonically increasing counter) ensures determinism

### Dongle Cooldown

After a coder releases a dongle, it enters a cooldown period (`dongle_cooldown` ms). The next coder in the priority queue uses `pthread_cond_timedwait` with the cooldown expiration time as the absolute timeout, ensuring it wakes precisely when the dongle becomes available. If a coder is not at the front of the queue, it waits using `pthread_cond_wait` and is awakened by broadcasts when the queue state changes.

### Precise Burnout Detection

A dedicated **monitor thread** continuously checks each coder's deadline (`last_compile_start + time_to_burnout` against the current time). It runs every 500 microseconds and logs the burnout message within 10ms of the actual burnout. On detecting burnout:

1. Sets `sim->stopped = 1` under `run_mutex`
2. Logs the burnout message
3. Broadcasts on all dongle condition variables to wake waiting coders, which then check `sim->stopped` and exit

### Log Serialization

All log output is protected by a dedicated `print_mutex`. A coder or the monitor locks this mutex before calling `printf`, ensuring messages never interleave on a single line. After the simulation stops, non-burnout messages are suppressed to avoid spurious output.

## Thread Synchronization Mechanisms

### Mutexes

| Mutex | Location | Purpose |
|---|---|---|
| `run_mutex` | `t_sim` | Protects simulation state (`stopped`, `coders_finished`, `last_compile_start`) |
| `print_mutex` | `t_sim` | Serializes log output to prevent interleaved messages |
| `order_mutex` | `t_sim` | Protects the global request order counter |
| `dongle[i].mutex` | Each `t_dongle` | Protects dongle state (held flag, cooldown, priority queue) |

### Condition Variables

| Condvar | Location | Purpose |
|---|---|---|
| `dongle[i].cond` | Each `t_dongle` | Blocks coders waiting for a dongle; broadcast on release to wake waiters |
| `run_cond` | `t_sim` | Broadcast when all coders finish to wake any waiting threads |

### Custom Priority Queue (Min-Heap)

A binary heap is implemented from scratch for dongle arbitration. Each heap node contains:

- `coder_id` — the waiting coder
- `priority` — timestamp (FIFO) or deadline (EDF)
- `order` — global sequence number for deterministic tie-breaking

Operations: `pq_push` (insert with sift-up), `pq_pop` (extract min with sift-down), `pq_peek` (read min without removal), `pq_remove_coder` (remove a specific coder when the simulation stops).

### Coder-Monitor Communication

Coders and the monitor communicate through shared state protected by `run_mutex`:

- **Coder → Monitor**: The coder updates `last_compile_start` before logging "is compiling". The monitor reads this to compute deadlines.
- **Monitor → Coder**: The monitor sets `sim->stopped` and broadcasts on all dongle condition variables. Coders waiting on dongles wake up, detect `sim->stopped`, remove themselves from the queue, and exit.

The monitor also checks `coders_finished` (incremented by each coder after completing a compile) and terminates the simulation when all coders have compiled sufficiently.

## Resources

### References

- [POSIX Threads Programming](https://computing.llnl.gov/tutorials/pthreads/) — LLNL Pthreads tutorial
- [Dining Philosophers Problem](https://en.wikipedia.org/wiki/Dining_philosophers_problem) — Wikipedia
- [Coffman Conditions for Deadlock](https://en.wikipedia.org/wiki/Coffman_conditions) — Wikipedia
- [Earliest Deadline First Scheduling](https://en.wikipedia.org/wiki/Earliest_deadline_first_scheduling) — Wikipedia
- [Binary Heap](https://en.wikipedia.org/wiki/Binary_heap) — Wikipedia

### AI Usage

AI was used to:

- Design and refine the overall architecture (struct layout, thread coordination patterns)
- Debug concurrency issues (cooldown deadlock, missing variable initialization, incorrect lock ordering)
- Generate initial function bodies for the priority queue, monitoring logic, and synchronization primitives
- All AI-generated code was reviewed, tested, and modified to ensure correctness and norm compliance
