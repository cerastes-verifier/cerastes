// expect: rejected cerastes-unsupported_expression @ L13
// (glibc declares malloc's result-use attribute only under _FORTIFY_SOURCE, so EDG gives no warning here;
// the macOS sibling reject_malloc_discarded.c lists the SDK's two warnings)
// mode: c23, c++20; platform: linux
// twin: tests/memory/heap_malloc_free.c
// Pins: 0047 item 11 (EDG warns twice that the result of the (void) cast and of the call is not used: a discarded result cannot be written without them): a discarded malloc result: the (A) result is not the direct operand of a void* to T* conversion (M4.7, M8)
// Pre-registered from record 0047 item 11: rejected unsupported_expression at the marked line (tag guessed: the nearest existing analogue)
#include <cerastes.h>
#include <stdlib.h>

int f(void)
{
  (void)malloc(sizeof(int));  // <- here
  return 0;
}
