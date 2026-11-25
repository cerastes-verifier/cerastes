// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0063 item 6: the array member takes a nested braced list (C's zero fill for the elements it leaves out), a designated member with a list, a list of non-constant elements, {} in both modes, and {0} and {{0}, 0} where the braces are written or the structure has the array as its only member (clang++ warns at -Wall on {0} eliding an array member's braces in a structure of two or more members: that one is the pin struct_array_reject_zero_cpp.cpp)
// Pre-registered from record 0063 item 6 (Part C), before any run: verifies
#include <cerastes.h>
#include <stddef.h>

struct S {
  int a[4];
  int n;
};

struct W {
  int a[4];
};

int forms_a(void)
    CERASTES_ENSURES(r, r == 0)
{
  struct S nested = {{1, 2, 3, 4}, 5};
  struct S named = {.a = {1, 2}, .n = 3};
  struct S partial = {{1, 2}, 5};
  CERASTES_CHECK(nested.a[3] == 4 && nested.n == 5);
  CERASTES_CHECK(named.a[1] == 2 && named.a[2] == 0 && named.a[3] == 0 && named.n == 3);
  CERASTES_CHECK(partial.a[1] == 2 && partial.a[2] == 0 && partial.a[3] == 0);
  return nested.n - 5 + (partial.n - 5) + (named.n - 3);
}

int forms_b(int x)
    CERASTES_REQUIRES(0 <= x && x < 100)
    CERASTES_ENSURES(r, r == 0)
{
  struct S dynamic = {{x, x + 1, 3, 4}, 5};
  struct S empty = {};
  struct W w = {0};
  struct S zeros = {{0}, 0};
  CERASTES_CHECK(dynamic.a[0] == x && dynamic.a[1] == x + 1 && dynamic.a[3] == 4);
  CERASTES_CHECK(empty.a[0] == 0 && empty.a[3] == 0 && empty.n == 0);
  CERASTES_CHECK(w.a[2] == 0 && zeros.a[1] == 0 && zeros.n == 0);
  return dynamic.n - 5 + empty.n + w.a[1] + zeros.n;
}
