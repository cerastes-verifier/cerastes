// expect: rejected cerastes-unsupported_declaration @ L7
// mode: c23, c++20
// twin: tests/memory/global_declared_unused.c
// Pins: 0041 item 2: a global defined in another translation unit (an extern declaration with no definition here) is rejected until M8
#include <cerastes.h>

extern int outside;

int get(void)
{
  return outside;
}
