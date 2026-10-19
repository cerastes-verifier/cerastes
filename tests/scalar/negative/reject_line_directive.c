// expect: rejected cerastes-line_directive_in_verify @ L9
// mode: c23, c++20
// twin: tests/scalar/if_else.c
// Pins: 0011 item 4: a #line directive in a verified file is rejected
#include <cerastes.h>

int renumbered(int x)
{
#line 100
  return x;
}
