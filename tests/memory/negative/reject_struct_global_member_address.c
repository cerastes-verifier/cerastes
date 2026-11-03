// expect: rejected cerastes-unsupported_expression @ L13
// mode: c23, c++20
// twin: tests/memory/struct_global_zero.c
// Pins: 0044 item 8 (M8, listed): &h.x as a global initializer is told from &h by its constant's subobject path and rejected
#include <cerastes.h>

struct S {
  int x;
  int y;
};

static struct S h;
static int *gp = &h.x;  // <- here

int main(void)
{
  return gp == &h.x;
}
