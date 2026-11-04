// expect: rejected cerastes-unsupported_expression @ L11
// mode: c23, c++20
// twin: tests/memory/heap_malloc_free.c
// Pins: 0047 item 11: an allocation size other than sizeof(T) (0); re-expected by M4.5e2 (n * sizeof(T) and block contents)
// Pre-registered from record 0047 item 11: rejected unsupported_expression at the marked line
#include <cerastes.h>
#include <stdlib.h>

int f(void)
{
  int *p = (int *)malloc(0);  // <- here
  free(p);
  return 0;
}
