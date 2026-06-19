# xv6 — `suicide_bomb` System Call

An extended version of the [MIT xv6-riscv](https://github.com/mit-pdos/xv6-riscv) teaching OS, modified to support a kernel-enforced, hardware-backed self-destruct timer for user-space processes.

---

## Overview

The `suicide_bomb(int ticks)` system call arms a per-process countdown monitored directly inside the supervisor-mode timer interrupt handler. When the countdown expires, the process terminates itself and cascades immediate termination to all of its direct children — cleanly, without kernel deadlocks.

---

## Features

| Feature                     | Description                                                                                                                  |
| --------------------------- | ---------------------------------------------------------------------------------------------------------------------------- |
| **Kernel-backed countdown** | Countdown is decremented inside the timer interrupt vector (`which_dev == 2`) — no user-space polling                        |
| **Cascading termination**   | On expiry, the process and all direct children are killed via a dedicated `kill_children` routine with correct lock ordering |
| **Fork isolation**          | `bomb_armed` and `bomb_ticks` fields are zeroed during `fork()` — child processes never inherit a parent's countdown         |
| **Disarm / override**       | Call with `0` ticks to disarm; call again with any positive value to override the active countdown                           |
| **Safety boundary checks**  | Negative arguments return `-1` immediately without modifying process state                                                   |

---

## Implementation

### `kernel/proc.h` & `kernel/proc.c`

- Added `bomb_armed` and `bomb_ticks` fields to `struct proc`
- Initialized to clean state in `allocproc`
- Zeroed in `fork` to enforce child isolation
- `kill_children`: walks the process table and marks direct children with `p->killed = 1`, waking them if sleeping

### `kernel/trap.c`

- Inside the timer interrupt branch (`which_dev == 2`), decrements `bomb_ticks` for any armed process
- On expiry, sets `p->killed = 1` and calls `kill_children` — deferred to context-switch exit to avoid holding locks at kill time

### `kernel/sysproc.c`

- `sys_suicide_bomb`: extracts and validates the `ticks` argument, rejects negatives with `-1`, otherwise arms (or disarms/overrides) the countdown

---

## Building & Running

> **Requirements:** RISC-V `newlib` toolchain and QEMU compiled for `riscv64-softmmu` must be in your `PATH`.

```bash
# 1. Clean the build
make clean

# 2. Compile and boot
make qemu
```

---

## Test Suite

Two test programs are included: `bomb_scenarios` (multi-scenario) and `bombTest` (parent/child).

```
$ bomb_scenarios a       # Scenario A: standard timeout detonation
$ bomb_scenarios b       # Scenario B: computation followed by clean disarm
$ bomb_scenarios neg     # Validates negative-argument rejection
$ bomb_scenarios double  # Validates mid-run countdown override
$ bomb_scenarios fork    # Validates child process lifecycle immunity
$ bomb_scenarios multi   # Multi-process concurrent stress test
$ bombTest               # Process with a child — isolation verification
```

---

## System Call Interface

```c
int suicide_bomb(int ticks);
```

| Argument     | Behavior                                                      |
| ------------ | ------------------------------------------------------------- |
| `ticks > 0`  | Arms (or overrides) the countdown to `ticks` timer interrupts |
| `ticks == 0` | Disarms any active countdown                                  |
| `ticks < 0`  | Returns `-1` (invalid argument)                               |

On expiry, the calling process and all direct children receive `SIGKILL`-equivalent termination via `p->killed = 1`.

---

## Project Report

Detailed technical analysis, design trade-offs, lock ordering strategy, and validation results are available in the project report.

- **View Report:** [Report.pdf](Report.pdf)

## Based On

[xv6-riscv](https://github.com/mit-pdos/xv6-riscv) — MIT PDOS  
Original xv6 book and source: https://pdos.csail.mit.edu/6.828/
