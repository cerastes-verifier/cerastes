// expect: rejected cerastes-unsupported_annotation @ L11
// mode: c23, c++20
// twin: tests/memory/owning_local_free.c
// Pins: 0052 item 3: owning(n), a counted owner, on a local: M4.5e2 (owning(n) beyond parameters, record 0059 item 1); like reject_owning_count.c for a parameter. Not native-ok: the raw attribute is an unknown attribute to the other compilers
// Pre-registered from record 0052 item 3: rejected unsupported_annotation at the declaration
#include <cerastes.h>
#include <stddef.h>

int f(void)
{
  [[cerastes::owning(2)]] int *p = NULL;  // <- here
  return p == NULL ? 0 : 1;
}
