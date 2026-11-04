// expect: error leak @ L21
// mode: c23, c++20
// twin: tests/memory/heap_leak_paths.c
// Pins: 0047 item 7: one branch frees and the other does not: the leak is reported at the function's closing brace
// Pre-registered from record 0047 item 7: leak at the closing brace, one error (the path that freed holds nothing); re-registered after audit round 1 (record 0047 item 7): closing brace
#include <cerastes.h>
#include <stdlib.h>

int branches(int c)
    CERASTES_ENSURES(r, r == 0 || r == 1 || r == -1)
{
  int *p = (int *)malloc(sizeof(int));
  if (p == NULL) {
    return -1;
  }
  if (c != 0) {
    *p = 1;
    free(p);
  }
  return 0;
}  // <- here
