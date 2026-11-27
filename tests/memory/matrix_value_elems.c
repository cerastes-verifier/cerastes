// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 item 12 (question 353, the second type of the "tested so" pair): struct T { struct W v[2]; int k; } with struct W { int a[2]; int n; } (a member array of structures whose elements have an array leaf: six counted cells in v, k beside them) is passed by value, returned by value and held in a local: the callee's requires and the ensures state the member BESIDE the array of structures (t.k, r.k: a contract cannot read an element of such a value), so a misalignment of the leaves of v against k would be observed; the caller reads k of the result back by a check; negative/matrix_value_elems_wrong.c is the twin
// Pre-registered from record 0064 item 12 as amended on 2026-11-26 (Part E, slice 4), before any run (test-mutator-11, 2026-11-27): verifies (time it first: struct T locals are the costly ones, 15 to 23 s each)
#include <cerastes.h>

struct W {
  int a[2];
  int n;
};

struct T {
  struct W v[2];
  int k;
};

int pick(struct T t)
    CERASTES_REQUIRES(t.k == 7)
    CERASTES_ENSURES(r, r == 7)
{
  return t.k;
}

int use(void)
    CERASTES_ENSURES(r, r == 7)
{
  struct T t = {{{{1, 2}, 3}, {{4, 5}, 6}}, 7};
  return pick(t);
}

struct T make(void)
    CERASTES_ENSURES(r, r.k == 7)
{
  struct T t = {{{{1, 2}, 3}, {{4, 5}, 6}}, 7};
  return t;
}

int use_result(void)
    CERASTES_ENSURES(r, r == 7)
{
  struct T w = make();
  CERASTES_CHECK(w.k == 7);
  return w.k;
}
