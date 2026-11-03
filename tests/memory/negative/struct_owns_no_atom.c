// expect: error permission @ L16
// mode: c23, c++20
// twin: tests/memory/struct_owns_param.c
// Pins: 0043 item 5: no atom for *p: the write fails its permission check, naming p->x
#include <cerastes.h>
#include <stddef.h>

struct S {
  int x;
  int y;
};

void put(struct S *p)
    CERASTES_REQUIRES(p != NULL)
{
  p->x = 1;  // <- here
}
