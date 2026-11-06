// expect: rejected cerastes-unsupported_annotation @ L11
// mode: c23, c++20
// twin: tests/memory/ref_to_uninit_local.c
// Pins: 0052 items 4 and 8: cerastes::ref_to_uninit on a static local: rejected (M4.5d2b2)
// Pre-registered from record 0052 item 4: rejected unsupported_annotation at the declaration (tag and line guessed)
#include <cerastes.h>
#include <stdlib.h>

int f(void)
{
  CERASTES_REF_TO_UNINIT static int *s = NULL;  // <- here
  return s == NULL ? 0 : 1;
}
