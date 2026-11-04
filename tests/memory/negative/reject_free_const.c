// expect: rejected cerastes-unsupported_expression @ L11
// mode: c23, c++20
// twin: tests/memory/heap_malloc_free.c
// Pins: 0047 item 11: free through a pointer to const: the typed view has no const object (M4.6)
// Pre-registered from record 0047 item 11: rejected unsupported_expression at the marked line (tag guessed: the nearest existing analogue)
#include <cerastes.h>
#include <stdlib.h>

int f(const int *p)
{
  free((void *)p);  // <- here
  return 0;
}
