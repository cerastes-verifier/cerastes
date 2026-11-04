// expect: error leak @ L22
// mode: c23, c++20
// twin: tests/memory/heap_leak_paths.c
// Pins: 0047 item 7: a path returns with the allocation held (no free on it): one leak row per function, reported at its closing brace
// Pre-registered from record 0047 item 7 (the leak row follows the end label): leak at the function's closing brace; re-registered after audit round 1 (record 0047 item 7): the row is at the closing brace, not at the return
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
    return 1;
  }
  free(p);
  return 0;
}  // <- here
