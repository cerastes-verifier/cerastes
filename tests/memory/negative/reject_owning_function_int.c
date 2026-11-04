// expect: rejected cerastes-unsupported_annotation @ L8
// mode: c23, c++20
// twin: tests/memory/heap_owning_result.c
// Pins: 0047 item 5: [[cerastes::owning]] on a function that returns no pointer
// Pre-registered from record 0047 item 5: rejected unsupported_annotation at the marked line (tag guessed: the nearest existing analogue)
#include <cerastes.h>

CERASTES_OWNING int f(void)  // <- here
{
  return 0;
}
