<h1 align="center">Codexion</h1>

<p align="center">
  <strong>Concurrency simulation in C using POSIX threads, mutex synchronization, and priority scheduling.</strong><br>
  <em>Created as part of the 42 curriculum by bpasquer.</em>
</p>

<p align="center">
  <a href="#overview">Overview</a> •
  <a href="#features">Features</a> •
  <a href="#concurrency-model">Concurrency Model</a> •
  <a href="#scheduling-policies">Scheduling Policies</a> •
  <a href="#deadlock--starvation-prevention">Deadlock & Starvation</a> •
  <a href="#installation">Installation</a> •
  <a href="#usage">Usage</a> •
  <a href="#license">License</a>
</p>

<p align="center">
  <img src="https://img.shields.io/badge/C-C99-A8B9CC?style=flat-square&logo=c&logoColor=white" alt="C" />
  <img src="https://img.shields.io/badge/POSIX-pthreads-blue?style=flat-square" alt="POSIX pthreads" />
  <img src="https://img.shields.io/badge/Synchronization-Mutexes-success?style=flat-square" alt="Mutexes" />
  <img src="https://img.shields.io/badge/Scheduler-FIFO%20%26%20EDF-orange?style=flat-square" alt="Scheduler" />
  <img src="https://img.shields.io/badge/Build-Makefile-lightgrey?style=flat-square&logo=gnu&logoColor=white" alt="Makefile" />
  <img src="https://img.shields.io/badge/License-MIT-black?style=flat-square" alt="License" />
</p>

---

## Overview

**Codexion** is a high-performance multithreaded systems simulation written in C99. Built around an advanced variation of Edsger Dijkstra's classic *Dining Philosophers Problem*, it models a circular co-working hub where developers compete for a constrained pool of shared USB compilation dongles.

In this environment:
- Several coders sit around a circular table. Between each adjacent pair of coders lies a shared hardware compilation dongle.
- To **compile**, a coder must simultaneously secure both their **left** and **right** adjacent dongles.
- Coders transition cyclically through three distinct states:
  $$\text{Compiling} \longrightarrow \text{Debugging} \longrightarrow \text{Refactoring}$$
- When releasing dongles, each dongle enters a mandatory **hardware cooldown** state during which neither adjacent coder can re-acquire it.
- A coder **burns out** if they do not begin compiling before their deadline:
  $$\text{Deadline} = \text{last\_compile\_start} + \text{time\_to\_burnout}$$
- Access to dongles is orchestrated by pluggable priority schedulers: **FIFO** (*First-In-First-Out*) and **EDF** (*Earliest Deadline First*).

The simulation terminates under two conditions:
1. **Fatal burnout**: A coder fails to secure both dongles before their burnout threshold.
2. **Successful completion**: Every coder completes at least `number_of_compiles_required` compilation cycles without any burnout occurring.

---

## Features

- **POSIX Multithreading (`pthreads`)**: Each coder executes on an isolated worker thread alongside an asynchronous supervisor thread.
- **Granular Mutex Synchronization**: Exclusive, contention-minimized locking over shared resources without busy-waiting.
- **Pluggable Priority Schedulers**:
  - `fifo`: Deterministic arrival-order queuing.
  - `edf`: Dynamic priority arbitration favoring the coder nearest their burnout threshold.
- **Custom Binary Heap**: High-performance priority min-heap ($O(\log N)$ push/pop) powering scheduler arbitration.
- **Per-Dongle Cooldown Windows**: Hardware cooldown timers tied directly to the physical dongle resources rather than thread sleeping.
- **Coffman Deadlock Immunity**: Architectural deadlock prevention eliminating circular wait and partial hold-and-wait conditions.
- **Real-Time Burnout Supervision**: Microsecond-accurate monitor thread tracking coder health and stopping the simulation instantly on failure.
- **Thread-Safe Log Serialization**: Lock-protected atomic logging ensuring stdout lines never interleave.
- **Clean Architecture & 42 Norm Compliant**: Zero memory leaks, zero data races, structured strictly according to 42 C standards.

---

## Concurrency Model

### Circular Topology

Coders are arranged in a closed ring. Dongle $i$ is situated between Coder $i$ and Coder $(i + 1) \pmod N$:

```text
                     [ Dongle 0 ]
               Coder 0          Coder 1
           [ Dongle 4 ]         [ Dongle 1 ]
          Coder 4                 Coder 2
               [ Dongle 3 ]     [ Dongle 2 ]
                     Coder 3
```

### State Machine Lifecycle

Each coder alternates continuously through the following state transitions:

```text
       +-----------------------------------------------+
       |                                               |
       v                                               |
+--------------+    Acquire Dongles L & R       +--------------+
|   WAITING    | =============================> |  COMPILING   |
| (Heap Queue) |                                |  (Dual Lock) |
+--------------+                                +--------------+
                                                       |
                                                       | Release Dongles
                                                       | (Trigger Cooldown)
                                                       v
+--------------+                                +--------------+
| REFACTORING  | <============================= |  DEBUGGING   |
+--------------+                                +--------------+
       |                                               ^
       +-----------------------------------------------+
```

### Mutex Synchronization Hierarchy

Shared resources are isolated behind fine-grained `pthread_mutex_t` primitives to ensure race-free execution:

| Mutex | Scope | Protected Resource |
|---|---|---|
| `dongle->mutex` | Per Dongle | Hardware availability flag and cooldown expiration timestamp |
| `coder->last_compile_mutex` | Per Coder | `last_compile_time` and completed `compile_count` |
| `heap->mutex` | Global Scheduler | Binary heap array and priority queue ordering |
| `data->stop_mutex` | Global Simulation | Simulation termination flag (`data->stop`) |
| `data->log_mutex` | Global I/O | Standard output stream serialization |

---

## Scheduling Policies

Dongle contention is managed through a central priority queue implemented as an in-memory binary min-heap.

### 1. FIFO (First-In, First-Out)
- **Arbitration Metric**: Timestamp of when the coder requested access to the dongles.
- **Behavior**: Requests are queued strictly in arrival order. A coder who entered the waiting state earlier will always be evaluated before newer requests.

### 2. EDF (Earliest Deadline First)
- **Arbitration Metric**: Calculated burnout deadline:
  $$\text{Priority Key} = \text{last\_compile\_start} + \text{time\_to\_burnout}$$
- **Behavior**: The scheduler dynamically prioritizes the coder who is physically closest to burning out. This policy significantly improves throughput and survival rates under tight timing constraints.

```text
                Min-Heap Priority Queue
                         [ Coder 3 ] (Deadline: 780ms)
                        /           \
     [ Coder 1 ] (Deadline: 820ms)   [ Coder 4 ] (Deadline: 850ms)
```

---

## Deadlock & Starvation Prevention

Concurrency hazards are resolved by addressing the **Coffman Conditions**:

1. **Mutual Exclusion**: Preserved by necessity—a physical USB dongle cannot be split between two machines simultaneously.
2. **Hold and Wait**: Eliminated by atomic resource validation. A coder never holds one dongle while waiting indefinitely for the second. The scheduler verifies that *both* adjacent dongles are available and clear of cooldown before committing acquisition.
3. **No Preemption**: Mutual exclusion ownership cannot be forcibly revoked; locks are voluntarily released upon compilation completion.
4. **Circular Wait**: Broken by priority-directed arbitration. Because resource allocation is driven by the min-heap order rather than unilateral thread grabbing, circular waiting chains cannot form.

### Starvation Mitigation
Under both `fifo` and `edf`, coders cannot be starved indefinitely by neighboring threads:
- In `fifo`, the arrival key guarantees eventual service.
- In `edf`, as time advances without compiling, an unserved coder's remaining time decreases, driving their priority toward the root of the heap.

---

## Installation

### Prerequisites
- POSIX-compliant operating system (Linux, macOS)
- `gcc` or `clang` with POSIX Threads library (`-pthread`)
- `make`

### Building the Project

Clone the repository and compile using the provided `Makefile`:

```bash
git clone https://github.com/baptiste1307/Codexion.git
cd Codexion
make
```

### Build Targets

```bash
make        # Compile the codexion binary
make clean  # Remove intermediate object files (.o)
make fclean # Remove object files and the compiled binary
make re     # Clean rebuild from scratch
```

---

## Usage

### Syntax

```bash
./codexion <number_of_coders> \
           <time_to_burnout> \
           <time_to_compile> \
           <time_to_debug> \
           <time_to_refactor> \
           <number_of_compiles_required> \
           <dongle_cooldown> \
           <scheduler>
```

### Argument Reference

| Argument | Type | Unit | Description |
|---|---|---|---|
| `number_of_coders` | `int` | Count | Number of coders and identical number of shared dongles |
| `time_to_burnout` | `int` | Milliseconds | Max time without starting a compile before burnout occurs |
| `time_to_compile` | `int` | Milliseconds | Time spent compiling (holds 2 dongles) |
| `time_to_debug` | `int` | Milliseconds | Time spent debugging after compilation |
| `time_to_refactor` | `int` | Milliseconds | Time spent refactoring before next compile request |
| `number_of_compiles_required` | `int` | Count | Target compile count per coder to trigger clean exit |
| `dongle_cooldown` | `int` | Milliseconds | Mandatory rest period for a dongle after release |
| `scheduler` | `string` | — | Arbitration policy: `fifo` or `edf` |

### Output Format

Events are streamed in real time to `stdout` with millisecond precision:

```text
<timestamp_in_ms> <coder_id> <action>
```

Possible actions:
- `has taken a dongle`
- `is compiling`
- `is debugging`
- `is refactoring`
- `burned out`

### Simulation Examples

#### 1. Successful Run (Everyone Completes Cycles)

```bash
./codexion 4 600 100 100 100 2 0 edf
```

```text
0 1 has taken a dongle
0 1 has taken a dongle
0 1 is compiling
0 3 has taken a dongle
0 3 has taken a dongle
0 3 is compiling
100 1 is debugging
100 3 is debugging
200 1 is refactoring
200 3 is refactoring
300 2 has taken a dongle
300 2 has taken a dongle
300 2 is compiling
...
```

#### 2. Burnout Scenario (Contention Exceeds Burnout Threshold)

```bash
./codexion 5 800 200 200 200 2 10 edf
```

```text
0 1 has taken a dongle
0 1 has taken a dongle
0 1 is compiling
0 3 has taken a dongle
0 3 has taken a dongle
0 3 is compiling
200 1 is debugging
200 3 is debugging
400 1 is refactoring
400 3 is refactoring
610 2 has taken a dongle
610 2 has taken a dongle
610 2 is compiling
802 1 burned out
```

---

## Technical Details

- **Time Measurement**: Uses `gettimeofday()` converted to relative milliseconds from the simulation epoch.
- **Monitor Thread**: A detached supervisor periodically sweeps all coder `last_compile_time` records under mutex locking, guaranteeing immediate simulation teardown upon burnout.
- **No Global Variables**: All simulation context is encapsulated within clean, heap-allocated or stack-allocated data structures passed via thread arguments.

---

## License

This project is licensed under the [MIT License](LICENSE) - see the [LICENSE](LICENSE) file for details.
