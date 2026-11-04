// expect: error leak @ L21
// c-only: C converts a void* implicitly (initializer, assignment, return, argument); C++ needs a cast
// twin: tests/memory/heap_malloc_implicit.c
// Pins: 0047 items 2, 7: an owning result obtained through C's implicit conversion is held, and the caller returns without freeing it
// Pre-registered from record 0047 item 7: leak at the closing brace; re-registered after audit round 1 (record 0047 item 7): closing brace, not the return
#include <cerastes.h>
#include <stdlib.h>

CERASTES_OWNING int *make_zero(void)
{
  return calloc(1, sizeof(int));
}

int implicit_forms(void)
{
  int *z = make_zero();
  if (z == NULL) {
    return -1;
  }
  return 0;
}  // <- here
