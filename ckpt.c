#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <ucontext.h>
#include <unistd.h>

#include "ckpt.h"

int recover_flag = 0;

void parseMemRegion(char line[], struct MemoryRegion* mr) {
  char r, w, x, p;
  unsigned int offset;
  sscanf(line, "%p-%p %c%c%c%c %x", &mr->startAddr, &mr->endAddr, &r, &w, &x,
         &p, &offset);
  mr->isReadable = (r == 'r');
  mr->isWritable = (w == 'w');
  mr->isExecutable = (x == 'x');
}

void saveCkptImg() {
  ucontext_t context;
  if (getcontext(&context) == -1) handleError("get context");

  if (recover_flag == 0) {
    recover_flag = 1;

    FILE* fpCkpt = fopen(CKPT_PATH, "wb+");
    FILE* fpMem = fopen(MAP_PATH, "r");
    if (!fpCkpt || !fpMem) handleError("open file");

    fwrite(&context, sizeof(ucontext_t), 1, fpCkpt);

    char line[LINE_SIZE];
    struct MemoryRegion mr;
    while (fgets(line, LINE_SIZE, fpMem)) {
      parseMemRegion(line, &mr);
      if (mr.isReadable && !strstr(line, "vsyscall")) {
        fwrite(&mr, sizeof(struct MemoryRegion), 1, fpCkpt);
        fwrite(mr.startAddr, 1, mr.endAddr - mr.startAddr, fpCkpt);
      }
    }

    fclose(fpMem);
    fclose(fpCkpt);
    printf(" >> restored >> ");
  }
}

void sigusr2Handler(int signum) {
  if (signum == SIGUSR2) saveCkptImg();
}

__attribute__((constructor)) void myconstructor() {
  if (signal(SIGUSR2, sigusr2Handler) == SIG_ERR) handleError("handle signal");
}
