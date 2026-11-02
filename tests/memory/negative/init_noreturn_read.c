// expect: error uninitialized @ L16
// mode: c23, c++20
// twin: tests/memory/init_noreturn.c
// Pins: 0042 item 4: the path where bail is not called reads x uninitialized
#include <cerastes.h>

[[noreturn]] void bail(int *p CERASTES_REF_TO_UNINIT)
    CERASTES_REQUIRES(cerastes_owns(*p));

int use_bail(int c)
{
  int x;
  if (c != 0) {
    bail(&x);
  }
  return x;  // <- here
}
