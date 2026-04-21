# Multi-Container Runtime 

This project implements a kernel-space memory monitor and scheduling experiments as part of a lightweight container runtime.

---
## Task 1: Environment and Core Structures
This stage focused on establishing the foundational environment and the internal data structures required to track isolated processes.

### Features
* **Environment Setup:** Utilized Oracle VirtualBox with Ubuntu 24.04 and Alpine Linux minirootfs to create a lightweight, isolated filesystem.
* **Internal Metadata:** Implemented a `container_record_t` structure to track essential container state:
    * **Container ID:** Human-readable nickname (e.g., alpha, beta).
    * **Host-side PID:** The unique process identifier assigned by the Linux kernel.
    * **Execution Status:** Tracks lifecycle states including `Running`, `Exited`, and `Killed`.
    * **Resource Configuration:** Stores soft and hard memory limits for enforcement.
* **Thread-Safe Management:** Utilized a linked list guarded by **pthread mutex locks** to manage active containers safely within the supervisor’s memory.

---

## Task 2: Multi-Container Supervision & IPC
We implemented the primary supervisor-client architecture, allowing for the concurrent management of multiple containers through robust Inter-Process Communication (IPC).

### Supervision Logic
* **Double Forking:** Implemented a `fork()` and `execve()` flow to spawn containerized processes within their own unique root filesystems.
* **Unix Domain Sockets:** Established the primary IPC mechanism via a socket (located at `/tmp/mini_runtime.sock`), enabling the CLI tool to communicate with the background supervisor.
* **Command Protocol:**
    * `start`: Instructs the supervisor to spawn a new container.
    * `ps`: Queries the internal linked list to retrieve real-time container metadata.
    * `stop/logs`: Manages the lifecycle and output retrieval of specific containers.

### Namespace & Isolation
* **Mount Isolation:** Used `chroot` logic to isolate the container’s view of the filesystem to its specific `rootfs`.
* **Proc Management:** Automatically handled the mounting and unmounting of the `/proc` filesystem within each container, ensuring system utilities like `top` only display container-local information.

---

## Task 3: Bounded-Buffer Logging Pipeline
The system was equipped with a high-performance logging pipeline to capture and persist container output without blocking the execution of the supervisor.

### Pipeline Architecture
* **Redirection:** Used `pipe()` and `dup2()` to redirect container `stdout` and `stderr` back to the supervisor process.
* **Bounded Buffer:** Implemented a thread-safe, circular buffer (Producer-Consumer model) to handle log entries:
    * **Producer:** A dedicated thread for each container reads from the pipe and "pushes" data into the buffer.
    * **Consumer:** A central `logging_thread` "pops" data from the buffer and writes it to the `logs/` directory.
* **Synchronization:** Utilized **POSIX semaphores** (`sem_wait`, `sem_post`) and condition variables to manage buffer access, preventing race conditions or memory overflows during high-volume logging.

### Result
* **Asynchronous I/O:** Container logging is decoupled from management tasks, preventing I/O bottlenecks.
* **Persistence:** All container activity is preserved in individual `.log` files for post-execution analysis.


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
