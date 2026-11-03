// expect: error permission @ L18
// mode: c23, c++20
// twin: tests/memory/struct_global_zero.c
// Pins: 0044 item 8: touch has only owns(origin.x) and writes origin.y: permission at the store
#include <cerastes.h>

struct S {
  int x;
  int y;
};

static struct S origin;

void touch(void)
    CERASTES_REQUIRES(cerastes_owns(origin.x))
    CERASTES_ENSURES(cerastes_owns(origin.x))
{
  origin.y = 1;  // <- here
}

int main(void)
{
  return origin.x;
}
