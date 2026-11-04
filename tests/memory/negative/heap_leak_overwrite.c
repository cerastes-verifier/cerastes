// expect: error leak @ L18
// mode: c23, c++20
// twin: tests/memory/heap_leak_paths.c
// Pins: 0047 item 7: the first block is lost when p is overwritten: its token is still held at the function's end (a leak is about the token, not about a pointer to it)
// Pre-registered from record 0047 item 7: leak at the closing brace, one error (both paths through the overwrite hold the first block; the identical reports are one error); re-registered after audit round 1 (record 0047 item 7): closing brace
#include <cerastes.h>
#include <stdlib.h>

int replace(void)
    CERASTES_ENSURES(r, r == 0)
{
  int *p = (int *)malloc(sizeof(int));
  if (p != NULL) {
    p = (int *)malloc(sizeof(int));
    free(p);
  }
  return 0;
}  // <- here
