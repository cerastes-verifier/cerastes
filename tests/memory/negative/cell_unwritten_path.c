// expect: error uninitialized @ L16
// mode: c23, c++20
// twin: tests/memory/cell_written_on_all_paths.c
// Pins: 0019 and 0032 item 6: the path with a <= 0 returns a cell it never wrote
#include <cerastes.h>

int magnitude(int a)
    CERASTES_REQUIRES(-1000 < a && a < 1000)
    CERASTES_ENSURES(r, r >= 0 && (r == a || r == -a))
{
  int x;
  int *p = &x;
  if (a > 0) {
    *p = a;
  }
  return x;
}
