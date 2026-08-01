# Codexion Bug Report & Resolution

## Bug 1: Thread Creation Race (Barrier Missing)

### Detection
While reviewing `thread.c` → `create_coder_threads()` → `coder_routine()`, I noticed that coder threads start executing **immediately** upon `pthread_create()`. This means coder threads compete for dongles before all threads have been created, giving early-started coders an unfair advantage. The subject explicitly requires: *"don't start until all coders have been created"*.

### Symptoms
- Coders created first (e.g., coder 1) could acquire dongles before later coders (e.g., coder 8) were even instantiated.
- The early coders' `last_compile` timestamp was set to `start_time`, while late coders started with the same timestamp but missed the initial race, causing them to appear closer to burnout.
- Unfair scheduling: the first few threads monopolized dongles before others could compete.

### Fix
Added a `pthread_barrier_t start_barrier` to `t_sim` (in `codexion.h`). The barrier is initialized with count = `nb_coders + 1` (one for each coder thread + the main thread). All coder threads wait on the barrier at the start of `coder_routine()`. After creating all coder threads, the main thread also waits on the barrier, releasing all coders simultaneously.

**Files modified:**
- `src/codexion.h`: Added `pthread_barrier_t start_barrier` field
- `src/init.c`: Initialize barrier in `init_sim()`
- `src/main.c`: Main thread waits on barrier after creating coders
- `src/thread.c`: Coder threads wait on barrier before starting
- `src/cleanup.c`: Destroy barrier on cleanup

---

## Bug 2: Critical Timeout Logic Error in `request_dongle()`

### Detection
In `src/scheduler.c`, the `request_dongle()` function had this code:

```c
get_timeout(&timeout, 2);
pthread_cond_timedwait(&dongle->cond, &dongle->mutex, &timeout);
// TODO: I dont Understand the Below Condtion
if (get_time() > timeout.tv_sec * 1000)
    break;
```

The condition `get_time() > timeout.tv_sec * 1000` is **always true immediately** because:
- `get_time()` returns epoch **milliseconds** (e.g., `1745000000123`)
- `timeout.tv_sec` is epoch **seconds** from `clock_gettime()` (e.g., `1745000000`)
- `timeout.tv_sec * 1000` = `1745000000000`
- `1745000000123 > 1745000000000` → **TRUE** (within 1ms of starting!)

This means the waiting loop always breaks **after a single 2ms wait**, even though the dongle hasn't been acquired. The coder would:
1. Acquire first dongle ✅
2. Push request to second dongle's heap
3. Wait ~2ms (the `pthread_cond_timedwait`)
4. The broken timeout check exits the loop ❌
5. Remove itself from the heap
6. Release the first dongle
7. Retry from scratch — constant churn!

This caused **massive livelock**: coders constantly acquired and released their first dongle without ever getting the second one, making no forward progress.

### Symptoms
- Only 1–2 "lucky" coders would make progress while others starved and burned out.
- Coder 1 consistently burned out at exactly `time_to_burnout` ms.
- The bug was masked in the 1-coder case because the premature loop exit accidentally prevented deadlock (the second `request_dongle` would have deadlocked waiting for the same dongle).

### Fix
Removed the incorrect `if (get_time() > timeout.tv_sec * 1000) break;` check entirely. The loop now waits patiently until either:
- The coder reaches the top of the heap AND the dongle is available (`!dongle->held`) AND cooldown has expired — then `return (1)` ✅
- The simulation stops (monitor broadcasts all condition variables) — then `return (0)` ✅

Changed the timeout from 2ms to 10ms to reduce CPU usage from spurious wakeups, since `pthread_cond_timedwait` is only needed to periodically check `simulation_stopped()`.

**Files modified:**
- `src/scheduler.c`: Removed buggy timeout check

---

## Bug 3: 1-Coder Case Requests Same Dongle Twice (Deadlock)

### Detection
In `src/thread.c`, the `one_coder()` function handled the special case of a single coder with a single dongle. It called `request_dongle()` **twice** for the same dongle — once for "left" and once for "right":

```c
if (!request_dongle(coder, &coder->sim->dongles[0]))
    return (1);
log_status(coder, "has taken a dongle");
// ... then calls request_dongle AGAIN for the same dongle
if (!request_dongle(coder, &coder->sim->dongles[0]))
    return (1);
log_status(coder, "has taken a dongle");
```

The first call acquires the dongle (sets `dongle->held = 1`). The second call pushes to the heap and waits... but `dongle->held == 1`, and **the same thread holds it**. With the timeout bug, this accidentally worked because the premature break let it escape. But with the timeout bug fixed, this would be a **permanent deadlock** — the second `request_dongle` would wait forever because the dongle is held by the same thread that's waiting for it.

### Symptoms
- With the timeout fix applied (Bug 2), the 1-coder test would hang indefinitely.
- Before the fix, it "worked" only because of Bug 2's premature timeout bailout.

### Fix
Since the subject says *"If there is only one coder, there should be only one dongle on the table"*, a single coder only needs **one dongle** to compile. Removed the redundant second `request_dongle` call. The single dongle is acquired once, held during compile, and released afterward.

**Files modified:**
- `src/thread.c`: Simplified `one_coder()` to acquire the single dongle exactly once

---

## Summary of Results After Fixes

| Test Case | Before Fixes | After Fixes |
|-----------|-------------|-------------|
| `1 coder fifo` | Worked (by accident) | Works correctly |
| `2 coders fifo` | Coder 2 burned out | **All coders compiled!** ✅ |
| `3 coders fifo` | Coder 1 burned out | **All coders compiled!** ✅ |
| `4 coders fifo` | Coder 1 burned out | **All coders compiled!** ✅ |
| `4 coders edf` | Coder 1 burned out | **All coders compiled!** ✅ |
| `5 coders fifo` | Coder 1 burned out | **All coders compiled!** ✅ |
| `6 coders edf` | Coder 1 burned out | **All coders compiled!** ✅ |
| `8 coders fifo` | N/A | **All coders compiled!** ✅ |
| `10 coders edf` | N/A | **All coders compiled!** ✅ |
| `10 coders fifo` | N/A | **All coders compiled!** ✅ |

### Validation Results
- ✅ **Valgrind Memcheck**: 0 memory leaks — all heap blocks freed
- ✅ **Valgrind Helgrind**: 0 data races detected
- ✅ **Compilation**: Clean with `-Wall -Wextra -Werror -pthread`
- ✅ **No burnout** across all test configurations
- ✅ **All coders** participate in compiling (no starvation)
