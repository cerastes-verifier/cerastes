// expect: rejected cerastes-diagnostic_pragma_in_verify @ L7
// expect: rejected cerastes-unsupported_expression @ L14
// c-only: C++ accepts a const local in a case value; C23 does not (6.6p8)
// twin: tests/scalar/switch_case_named.cpp
// Pins: 0017 item 2: the diag pragma itself is refused, and it cannot reopen the C rule: case k naming a const object is still rejected at the label
// A C _Static_assert naming a const under such a pragma is a documented gap (0017 item 2).
#pragma diag_suppress 1901
#include <cerastes.h>

int pick(int x)
{
  const int k = 5;
  switch (x) {
  case k:
    return 1;
  default:
    return 0;
  }
}
