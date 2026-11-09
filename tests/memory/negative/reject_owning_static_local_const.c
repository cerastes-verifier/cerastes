// expect: rejected cerastes-unsupported_annotation @ L11
// mode: c23, c++20
// twin: tests/memory/owning_static_local_cache.c
// Pins: 0054 item 5: owning on a pointer to a const object, static local
// Pre-registered from record 0054 item 5: rejected unsupported_annotation at the declaration
#include <cerastes.h>
#include <stdlib.h>

int f(void)
{
  CERASTES_OWNING static const int *s = NULL;  // <- here
  return s == NULL;
}
