*This project has been created as part of the 42 curriculum by yikoubaz.*

## Table of contents

- [Description](#description)
- [The Project Idea](#the-project-idea)
- [How I started](#how-i-started)
- [Core Concepts Explained](#core-concepts-explained)
  - [Thread vs Process](#thread-vs-process)
  - [Mutex](#mutex)
  - [Data race](#data-race)
- [Architecture / Codebase Structure](#architecture--codebase-structure)
- [Data Flow: from INPUT to simulated OUTPUT](#data-flow-from-input-to-simulated-output)
- [Compilation](#compilation)
- [Usage](#usage)
- [Deadlock Prevention Strategy](#deadlock-prevention-strategy)
- [Blocking cases handled](#blocking-cases-handled)
- [Thread synchronization mechanisms](#thread-synchronization-mechanisms)
- [Resources](#resources)

## Description

Codexion is a concurrency simulation that models multiple coders competing for limited USB dongles in a shared co-working hub. Each coder needs two dongles to compile quantum code, and must coordinate with others to avoid deadlocks, starvation, and burnout. The project implements POSIX threads, mutexes, condition variables, and priority scheduling (FIFO/EDF) to orchestrate resource sharing.

The simulation stops when all coders have compiled a required number of times, or immediately when any coder burns out from missing their compilation deadline.

## The Project Idea

Imagine a co-working hub with a fixed number of USB dongles — exactly one per coder. But here is the twist: **compiling quantum code requires two dongles at the same time**. A coder cannot compile with just one.

Since every coder needs two dongles and there is exactly one dongle per coder, the system is **always in contention**. The hub has no manager; every coder is a selfish thread grabbing dongles on its own. This is the classic *Dining Philosophers* problem in disguise:

- **Coders** = philosophers (threads)
- **Dongles** = forks (shared resources)
- **Compiling** = eating (critical section)
- **Burning out** = dying of starvation

The challenge is to make sure that:

1. Every coder eventually gets to compile (no **starvation**).
2. No subset of coders gets permanently stuck waiting on each other (no **deadlock**).
3. The whole simulation behaves deterministically enough to be testable.

To make it more realistic (and harder), I added three real-world mechanics on top of the classic problem:

- **Work phases**: after compiling, a coder debugs and refactors. These phases are done **without** holding dongles, freeing resources for others.
- **Cooldown**: a released dongle needs a cooldown period before it can be grabbed again, preventing the same coder from instantly re-acquiring it.
- **Schedulers**: instead of random grabbing, coders file *requests* into a priority queue and are granted dongles by a scheduling policy (`fifo` or `edf`).

## How I started

The project started from the classic **Dining Philosophers** framing, then grew organically:

1. **Model the problem** — turn coders/dongles into threads and shared resources. Each coder is a `pthread_t`, each dongle is protected by a `pthread_mutex_t`.
2. **Solve deadlock first** — before any scheduling fancy, I had to guarantee the system cannot freeze. The first move was enforcing a **resource hierarchy** (more on that in [Deadlock Prevention](#deadlock-prevention-strategy)).
3. **Add fairness** — a plain grab-when-free system starves low-priority coders. That pushed me to design a **request/scheduler layer**: coders enqueue `t_request` entries into per-dongle **min-heaps**, and a scheduler decides who is next.
4. **Add the monitor** — a dedicated thread (`monitor_routine`) watches every coder's `last_compile` time and kills the simulation the moment anyone burns out, or stops it cleanly when everyone has compiled enough times.
5. **Harden with synchronization** — every shared field got a protecting mutex, and every sleep became a `smart_sleep` that aborts early when the simulation stops, so threads never hang on shutdown.

The final structure mirrors that journey: each concern lives in its own file (see [Architecture](#architecture--codebase-structure)).

## Core Concepts Explained

### Thread vs Process

| | Process | Thread |
|---|---|---|
| Memory | Owns its own memory space (isolated) | Shares the process's memory space |
| Communication | Needs IPC (pipes, sockets, ...) | Directly reads/writes shared variables |
| Context switch | Expensive | Cheap |
| Crash | One crash takes only its process down | A bug can corrupt shared state for everyone |

Codexion uses **threads**, not processes, because the whole point is that all coders **share** the same dongles and the same simulation clock. That sharing is what makes the simulation realistic — and what makes it dangerous, because shared memory is where **data races** happen.

### Mutex

A **mutex** (mutual exclusion) is a lock that guarantees only one thread can enter a critical section at a time:

```
pthread_mutex_lock(&m);   // 1 thread enters, others block
  ... shared data access ...   // critical section
pthread_mutex_unlock(&m); // next waiting thread may enter
```

In Codexion, mutexes protect:

- Each **dongle's state** (`held`, `cooldown_until`) — `dongle->mutex`
- Each **coder's state** (`last_compile`, `compiles_count`) — `coder->state_mutex`
- The **simulation stop flag** — `sim->stop_mutex`
- The **scheduler arbitration** (ordering of grants) — `sim->grant_mutex`
- The **console output** — `sim->print_mutex`

### Data race

A **data race** is when two or more threads access the same memory location at the same time, and at least one of them writes — without any synchronization. The result is undefined behavior: lost updates, corrupted state, or crashes that only happen sometimes.

Example of a race in Codexion if `last_compile` had no mutex:

```
Thread A (coder 1)                    Thread B (monitor)
last_compile = get_time();            if (get_time() - last_compile >= burnout)
  // A writes, B reads simultaneously → last_compile may be read as garbage
```

Codexion avoids this by locking the **state_mutex** every single time `last_compile` or `compiles_count` is read or written (see `find_burned_coder`, `all_coders_done`, `do_work` in `thread.c`). If even one access is unprotected, the race silently reappears — this is why the mutex discipline has to be *everywhere*, not just "mostly".

## Architecture / Codebase Structure

| File | Role |
|---|---|
| `src/main.c` | Entry point: orchestration (parse → init → run → cleanup) |
| `src/parser.c` | Validate argv and fill `t_data` |
| `src/codexion.h` | All data structures, constants, and prototypes |
| `src/init.c` | Allocate and initialize `t_sim`, `t_dongle`, `t_coder` |
| `src/thread.c` | Coder threads: the coder lifecycle (compile/debug/refactor) |
| `src/thread_time_init.c` | Start the shared clock and stamp every coder's start time |
| `src/simulation.c` | Global simulation state helpers (stop flag, elapsed time) |
| `src/dongles.c` | Acquire/release dongles for the two-dongle case |
| `src/scheduler.c` | Request/single-dongle grant logic (`request_dongle`) |
| `src/scheduler_utils.c` | Request building, eligibility checks, dongle state helpers |
| `src/heap.c` | Min-heap core: init, push, pop, peek |
| `src/heap_utils.c` | Heap algorithms: `heapify_up/down`, removal, comparison |
| `src/monitor.c` | Monitor thread: burnout detection + completion check |
| `src/monitor_util.c` | `find_burned_coder`, `all_coders_done` |
| `src/logger.c` | Mutex-protected console output |
| `src/utils.c` | `get_time`, `smart_sleep`, `ft_atol`, ... |
| `src/cleanup.c` | Destroy all mutexes/conds/heaps and free memory |

## Data Flow: from INPUT to simulated OUTPUT

```
             ┌────────────────────────────────────────────────────────────┐
   INPUT     │                                                            │
   argv  ──► │ parser.c  ──►  t_data  ──►  init.c  ──►  t_sim (world)     │
             │                  (all arguments            ├─ coders[]      │
             │                   validated                ├─ dongles[]     │
             │                   & stored)                └─ mutexes/cond  │
             └────────────────────────────────────────────────────────────┘
                                        │
                                        ▼
             ┌────────────────────────────────────────────────────────────┐
 SIMULATION  │  thread_time_init()  ←  shared start_time clock starts      │
             │                                                            │
             │  coder threads (1 per coder)          monitor thread        │
             │  ┌──────────────────────────┐          ┌──────────────────┐ │
             │  │ loop while !stop:        │          │ loop while !stop │ │
             │  │   take_dongles()  ────►  │          │   find_burned?   │ │
             │  │     push request to      │          │   all done?      │ │
             │  │     per-dongle heap      │          │   else sleep 1ms │ │
             │  │   (granted by scheduler) │          └──────────────────┘ │
             │  │   compile ─► debug ─►    │               │  │            │
             │  │   refactor (smart_sleep) │        burnout│  │all done    │
             │  │   release dongles ──►    │               ▼  ▼            │
             │  │     cooldown starts      │          stop_simulation()    │
             │  └──────────────────────────┘               │               │
             └─────────────────────────────────────────────┼───────────────┘
                                        │
                                        ▼
             ┌────────────────────────────────────────────────────────────┐
   OUTPUT    │  logger.c prints, under print_mutex:                        │
             │    <elapsed_ms> <coder_id> is compiling / debugging / ...    │
             │    <elapsed_ms> <coder_id> burned out   (final, if burnout)  │
             │  Exit code 0 → clean end, all coders done                    │
             └────────────────────────────────────────────────────────────┘
```

Step by step:

1. **INPUT** — argv is validated (`parser.c`): exactly 8 arguments, all numeric (except the last), within `int` range, `>= 0`, and the scheduler is `fifo` or `edf`. Result is stored in `t_data`.
2. **WORLD BUILDING** — `init.c` allocates the coder and dongle arrays and initializes every mutex, condition variable, and request heap. Each coder is linked to its `left` and `right` dongle.
3. **START GATE** — `create_coder_threads` holds `start_mutex` while spawning all coder threads, so every thread blocks at the gate. When all are spawned, `thread_time_init` starts the global clock and stamps every coder's `last_compile`, then the gate opens. This guarantees a fair, synchronized start.
4. **CODERS RUN** — each coder loops: request its two dongles (via the scheduler), compile, release the dongles, then debug and refactor **without** holding resources. It repeats until it has compiled `nb_compiles_required` times or the simulation stops.
5. **MONITOR** — in parallel, the monitor checks whether anyone has gone `>= time_to_burnout` ms without a compile start, or whether everyone is done.
6. **STOP** — on burnout it prints the death line and stops; on completion it stops cleanly. `stop_simulation` sets the flag and broadcasts every dongle condition variable so no coder stays blocked.
7. **OUTPUT** — all messages go through `logger.c`, which holds `print_mutex` so lines never interleave.

## Compilation

```sh
make          # Build the codexion executable
make clean    # Remove object files
make fclean   # Remove object files and executable
make re       # Rebuild from scratch
```

## Usage

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

**Examples**

```sh
./codexion 4 2000 100 100 100 3 10 fifo
./codexion 5 3000 200 150 100 5 15 edf
./codexion 1 2000 100 100 100 3 10 fifo
```

## Deadlock Prevention Strategy

Deadlock needs **four conditions** (Coffman) to hold simultaneously:

1. Mutual exclusion — resources can't be shared.
2. Hold and wait — a thread holds one resource while waiting for another.
3. No preemption — resources can't be taken away.
4. Circular wait — a cycle of threads each waiting on a resource held by the next.

Codexion breaks the cycle by attacking condition **4** with a **resource hierarchy**: every coder is wired to its two dongles as `left` and `right` (init.c):

```
coder i → left = i, right = (i + 1) % nb_coders
```

A coder always files requests for its **lower-indexed dongle first** and can only obtain both in a consistent global order. In a ring topology this removes any possibility of a circular wait, because the whole system can be ordered linearly along dongle indices — there is no "everyone waits on the next one" cycle anymore. This is the classic *ordered resource allocation* from Coffman's own paper.

Additionally, the **single-dongle grant path** (`request_dongle` in `scheduler.c`) locks `grant_mutex` around every arbitration decision, and dongles are only ever marked `held` under their own mutex — so the *grant* itself is atomic and two coders can never both believe they own the same dongle.

## Blocking cases handled

### Starvation prevention

The **priority queue (heap)** ensures fair arbitration:
- **FIFO**: Requests are served in arrival order, preventing newer requests from bypassing older ones.
- **EDF (Earliest Deadline First)**: The coder with the closest burnout deadline is served first, with request order as tie-breaker.

Before a coder is granted a dongle, `is_eligible` (scheduler_utils.c) verifies that no higher-priority request exists — and if one does, that its owner is *currently blocked on the paired dongle*. This prevents a coder from grabbing one dongle while a "more urgent" coder is stuck holding nothing.

### Cooldown handling

After a coder releases a dongle, the dongle enters a cooldown period during which it cannot be re-acquired (`dongle->cooldown_until = now + dongle_cooldown`). This prevents immediate re-acquisition that could starve other waiting coders and adds realism to resource contention.

### Burnout detection

A dedicated monitor thread continuously checks each coder's elapsed time since their last compile start against the burnout threshold (`find_burned_coder`). When burnout is detected, the simulation stops and a burnout message is printed within 10ms.

### Clean shutdown

Every blocking wait and sleep is "smart": `smart_sleep` and the polling loops re-check `simulation_stopped()` every 200–500µs, and `stop_simulation` broadcasts all dongle condition variables. No thread is left hanging when the simulation ends.

### Log serialization

All log output is protected by a global print mutex, preventing interleaved messages from concurrent coders.

## Thread synchronization mechanisms

### Mutexes (`pthread_mutex_t`)

- **Per-dongle mutex**: Protects each dongle's state (held flag, cooldown timer) and its associated heap. All operations on a dongle's request queue are serialized through this mutex.
- **Print mutex**: Serializes all log output to prevent message interleaving.
- **Stop mutex**: Protects the simulation stop flag for safe cross-thread access.
- **Grant mutex**: Serializes request ordering and dongle grants across all dongles.
- **Start mutex**: Keeps all coder threads gated until every one of them is spawned, guaranteeing a synchronized start.
- **Per-coder state mutex**: Protects each coder's `last_compile` time and `compiles_count`.

### Condition variables (`pthread_cond_t`)

Each dongle has an associated condition variable. Coders waiting for a dongle can call `pthread_cond_wait` on the dongle's condition variable, releasing the dongle mutex and blocking until signaled. When a dongle is released (`held = 0`), `pthread_cond_broadcast` wakes all waiters, who then re-check the scheduling priority.

When the simulation stops, all dongle condition variables are broadcast to ensure no coder thread remains blocked.

### Priority queue (heap)

Each dongle maintains a **min-heap** of requests. The scheduling policy determines the priority:
- **FIFO**: Priority = request arrival order (lower = earlier)
- **EDF**: Priority = absolute deadline (`last_compile_start + time_to_burnout`)

Tie-breaking uses request order, ensuring deterministic behavior even with identical priorities.

### Race condition prevention

- Resource acquisition follows a strict ordering (ascending dongle index) to prevent deadlock.
- The heap pop operation is only performed by the thread that acquires the dongle, under the dongle mutex.
- Thread-safe communication between coders and the monitor is achieved through shared state protected by the `state_mutex`.

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
