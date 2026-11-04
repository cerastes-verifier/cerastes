// expect: rejected cerastes-unsupported_declaration @ L7
// expect: rejected cerastes-unsupported_annotation @ L8
// twin: tests/memory/heap_owning_result.c
// Pins: 0047 item 5 (audit S5; owner question 125): a function template is rejected in a verified file (unsupported_declaration, at the `template` line; M5), and the attribute on it or on one of its instances is rejected as well (unsupported_annotation, at the declaration; owner question 126)
// Pre-registered from record 0047 item 5: rejected unsupported_annotation at the declaration; re-registered after audit round 1 (record 0047 item 5): audit S5: every other routine entity registration lets the attribute reach is rejected
#include <cerastes.h>
template <class T>  // <- here
CERASTES_OWNING T *make()  // <- here
{
  return nullptr;
}
