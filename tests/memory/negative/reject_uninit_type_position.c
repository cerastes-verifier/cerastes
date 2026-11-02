// expect: rejected wrong_entity_for_attribute @ L10
// expect: frontend-warning declared_but_not_referenced @ L10
// mode: c23, c++20
// twin: tests/memory/init_uninit_local.c
// Pins: 0042 item 2 (audit S4, P64): uninit written after the * (on the type) is EDG's own refusal (#1835)
#include <cerastes.h>

void f(void)
{
  int *[[cerastes::uninit]] x;  // <- here
}
