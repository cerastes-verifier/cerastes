// expect: rejected cerastes-unsupported_annotation @ L11
// mode: c23, c++20
// twin: tests/memory/owning_local_free.c
// Pins: 0052 item 3 (permanent, 0047 N1): cerastes::owning on a pointer to const: the owner could not write or free what it holds
// Pre-registered from record 0052 item 3: rejected unsupported_annotation at the declaration
#include <cerastes.h>
#include <stddef.h>

int f(void)
{
  CERASTES_OWNING const int *p = NULL;  // <- here
  return p == NULL ? 0 : 1;
}
