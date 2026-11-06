// expect: rejected cerastes-unsupported_annotation @ L10
// mode: c23, c++20
// twin: tests/memory/owning_local_free.c
// Pins: 0052 item 3 (permanent): cerastes::owning on a local that is not a pointer
// Pre-registered from record 0052 item 3: rejected unsupported_annotation at the declaration
#include <cerastes.h>

int f(void)
{
  CERASTES_OWNING int x = 0;  // <- here
  return x;
}
