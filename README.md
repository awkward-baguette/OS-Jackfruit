# Multi-Container Runtime (Tasks 4, 5, 6)

This project implements a kernel-space memory monitor and scheduling experiments as part of a lightweight container runtime.

---

## Task 4: Kernel Memory Monitoring

We implemented a Linux Kernel Module (`monitor.c`) to track container processes and enforce memory limits.

### Features

- Device created at `/dev/container_monitor`
- Supervisor registers container PIDs using `ioctl`
- Kernel maintains a linked list of monitored containers
- Thread-safe list access using mutex
- Periodic monitoring using a timer

### Memory Monitoring

- RSS (Resident Set Size) is measured using `get_mm_rss()`
- Memory usage is calculated in bytes

### Policies

**Soft Limit**
- Logs a warning when memory exceeds the soft limit
- Triggered only once per container

**Hard Limit**
- Process is killed using `SIGKILL` when hard limit is exceeded
- Entry is removed from the list

### Additional Behavior

- Automatically removes entries for exited processes
- Supports register and unregister using ioctl
- Uses safe list iteration to avoid race conditions

---

## Task 5: Scheduler Experiments

We implemented two workloads to analyze Linux scheduling behavior.

### Workloads

**1. CPU-bound workload (`cpu_hog.c`)**
- Performs continuous computation
- Fully utilizes CPU

**2. I/O-bound workload (`io_pulse.c`)**
- Writes to a file and sleeps
- Simulates I/O-heavy behavior

---

### Experiment 1: CPU vs CPU (Different Priorities)

Two CPU-bound containers were executed with different priorities:

- Container A: nice = -5 (higher priority)
- Container B: nice = 10 (lower priority)

**Observation:**
- Container A received more CPU time
- Container B executed slower

---

### Experiment 2: CPU vs I/O

A CPU-bound and I/O-bound container were run simultaneously.

**Observation:**
- CPU-bound process consumed most CPU
- I/O-bound process remained responsive due to sleep intervals

---

### Conclusion

Linux uses the **Completely Fair Scheduler (CFS)**:
- Ensures fair CPU allocation
- Prioritizes lower nice values
- Maintains responsiveness for I/O-bound workloads

---

## Task 6: Resource Cleanup

The system ensures proper cleanup in both kernel and user space.

### Kernel Cleanup

- Timer is stopped using `del_timer_sync`
- All linked list entries are removed and freed using `kfree`
- Device `/dev/container_monitor` is destroyed properly
- No memory leaks remain after module unload

---

### User-Space Cleanup

- Supervisor uses `waitpid` to prevent zombie processes
- Logging threads terminate cleanly
- File descriptors are properly closed

---

### Result

- No zombie processes remain
- No memory leaks in kernel or user space
- Clean shutdown of all components
