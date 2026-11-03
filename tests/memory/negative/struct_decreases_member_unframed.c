// expect: error permission @ L14
// mode: c23, c++20
// twin: tests/memory/struct_decreases_member.c
// Pins: 0044 item 10 (0039 item 8): decreases(p->x) with no atom for *p: the measure reads p->x without owning it
#include <cerastes.h>

struct S {
  int x;
  int y;
};

int count(struct S *p)
    CERASTES_REQUIRES(p != 0)
    CERASTES_DECREASES(p->x)  // <- here
{
  return 0;
}
