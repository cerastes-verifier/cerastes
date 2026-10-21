// expect: rejected cerastes-unsupported_expression @ L10
// twin: tests/scalar/switch_fallthrough.c
// Pins: 0014 item 6: a case value that is a const variable (a C++ constant expression) is rejected, not guessed
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
