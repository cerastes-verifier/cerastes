// expect: error uninitialized @ L15
// mode: c23, c++20
// twin: tests/scalar/uninit_path_precise.c
// Pins: 0019 item 3: the second condition admits c == 0, where x was not assigned
#include <cerastes.h>

int guarded(int c)
    CERASTES_ENSURES(r, (c > 0 && r == 1) || c <= 0)
{
  int x;
  if (c > 0) {
    x = 1;
  }
  if (c >= 0) {
    return x;
  }
  return 0;
}
