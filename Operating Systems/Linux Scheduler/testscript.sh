#!/bin/bash
#Shambhavi Srivastava
# Wall = %e, User = %U, System= %S,
# CPU  = %P, Involuntary context switches = %c,
# Voluntary context switches = %w


TIMEFORMAT="%e,%U,%S,%P,%c,%w"
MAKE="make -s"
schedulers[1]=SCHED_OTHER
schedulers[2]=SCHED_FIFO
schedulers[3]=SCHED_RR
size[1]=SMALL
size[2]=MEDIUM
size[3]=LARGE
process[1]=CPU
process[2]=IO
process[3]=MIXED
#NICE_VAL="-7"
RESULT_FILE="/home/user/Desktop/Lab4/data/results.csv"
RESULT_FILE_NICE="/home/user/Desktop/Lab4/data/resultsNice.csv"
echo Building code...
$MAKE clean
$MAKE
x=1
while [ $x -le 100 ]
do
  echo "Creating rwinput-$x"
  dd if=/dev/urandom of=./rwinput-$x bs=$INPUTBLOCKSIZEBYTES count=$INPUTBLOCKS > /dev/null 2>&1
  x=$(( $x + 1 ))
done


#Echo out the header file for the CSV - overwrite any existing benchmarks
echo "\"Process Type\",\"Scheduler Type\",Iterations,\"Num Processes\",Wall,User,System,CPU,I-Switched,V-switched,Priority" > "$RESULT_FILE"

echo Starting test runs...
echo
echo cpu-process tests...
echo

for l in 1 2 3
do
for k in 1 2 3
do
for j in 1 2 3
do
for i in 1 2 3 4 5
do
/usr/bin/time -f "$TIMEFORMAT" -o "$RESULT_FILE" -a sudo ./scheduler ${schedulers[j]} ${size[k]} ${process[l]}  > /dev/null
done
echo Running "${process[l]}"-process with "${schedulers[j]}" and "${size[k]}" simultaneous processes...
echo
done
done
done

for l in 1 2 3
do
for k in 1 2 3
do
for j in 1 2 3
do
for i in 1 2 3 4 5
do
/usr/bin/time -f "$TIMEFORMAT" -o "$RESULT_FILE_NICE" -a sudo nice -n $[RANDOM%-20+19] ./scheduler ${schedulers[j]} ${size[k]} ${process[l]}  > /dev/null
done
echo Running "${process[l]}"-process with "${schedulers[j]}" and "${size[k]}" simultaneous processes...
echo
done
done
done


$MAKE clean
