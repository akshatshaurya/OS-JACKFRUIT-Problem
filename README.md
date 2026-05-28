# OS Jackfruit Mini Container Runtime

## Problem Statement

This project implements a lightweight container runtime using Linux operating system concepts such as process creation, inter-process communication, filesystem isolation, namespaces, and cgroups.

The objective is to understand how modern container technologies like Docker work internally using low-level Linux primitives.

## Date

28 May 2026

---

# Features Implemented

* Process creation using `fork()`
* Program execution using `exec()`
* Supervisor-client communication using UNIX domain sockets
* Filesystem isolation using `chroot()`
* Namespace-based process isolation
* Memory management using Linux cgroups
* Workload programs for CPU, memory, and I/O testing

---

# OS Concepts Used

| Concept      | Usage                             |
| ------------ | --------------------------------- |
| fork()       | Creates container process         |
| exec()       | Runs program inside container     |
| UNIX sockets | IPC between supervisor and client |
| chroot()     | Filesystem isolation              |
| Namespaces   | Process isolation                 |
| cgroups      | Resource and memory control       |

---

# Files Included

| File         | Description             |
| ------------ | ----------------------- |
| engine.c     | Main container runtime  |
| memory_hog.c | Memory stress testing   |
| cpu_hog.c    | CPU stress testing      |
| io_pulse.c   | I/O workload generation |
| Makefile     | Build automation        |
| README.md    | Project documentation   |

---

# Build Instructions

```bash
make clean
make
```

---

# Running the Supervisor

```bash
sudo ./engine supervisor ../rootfs-base
```

This starts the supervisor process which listens for container creation requests.

---

# Starting a Container

```bash
sudo ./engine start alpha ../rootfs-alpha /bin/sh
```

This creates a containerized shell environment.

---

# Cgroup Setup

Enable memory controller:

```bash
sudo sh -c 'echo +memory > /sys/fs/cgroup/cgroup.subtree_control'
```

Create cgroup:

```bash
sudo mkdir -p /sys/fs/cgroup/mycontainer
```

Set memory limit:

```bash
echo $((30*1024*1024)) | sudo tee /sys/fs/cgroup/mycontainer/memory.max
```

---

# Memory Monitoring

```bash
watch -n 1 cat /sys/fs/cgroup/mycontainer/memory.current
```

This command continuously displays memory usage of the container.

---

# Running Memory Stress Test

Inside container:

```bash
/bin/memory_hog
```

This continuously allocates memory to test cgroup enforcement.

---

# Challenges Faced

* Correctly configuring cgroup v2 memory controllers
* Attaching container processes to cgroups
* Handling socket communication between supervisor and client
* Managing isolated filesystem environments using chroot

---

# Conclusion

This project demonstrates the fundamental principles behind modern container runtimes such as Docker. Using Linux primitives like fork, exec, chroot, namespaces, and cgroups, we successfully implemented process isolation and resource control in a lightweight container environment.
