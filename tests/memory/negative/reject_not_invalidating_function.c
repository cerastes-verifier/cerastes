// expect: rejected wrong_entity_for_attribute @ L8
// mode: c23, c++20
// twin: tests/memory/not_invalidating_int_pointer.c
// Pins: 0058 item 5: not_invalidating on a function is EDG's own refusal (#1835, the registration's target is p only), as must_init on a function is
// Pre-registered from record 0058 item 5: rejected wrong_entity_for_attribute (as reject_must_init_function.c)
#include <cerastes.h>

CERASTES_NOT_INVALIDATING void f(void);  // <- here
