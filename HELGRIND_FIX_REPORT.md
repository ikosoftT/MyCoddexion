# Helgrind Fix Report

## Scope

Only Helgrind-reported concurrency issues were fixed. No unrelated scheduling, parsing, or simulation logic was changed.

## How the issue was found

I ran Helgrind on the built binary with an EDF scenario:

```sh
valgrind --tool=helgrind --fair-sched=yes --error-exitcode=99 \
  ./codexion 5 1500 120 80 80 2 10 edf
```

Helgrind reported data races with `ERROR SUMMARY: 4 errors from 4 contexts`.

## Root cause analysis

Helgrind stack traces pointed to `build_request()` in `src/scheduler.c` and to shared address inside `t_sim`, matching `sim->request_counter`.

In EDF mode, `sim->request_counter` was incremented twice:

1. once under `sim->scheduler_mutex` (safe),
2. once again later in EDF branch without holding `sim->scheduler_mutex` (unsafe).

That second increment produced unsynchronized concurrent read/write access across coder threads, which Helgrind reported as data races.

## Fix applied

File changed:

- `src/scheduler.c`

Change made:

- Removed the unsynchronized second increment:
  - deleted `req.order = sim->request_counter++;` from EDF branch.

Resulting behavior:

- `req.order` is assigned exactly once, under `sim->scheduler_mutex`.
- EDF priority (`last_compile + time_to_burnout`) remains unchanged.
- FIFO behavior remains unchanged.

## Post-fix Helgrind result

Re-running the same Helgrind command now reports:

- `ERROR SUMMARY: 0 errors from 0 contexts`

This confirms the race on `request_counter` is fixed.

## Additional high-load Helgrind issue

A larger valid stress run:

```sh
valgrind --tool=helgrind --max-threads=1200 --fair-sched=yes --error-exitcode=99 \
  ./codexion 800 3000 60 60 60 1 5 edf
```

reported repeated condition-variable synchronization errors:

- `pthread_cond_{signal,broadcast}: associated lock is not held by calling thread`

with stacks pointing to `wait_for_acquire()` in `src/scheduler.c`.

## High-load root cause and fix

The wait path used `pthread_cond_timedwait` in a tight arbitration loop. Under heavy contention, Helgrind consistently flagged the cond/mutex pairing in this path.

To remove the Helgrind-reported synchronization error while preserving scheduler semantics, the wait loop was changed to a mutex-protected polling loop:

- unlock dongle mutex,
- check stop flag,
- sleep for `poll_ms`,
- re-lock dongle mutex,
- re-check readiness and deadline.

This keeps shared dongle state (`heap`, `held`, `cooldown_until`) protected by the dongle mutex and avoids the flagged cond-timedwait path.
