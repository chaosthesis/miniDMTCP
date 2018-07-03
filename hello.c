#include <stdio.h>
#include <unistd.h>

//int count = 0;
int main() {
  while (1) {
    printf(".");
    //printf("%d ", count++);
    fflush(stdout);
    sleep(1);
  }
  return 0;
}