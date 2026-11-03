// expect: rejected cerastes-unsequenced_modification @ L15
// c-only: restrict and compound literal objects do not exist in C++
// twin: tests/memory/struct_literal_object.c
// Pins: 0044 item 7 (audit B1): {x, x++}: the write is in the second element
#include <cerastes.h>

struct S {
  int x;
  int y;
};

int f(int x)
    CERASTES_REQUIRES(0 <= x && x < 100)
{
  struct S *p = &(struct S){x, x++};  // <- here
  return p->y;
}
