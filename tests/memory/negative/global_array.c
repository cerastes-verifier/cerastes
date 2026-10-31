// expect: rejected cerastes-unsupported_declaration @ L7
// mode: c23, c++20
// twin: tests/memory/global_declared_unused.c
// Pins: 0041 item 2: an array global is rejected until M4.5e1
#include <cerastes.h>

int values[3];

int ident(int a)
{
  return a;
}
