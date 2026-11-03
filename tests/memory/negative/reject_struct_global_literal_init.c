// expect: rejected cerastes-unsupported_declaration @ L12
// mode: c23, c++20
// twin: tests/memory/struct_global_zero.c
// Pins: 0044 item 7: a file-scope compound literal as the initializer of a struct global is static storage (P67 H5): rejected
#include <cerastes.h>

struct S {
  int x;
  int y;
};

static struct S gs = (struct S){1, 2};  // <- here

int main(void)
{
  return gs.x;
}
