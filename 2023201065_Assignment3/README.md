# Operating System Scheduling and Tracing Algorithms

This project focuses on enhancing process scheduling algorithms and implementing a system tracing utility within an operating system. Below is a brief overview of the key components of the project:

## First-Come-First-Serve (FCFS)

- **Process Selection**: The FCFS algorithm schedules processes based on their creation order, looking through the process table for the earliest created process.

- **Initialization**: A variable within the `proc` structure stores the creation time of a process using the `allocproc` function.

- **Lock Acquisition**: In the scheduler function, a lock is acquired before evaluating the best process to run. If a better candidate is found, the lock is released before performing a context switch.

- **Preemption Disablement**: The `yield` function in `trap.c` is disabled to prevent preemption.

## Priority-Based Scheduling (PBS)

- **Priority Initialization**: Processes are assigned a static priority of 60, with additional variables tracking scheduling statistics.

- **Update Time Function**: The `update_time` function keeps track of run and sleep times for all processes in the process table.

- **Dynamic Priority**: A formula calculates dynamic priorities based on static priorities and recent behavior to select the next process to run.

## Multi-Level Feedback Queue (MLFQ)

- **Queue Framework**: A queue data structure and related functions are added to `proc.h` and implemented in `proc.c`.

- **Queue Initialization**: A 5-level queue system is initialized, with each queue representing a priority level.

- **Process Queueing**: The scheduler function adds RUNNABLE processes not in any queue to the appropriate queue.

- **Time Quantum Enforcement**: Processes exceeding their time quantum are moved to the next queue in the clock_interrupt function.

- **Aging and Priority Adjustment**: Aging is implemented by increasing priority if a process's wait time exceeds a threshold. Priority queues are adjusted accordingly.

- **Queue Behavior**: Processes relinquishing control before their timeslice completion remain in the same queue, promoting fairness.

## Strace

- **Syscall Creation**: A custom syscall for Strace is created.

- **Mask Storage**: An integer mask is added to the `struct proc` to store syscall tracing preferences.

- **Parent-to-Child Mask Copy**: The mask is copied from parent to child processes in `proc.c`.

- **Syscall Metadata**: Arrays store syscall names and argument counts for printing the respective information from the trapframe registers, including return values.

