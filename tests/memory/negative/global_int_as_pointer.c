// expect: rejected cerastes-unsupported_declaration @ L7
// mode: c23, c++20
// twin: tests/memory/global_pointer_declared_unused.c
// Pins: 0041 item 2 (audit; P63 H3): a non-zero integer constant as a pointer initial value is not an address of a global (0032 item 3): rejected (M8)
#include <cerastes.h>

int *ip = (int *)4;

int ident(int a)
{
  return a;
}
