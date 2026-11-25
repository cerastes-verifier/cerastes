// expect: error check @ L25
// mode: c23, c++20
// twin: tests/memory/struct_array_loops.c
// Pins: 0063 item 10 (audit round 3 N9, a pinned false alarm by design, fail closed): a store to t.a[2] in a loop reaches every array place of the field a of structure S (the loop's array places are per field, not per object), so the values of s.a are not kept and the check fails
// Pre-registered from audit round 3 N9 and record 0063 item 10 (Part C), before any run: the one error
#include <cerastes.h>
#include <stddef.h>

struct S {
  int a[4];
  int n;
};

int main(void)
{
  struct S s = {{1, 2, 3, 4}, 5};
  struct S t = {{0}, 0};
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3)
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    t.a[2] = 7;
    i = i + 1;
  }
  CERASTES_CHECK(s.a[2] == 3);  // <- here
  return s.n * 0 + t.n * 0;
}
