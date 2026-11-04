// expect: error invalid-free @ L18
// mode: c23, c++20
// twin: tests/memory/heap_malloc_free.c
// Pins: 0047 item 6 (trusted contract of free, 0046 item 9): the second free finds no token (a double free): invalid-free at the call, the first token row of the call
// Pre-registered from record 0047 item 6 (rows in order: offset, token, ...): invalid-free at the second free only; the first free is fine and the path ends silently at the failed token row; re-registered after audit round 1 (record 0047 item 6): the free's argument use check is replaced by the free rows: ONE invalid-free, no lifetime
#include <cerastes.h>
#include <stdlib.h>

int roundtrip(void)
    CERASTES_ENSURES(r, r == 7 || r == -1)
{
  int *p = (int *)malloc(sizeof(int));
  if (p == NULL) {
    return -1;
  }
  *p = 7;
  free(p);
  free(p);  // <- here
  return 7;
}
