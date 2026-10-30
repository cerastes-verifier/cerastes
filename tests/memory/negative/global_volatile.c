// expect: rejected cerastes-unsupported_type @ L7
// mode: c23, c++20
// twin: tests/memory/global_declared_unused.c
// Pins: 0041 item 2 (0032 item 2): a volatile global is rejected (M8)
#include <cerastes.h>

volatile int flag = 0;

int ident(int a)
{
  return a;
}
