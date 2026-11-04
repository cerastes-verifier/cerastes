// expect: rejected cerastes-unsupported_annotation @ L7
// twin: tests/memory/heap_owning_result.c
// Pins: 0047 item 5 (audit S5): [[cerastes::owning]] on a predicate or pure function is rejected
// Pre-registered from record 0047 item 5: rejected unsupported_annotation at the declaration; re-registered after audit round 1 (record 0047 item 5): audit S5: every other routine entity registration lets the attribute reach is rejected
#include <cerastes.h>

CERASTES_OWNING CERASTES_PREDICATE bool is_small(int x)  // <- here
{
  return x < 10;
}
