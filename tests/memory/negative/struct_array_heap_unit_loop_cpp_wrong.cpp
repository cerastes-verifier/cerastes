// expect: error check @ L38
// mode: c++20
// twin: tests/memory/struct_array_heap_unit_loop_cpp.cpp
// Pins: 0063 item 10 (audit round 3 S4): the C++ unit with a wrong check after the loop: one check error, no abnormal end
// Pre-registered from audit round 3 S4 and record 0063 item 10 (Part C), before any run: the one error
#include <cerastes.h>
#include <stddef.h>
#include <stdlib.h>

struct S {
  int a[4];
  int n;
};

int g()
{
  int *h = (int *)malloc(sizeof(int));
  if (h == nullptr) {
    return 0;
  }
  *h = 1;
  int v = *h;
  free(h);
  return v - v;
}

int main()
{
  S s{{1, 2, 3, 4}, 5};
  S *p = &s;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3)
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    p->a[1] = i;
    i = i + 1;
  }
  CERASTES_CHECK(i == 4);  // <- here
  return g();
}
