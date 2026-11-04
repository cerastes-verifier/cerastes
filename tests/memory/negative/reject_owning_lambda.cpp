// expect: rejected wrong_entity_for_attribute @ L9
// twin: tests/memory/heap_owning_result.c
// Pins: 0047 item 5 (audit S5; owner question 125): EDG itself refuses [[cerastes::owning]] on a lambda (wrong_entity_for_attribute): the registration lets the attribute reach routines, not a closure's call operator
// Pre-registered from record 0047 item 5: rejected unsupported_annotation at the lambda (the tag annotation_on_lambda may fire instead: for the first run to settle); re-registered after audit round 1 (record 0047 item 5): audit S5: every other routine entity registration lets the attribute reach is rejected
#include <cerastes.h>

int *use()
{
  auto make = [](int n) CERASTES_OWNING -> int * { (void)n; return nullptr; };  // <- here
  return make(1);
}
