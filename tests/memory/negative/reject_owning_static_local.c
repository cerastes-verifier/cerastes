// expect: rejected cerastes-unsupported_annotation @ L11
// mode: c23, c++20
// twin: tests/memory/owning_local_free.c
// Pins: 0052 items 3 and 8: cerastes::owning on a static local (storage that outlives the call): M4.5d2b2
// Pre-registered from record 0052 item 8: rejected unsupported_annotation at the declaration (tag and line guessed)
#include <cerastes.h>
#include <stdlib.h>

int f(void)
{
  CERASTES_OWNING static int *s = NULL;  // <- here
  return s == NULL ? 0 : 1;
}
