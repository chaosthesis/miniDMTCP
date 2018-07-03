#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <ucontext.h>
#include <unistd.h>
#include "ckpt.h"

#define STACK_ADDR 0x530000
#define STACK_SIZE 0x1000

char ckpt_image[1000];

void parseMemRegion(char line[], struct MemoryRegion *mr) {
  char r, w, x, p;
  unsigned int offset;
  sscanf(line, "%p-%p %c%c%c%c %x", &mr->startAddr, &mr->endAddr, &r, &w, &x,
         &p, &offset);
  mr->isReadable = (r == 'r');
  mr->isWritable = (w == 'w');
  mr->isExecutable = (x == 'x');
}

int getMemPerm(struct MemoryRegion *mr) {
  int perm = PROT_WRITE;
  if (mr->isReadable) perm |= PROT_READ;
  if (mr->isExecutable) perm |= PROT_EXEC;
  return perm;
}

void restoreMemRegion(int fpCkpt) {
  struct MemoryRegion mr;
  while ((read(fpCkpt, &mr, sizeof(struct MemoryRegion)))) {
    mr.startAddr =
        mmap(mr.startAddr, mr.endAddr - mr.startAddr, getMemPerm(&mr),
             MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0);
    if (mr.startAddr == MAP_FAILED) handleError("mmap");

    if (read(fpCkpt, mr.startAddr, mr.endAddr - mr.startAddr) == -1)
      handleError("restore data");
  }
}

void restoreCkpt() {
  ucontext_t context;
  int fpCkpt = open(ckpt_image, O_RDONLY);
  if (fpCkpt == -1) handleError("open image");
  if (read(fpCkpt, &context, sizeof(ucontext_t)) == -1)
    handleError("restore context");

  restoreMemRegion(fpCkpt);

  close(fpCkpt);
  setcontext(&context);
}

void unmapCurrentStack() {
  struct MemoryRegion mr;
  FILE *fpMem = fopen(MAP_PATH, "r");
  char line[LINE_SIZE];
  while (fgets(line, LINE_SIZE, fpMem)) {
    if (strstr(line, "[stack]")) {
      parseMemRegion(line, &mr);
      fclose(fpMem);
      break;
    }
  }
  munmap(mr.startAddr, mr.endAddr - mr.startAddr);
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("Usage: myrestart CKPT_IMG\n");
    return -1;
  }
  strcpy(ckpt_image, argv[1]);
  void *stack_ptr = mmap((void *)STACK_ADDR, STACK_SIZE, PROT_READ | PROT_WRITE,
                         MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0);

  if (stack_ptr == MAP_FAILED) handleError("mmap");
  stack_ptr += STACK_SIZE;

  asm volatile("mov %0,%%rsp" ::"g"(stack_ptr) : "memory");

  unmapCurrentStack();
  restoreCkpt();
}
