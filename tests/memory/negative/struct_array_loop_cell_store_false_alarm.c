// expect: error check @ L24
// mode: c23, c++20
// twin: tests/memory/struct_array_loops.c
// Pins: 0063 item 10 (audit round 4 S2, the probe n01; a pinned false alarm, fail closed): a store to a member of a structure local in a loop, a cell (s.n) or an element, reaches every place of that member's field type (0043 item 11's rule for members, 0044 item 8), so the values of s.a are not kept after a loop that writes only s.n, and the check fails; the element-store form is struct_array_loop_store_false_alarm.c
// Pre-registered from audit round 4 S2 and record 0063 item 10 (Commit 4), before any run: the one error
#include <cerastes.h>
#include <stddef.h>

struct S {
  int a[4];
  int n;
};

int main(void)
{
  struct S s = {{1, 2, 3, 4}, 5};
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3)
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    s.n = i;
    i = i + 1;
  }
  CERASTES_CHECK(s.a[2] == 3);  // <- here
  return s.n * 0;
}
