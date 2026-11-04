// expect: error null-dereference @ L12
// mode: c23, c++20
// twin: tests/vlibc/malloc_explicit.c
// Pins: 0047 item 1 (0046 item 9): a twin of malloc's trusted ensures: it promises `r == NULL || owns_block(r)`, not a non-null result
// Pre-registered from record 0047 item 1: null-dereference at the write
#include <vlibc/stdlib.h>
#include <cerastes.h>

int roundtrip(void)
{
  int *p = (int *)malloc(sizeof(int));
  *p = 7;  // <- here
  free(p);
  return 7;
}
