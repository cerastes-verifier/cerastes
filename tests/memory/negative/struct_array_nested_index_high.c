// expect: error out-of-bounds @ L23
// mode: c23, c++20
// twin: tests/memory/struct_array_nested.c
// Pins: 0063 items 3 and 5: the nested member's length is 4: i == 4 is out-of-bounds
// Pre-registered from record 0063 item 5 (Part C), before any run: the one error
#include <cerastes.h>
#include <stddef.h>

struct S {
  int a[4];
  int n;
};

struct T {
  struct S s;
  int z;
};

int use(int i)
    CERASTES_REQUIRES(0 <= i && i <= 4)
{
  struct T t = {{{1, 2, 3, 4}, 5}, 9};
  t.s.a[i] = 6;  // <- here
  return t.z;
}
