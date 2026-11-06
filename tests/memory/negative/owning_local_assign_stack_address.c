// expect: error permission @ L13
// mode: c23, c++20
// twin: tests/memory/owning_local_free.c
// Pins: 0052 item 3 (the value rule): the same by assignment: a non-null new value that is not a held heap block is permission at the assignment
// Pre-registered from record 0052 item 3: permission at the assignment
#include <cerastes.h>
#include <stddef.h>

void g(void)
{
  int x = 0;
  CERASTES_OWNING int *q = NULL;
  q = &x;  // <- here
  *q = 1;
}
