// expect: error division-by-zero @ L13
// mode: c23, c++20
// twin: tests/memory/compound_store_value.c
// Pins: 0039 item 2: *p /= d used as a value divides by zero when d may be 0; the error names this file and line (record 0053 item 10: a GCC build had lost the file)
#include <cerastes.h>

int shrink_through(int k, int d)
    CERASTES_REQUIRES(0 <= k && 0 <= d)
    CERASTES_ENSURES(r, d == 0 || r == k / d)
{
  int x = k;
  int *p = &x;
  int t = (*p /= d);
  return t;
}
