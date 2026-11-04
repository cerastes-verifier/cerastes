// expect: rejected cerastes-unsupported_expression @ L11
// mode: c23, c++20
// twin: tests/memory/heap_malloc_free.c
// Pins: 0047 item 11: a discarded malloc result: the (A) result is not the direct operand of a void* to T* conversion (M4.7, M8)
// Pre-registered from record 0047 item 11: rejected unsupported_expression at the marked line (tag guessed: the nearest existing analogue)
#include <cerastes.h>
#include <stdlib.h>

int f(void)
{
  (void)malloc(sizeof(int));  // <- here
  return 0;
}
