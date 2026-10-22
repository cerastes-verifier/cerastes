// expect: error uninitialized @ L16
// mode: c23, c++20
// twin: tests/scalar/uninit_goto_past_decl.c
// Pins: 0019 item 2: the goto skips the assignment, the read is after the label
#include <cerastes.h>

int skip_assign(int a)
    CERASTES_REQUIRES(0 <= a && a <= 10)
{
  int x;
  if (a > 5) {
    goto done;
  }
  x = a;
done:
  return x;
}
