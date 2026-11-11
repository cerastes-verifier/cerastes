// expect: error out-of-bounds @ L12
// mode: c23, c++20
// twin: tests/memory/array_check_index.c
// Pins: 0059 item 4: i may be 3 at the check: out-of-bounds at the check that reads a[i] (and nothing else: the read after it is not reached on the failing path)
// Pre-registered from the M4.5e1a sweep (report 5, s14)
#include <cerastes.h>

int get(int i)
    CERASTES_REQUIRES(0 <= i && i <= 3)
{
  int a[3] = {0};
  CERASTES_CHECK(a[i] == 0);
  return a[0];
}
