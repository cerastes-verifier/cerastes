// expect: rejected cerastes-unsupported_declaration @ L12
// mode: c23, c++20
// twin: tests/memory/struct_global_zero.c
// Pins: 0044 item 8 (M4.6): a const struct global is a const object
#include <cerastes.h>

struct S {
  int x;
  int y;
};

static const struct S k = {1, 2};  // <- here

int main(void)
{
  return k.x;
}
