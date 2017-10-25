Installation:

Make sure you are in the same working directory as the program.
RunL $make to compile the program

Usage:

$./testscript.sh

This program will run all 54 combinations of tests for the schedulers, processes
and the number of processes.

./scheduler <Scheduler> <#Processes> <ProcessType>

Options:
<Scheduler>: SCHED_FIFO, SCHED_OTHER, SCHED_RR
<#Processes>: SMALL, MEDIUM, LARGE
<ProcessType>:CPU, IO, MIXED
