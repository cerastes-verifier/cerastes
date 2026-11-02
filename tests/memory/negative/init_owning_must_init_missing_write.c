// expect: error postcondition @ L13
// mode: c23, c++20
// twin: tests/memory/init_owning_must_init.c
// Pins: 0042 items 4, 5: the owning atom's flag row is postcondition when p is not null and nothing wrote *p
#include <cerastes.h>
#include <stddef.h>

void set_zero(CERASTES_OWNING CERASTES_MUST_INIT int *p)
{
  if (p != NULL) {
    (void)0;
  }
}  // <- here
