// expect: rejected cerastes-unsupported_type @ L7
// mode: c23, c++20
// twin: tests/memory/global_pointer_declared_unused.c
// Pins: 0041 item 2 (0032 item 2): a void * global is rejected like a void * local
#include <cerastes.h>

void *vp;

int ident(int a)
{
  return a;
}
