// expect: rejected wrong_entity_for_attribute @ L8
// mode: c23, c++20
// twin: tests/memory/not_invalidating_int_pointer.c
// Pins: 0058 item 2 (extra, not a pin of the record): not_invalidating written after the * (on the type) is EDG's own refusal (#1835, P64), as reject_must_init_type_position.c
// Pre-registered from record 0042 item 2's rule for target p: rejected wrong_entity_for_attribute at the parameter
#include <cerastes.h>

void f(int *[[cerastes::not_invalidating]] p) { (void)p; }  // <- here
