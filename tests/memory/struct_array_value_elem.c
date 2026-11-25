// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0063 item 13: a by-value result (every element in the ensures, the caller's copy), `mk(7).a[1]` and `mk(7).a[i]` (an element of a function's value: one out-of-bounds obligation), and `s.a[i]` of a by-value parameter in a requires and an ensures (twins negative/struct_array_result_wrong.c, struct_array_value_index_high.c, struct_array_value_requires.c)
// Pre-registered from record 0063 item 13 (Part E), before any run: verifies
#include <cerastes.h>
#include <stddef.h>

struct S {
  int a[4];
  int n;
};

struct S mk(int x)
    CERASTES_ENSURES(r, r.a[0] == x && r.a[1] == x && r.a[2] == x && r.a[3] == x && r.n == 0)
{
  struct S s = {{x, x, x, x}, 0};
  return s;
}

int one(void)
    CERASTES_ENSURES(r, r == 7)
{
  return mk(7).a[1];
}

int pick(int i)
    CERASTES_REQUIRES(0 <= i && i < 4)
    CERASTES_ENSURES(r, r == 7)
{
  return mk(7).a[i];
}

int get(struct S s, int i)
    CERASTES_REQUIRES(0 <= i && i < 4 && s.a[i] > 0)
    CERASTES_ENSURES(r, r == s.a[i])
{
  return s.a[i];
}

int use(void)
    CERASTES_ENSURES(r, r == 10)
{
  struct S s = {{1, 2, 3, 4}, 5};
  int g = get(s, 2);
  CERASTES_CHECK(g == 3);
  struct S t = mk(7);
  CERASTES_CHECK(t.a[3] == 7 && t.n == 0);
  return g + t.a[0];
}
