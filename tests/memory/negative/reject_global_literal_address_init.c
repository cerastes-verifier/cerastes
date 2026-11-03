// expect: rejected cerastes-unsupported_expression @ L7
// mode: c23, c++20
// twin: tests/memory/struct_global_zero.c
// Pins: 0044 item 7: the address of a file-scope compound literal as a global initializer (static storage, P67 H5): rejected
#include <cerastes.h>

static int *gp = &(int){5};  // <- here

int main(void)
{
  return *gp;
}
