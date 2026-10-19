// expect: error division-by-zero @ L16
// mode: c23, c++20
// twin: tests/scalar/short_circuit.c
// Pins: 0012 item 5: b != 0 || a / b evaluates the division exactly when b == 0
#include <cerastes.h>

int and_guard(int a, int b)
    CERASTES_REQUIRES(a >= 0)
{
  return b != 0 && a / b > 1;
}

int or_guard(int a, int b)
    CERASTES_REQUIRES(a >= 0)
{
  return b != 0 || a / b > 1;
}

int cond_guard(int a, int b)
    CERASTES_REQUIRES(a >= 0)
{
  return b != 0 ? a / b : 0;
}
