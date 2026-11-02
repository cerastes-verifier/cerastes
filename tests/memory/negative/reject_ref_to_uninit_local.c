// expect: rejected wrong_entity_for_attribute @ L10
// mode: c23, c++20
// twin: tests/memory/init_ref_to_uninit.c
// Pins: 0042 item 2 (P54, P64): ref_to_uninit on a local is EDG's own refusal (target p only), re-expected by M4.5d2 (targets v and r)
#include <cerastes.h>

void g(void)
{
  int x = 0;
  CERASTES_REF_TO_UNINIT int *q = &x;  // <- here
  *q = 1;
}
