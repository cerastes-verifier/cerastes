// expect: error leak @ L17
// mode: c23, c++20
// twin: tests/memory/heap_exit_holds.c
// Pins: 0047 item 7 (question 123): main is checked like every function: blocks it holds when it returns are leaks (this differs on purpose from C23 5.1.2.3.4, where returning from main is exit)
// Pre-registered from record 0047 item 7 and question 123: leak at main's closing brace; re-registered after audit round 1 (record 0047 item 7): closing brace, not the return
#include <cerastes.h>
#include <stdlib.h>

int main(void)
{
  int *p = (int *)malloc(sizeof(int));
  if (p == NULL) {
    exit(1);
  }
  *p = 3;
  return 0;
}  // <- here
