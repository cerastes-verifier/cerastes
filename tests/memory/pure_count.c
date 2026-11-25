// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0062 item 7 (S46 (d) unaided): the exact count over a bool array, the sibling of array_loop_bool_post.c (whose loose invariant cannot show r < 3): count_to is a recursive pure function, the invariant c == count_to(a, i) is kept by the if and holds at the exit; the same over a local bool array. No concrete count is claimed (unfolding to a constant needs written steps)
// Pre-registered from record 0062 (Part D), before the code: verifies
#include <cerastes.h>

CERASTES_PURE int count_to(const bool *a, int n)
    CERASTES_REQUIRES(0 <= n && cerastes_reads_range(a, n))
    CERASTES_ENSURES(r, 0 <= r && r <= n)
    CERASTES_DECREASES(n)
{
  return n == 0 ? 0 : count_to(a, n - 1) + (a[n - 1] ? 1 : 0);
}

int count_true(const bool *a, int n)
    CERASTES_REQUIRES(0 <= n && cerastes_reads_range(a, n))
    CERASTES_ENSURES(r, cerastes_reads_range(a, n) && r == count_to(a, n))
{
  int c = 0;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && c == count_to(a, i))
  CERASTES_DECREASES(n - i)
  while (i < n) {
    if (a[i]) {
      c = c + 1;
    }
    i = i + 1;
  }
  return c;
}

int count_local(void)
{
  bool a[3] = {true, false, true};
  int c = 0;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3 && c == count_to(a, i))
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    if (a[i]) {
      c = c + 1;
    }
    i = i + 1;
  }
  CERASTES_CHECK(c == count_to(a, 3));
  return c;
}
