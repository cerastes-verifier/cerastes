// expect: rejected wrong_entity_for_attribute @ L7
// mode: c23, c++20
// twin: tests/memory/init_must_init.c
// Pins: 0042 item 2 (P54, P64): must_init on a function is EDG's own refusal
#include <cerastes.h>

CERASTES_MUST_INIT void f(void);  // <- here
