// expect: rejected cerastes-line_directive_in_verify @ tests/scalar/headers/line_directive.h:L5
// mode: c23, c++20
// twin: tests/scalar/header_use.c
// Pins: 0011 item 4: a #line directive in a user header is rejected, reported in the header
#include <cerastes.h>

#include "../headers/line_directive.h"

int use(int x)
    CERASTES_REQUIRES(x >= 0)
{
  return x;
}
