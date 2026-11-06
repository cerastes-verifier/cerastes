// expect: rejected cerastes-unsupported_annotation @ L9
// mode: c23, c++20
// twin: tests/memory/ref_to_uninit_local.c
// Pins: 0052 items 4 and 8 (amends 0042 item 2): cerastes::ref_to_uninit on a global: the registration accepts target v, the encoder rejects a global; M4.5d2b2
// Pre-registered from record 0052 item 4: rejected unsupported_annotation at the declaration (tag and line guessed)
#include <cerastes.h>
#include <stdlib.h>

CERASTES_REF_TO_UNINIT int *g = NULL;  // <- here

int main(void)
{
  return 0;
}
