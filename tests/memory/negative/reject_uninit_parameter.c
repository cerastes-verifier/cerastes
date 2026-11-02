// expect: rejected wrong_entity_for_attribute @ L7
// mode: c23, c++20
// twin: tests/memory/init_uninit_local.c
// Pins: 0042 item 2 (P54, P64): uninit on a parameter is EDG's own refusal (#1835)
#include <cerastes.h>

int f(CERASTES_UNINIT int n) { return n; }  // <- here
