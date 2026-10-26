// expect: rejected cerastes-self_comparison @ L8
// mode: c23, c++20
// twin: tests/scalar/warn_accepted_neighbours.c
// Pins: 0025 item 6 R5: in an annotation the compilers see nothing, so a macro does not exempt: `CERASTES_REQUIRES(x == x)` is rejected like `[[cerastes::requires(x == x)]]`
#include <cerastes.h>

int test(int x)
    CERASTES_REQUIRES(x == x)
{
  return x;
}
