// expect: rejected cerastes-annotation_macro_changed @ L7
// mode: c23, c++20
// twin: tests/vlibc/exit_explicit.c
// Pins: 0046 item 4 (audit S1, inject.c): a user macro over an identifier of vlibc's contract text (`false`) is refused, not read into the contract
// `#define false 1` would turn vlibc's `ensures(false)` into `ensures(1)`.
#include <cerastes.h>
#define false 1
#include <stdlib.h>

int f(int x)
    CERASTES_REQUIRES(0 <= x && x <= 10)
    CERASTES_ENSURES(r, r == 1)
{
  exit(x);
  return 0;
}
