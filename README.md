# CMPSCI-4760 P2

## Project Goal:

* Getting familiar with concurrent processing in Linux using shared memory.
* Solving the concurrency issues using the Bakery algorithm for synchronization of processes.

## How to run
Commands to run project from hoare server:

```
$ make
$ ./master -t ss n
```
Here, ss = termination time, n = maximum number of processes.

## Git

https://github.com/SanzidaH/SanzidaH-CMPSCI4760-p2.git

## Task Summary

> Master will spawn fan of child processes.
> Slave will process these processes and these processes will attempt to write on same shared segment (entering critical section) creating concurrency issue.
> This problem of mutual exclusion will be solved using Bakery algorithm.
