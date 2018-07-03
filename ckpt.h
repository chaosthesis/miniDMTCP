#ifndef CKPT_H__
#define CKPT_H__

#define MAP_PATH "/proc/self/maps"
#define CKPT_PATH "./myckpt"

#define LINE_SIZE 256

#define handleError(msg) \
  do {                   \
    perror(msg);         \
    exit(EXIT_FAILURE);  \
  } while (0)

struct MemoryRegion {
  void *startAddr;
  void *endAddr;
  int isReadable;
  int isWritable;
  int isExecutable;
};

#endif
