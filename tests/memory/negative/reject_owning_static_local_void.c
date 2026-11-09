// expect: rejected cerastes-unsupported_annotation @ L12
// expect: frontend-warning declared_but_not_referenced @ L12
// mode: c23, c++20
// twin: tests/memory/owning_static_local_cache.c
// Pins: 0054 item 5: owning on a void * static local
// Pre-registered from record 0054 item 5: rejected unsupported_annotation at the declaration
#include <cerastes.h>
#include <stdlib.h>

int f(void)
{
  CERASTES_OWNING static void *s;  // <- here
  return 0;
}
