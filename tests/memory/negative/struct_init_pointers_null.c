// expect: error check @ L16
// mode: c23, c++20
// twin: tests/memory/struct_init_pointers.c
// Pins: 0043 item 6: the pointer member the list leaves out is null
#include <cerastes.h>
#include <stddef.h>

struct L {
  int *p;
  int v;
};

int pointer_member(void)
{
  struct L m = {};
  CERASTES_CHECK(m.p != NULL);  // <- here
  return m.v;
}
