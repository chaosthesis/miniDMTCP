# A Mini-DMTCP

## Overview
   
This assignment requires building a mini-DMTCP, which is useful for checkpoint-
restart. The complete package includes:
   
- hello: a trvial program;
- libckpt.so: a shared library which enables a program to handle user's signal
  and build checkpoint;
- restart: restore program status/data from a existing checkpoint.
   
The detailed problem description can be found at:
http://www.ccs.neu.edu/home/kapil/courses/cs5600sp18/hw2.html

## Problem description

DMTCP is a widely used software package for checkpoint-restart: http://dmtcp.sourceforge.net.

You will write a shared library, *libckpt.so*. Given an arbitrary application in C or C++, you will then run it with this library with the following command-line:

```
LD_PRELOAD=/path/to/libckpt.so ./myprog
```

where /path/to/libckpt.so must be an absolute pathname for the *libckpt.so*.

## Saving a checkpoint image file

- Your code should read /proc/self/maps (which is the memory layout of the current process).
- Your code should then save that information in a checkpoint image.
- You should save a file header, to restore information about the entire process. This should include saving the registers. (See below on how to save and restore register values.)
- For each memory section, you should save information that you will need to later restore the memory:
	- a section header (including address range of that memory, and r/w/x permissions)
	- the data in that address range
- We need a simple way to trigger the checkpoint. Write a signal handler for SIGUSR2, that will initiate all of the previous steps. To trigger a checkpoint, you now just need to do from a command line: kill -12 PID 
- The signal handler will only help you if you can call signal() to register the signal handler. We must write the call to signal() inside libckpt.so. But it must be called before the main() routine of the end user. The solution is to define a constructor function in C inside libckpt.so. For the details of how to do this, see, below: "Declaring constructor functions"

# Restoring from a checkpoint image file

1. Create a file, myrestart.c, and compile it statically at an address that the target process is not likely to use.
For example:
```
 gcc -static \
        -Wl,-Ttext-segment=5000000 -Wl,-Tdata=5100000 -Wl,-Tbss=5200000 \
        -o myrestart myrestart.c 
```

2. When myrestart begins executing, it will need to move its stack to some infrequently used address, like 0x5300000. 
