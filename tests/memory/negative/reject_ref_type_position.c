// expect: rejected wrong_entity_for_attribute @ L7
// mode: c23, c++20
// twin: tests/memory/init_ref_to_uninit.c
// Pins: 0042 item 2 (audit S4, P64): ref_to_uninit written after the * (on the type) is EDG's own refusal (#1835)
#include <cerastes.h>

void f(int *[[cerastes::ref_to_uninit]] p) { (void)p; }  // <- here
