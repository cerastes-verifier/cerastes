// expect: error uninitialized @ L17
// mode: c23, c++20
// twin: tests/scalar/loop_body_nested_write.c
// Pins: 0019 item 13: the read of last comes before its nested write, where the declaration left it without a value
#include <cerastes.h>

int inner(int n)
    CERASTES_REQUIRES(0 <= n)
    CERASTES_ENSURES(r, r == n)
{
  int i = 0;
  int count = 0;
  CERASTES_INVARIANT(i <= n && count == i)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    int last;
    count = last + 1;
    i = (last = i) + 1;
  }
  return count;
}

int outer(int n)
    CERASTES_REQUIRES(0 <= n)
    CERASTES_ENSURES(r, r == n)
{
  int last;
  int i = 0;
  int count = 0;
  CERASTES_INVARIANT(i <= n && count == i)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    i = (last = i) + 1;
    count = last + 1;
  }
  return count;
}
