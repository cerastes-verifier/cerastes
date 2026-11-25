// expect: error check @ L20
// mode: c23, c++20
// twin: tests/memory/struct_array_address.c
// Pins: 0063 item 5: the write through &s.a[2] set the element to 8, so the check that s.a[2] is 7 fails
// Pre-registered from record 0063 item 5 (Part C), before any run: the one error
#include <cerastes.h>
#include <stddef.h>

struct S {
  int a[4];
  int n;
};

int use(int i)
    CERASTES_REQUIRES(0 <= i && i < 4)
{
  struct S s = {{1, 2, 3, 4}, 5};
  int *q = &s.a[2];
  *q = 8;
  CERASTES_CHECK(s.a[2] == 7);  // <- here
  return *q - 8 + i * 0;
}
