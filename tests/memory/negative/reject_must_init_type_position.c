// expect: rejected wrong_entity_for_attribute @ L7
// mode: c23, c++20
// twin: tests/memory/init_must_init.c
// Pins: 0042 item 2 (audit S4, P64): must_init written after the * (on the type) is EDG's own refusal (#1835)
#include <cerastes.h>

void f(int *[[cerastes::must_init]] p) { (void)p; }  // <- here
