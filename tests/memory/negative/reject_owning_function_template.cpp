// expect: rejected cerastes-unsupported_annotation @ L8
// twin: tests/memory/heap_owning_result.c
// Pins: 0047 item 5 (audit S5): [[cerastes::owning]] on a function template (or an instance) is rejected (M5)
// Pre-registered from record 0047 item 5: rejected unsupported_annotation at the declaration; re-registered after audit round 1 (record 0047 item 5): audit S5: every other routine entity registration lets the attribute reach is rejected
#include <cerastes.h>

template <class T>
CERASTES_OWNING T *make()  // <- here
{
  return nullptr;
}
