// expect: rejected wrong_entity_for_attribute @ L10
// mode: c23, c++20
// twin: tests/memory/init_must_init.c
// Pins: 0042 item 2 (P54, P64): must_init on a local is EDG's own refusal (target p only), re-expected by M4.5d2
#include <cerastes.h>

void g(void)
{
  int x = 0;
  CERASTES_MUST_INIT int *q = &x;  // <- here
  *q = 1;
}
