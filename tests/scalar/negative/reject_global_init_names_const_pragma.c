// expect: rejected cerastes-diagnostic_pragma_in_verify @ L7
// expect: rejected cerastes-unsupported_expression @ L11
// c-only: C++ accepts an initializer naming another constant; C23 does not (6.6p8)
// twin: tests/scalar/const_global_init_named.cpp
// Pins: 0017 item 2: the diag pragma itself is refused, and it cannot reopen the C rule: an initializer naming another const object is still rejected
// A C _Static_assert naming a const under such a pragma is a documented gap (0017 item 2).
#pragma diag_suppress 1901
#include <cerastes.h>

const int A = 5;
const int B = A + 1;

int get(void)
{
  return B;
}
