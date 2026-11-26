// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 items 11 and 12 (Part C2): a structure whose member is an array of structures is a value by its value view (4 element leaves, under the limit of 7): a by-value parameter that the callee changes (the caller's object is unchanged), a whole-structure copy that the copy's writes do not reach back, and a structure value from a call stored to the whole structure with a write to another object in the argument, and a call whose argument writes t's own element (a call is a sequence point: store_own) (the sequencing pin negative/struct_member_array_struct_reject_seq_store.c writes t.v[0].x there, in a literal). The contract of bump reads t.n only: a contract read of an element of an array member of a structure VALUE is not in the record and stays rejected (negative/struct_member_array_struct_reject_forms.c)
// Pre-registered from record 0064 items 11 and 12 (Part C2), before any run: verifies
#include <cerastes.h>

struct P {
  int x;
  int y;
};

struct T {
  struct P v[2];
  int n;
};

struct T mkt(int a)
    CERASTES_ENSURES(r, r.n == a);

int bump(struct T t)
    CERASTES_REQUIRES(t.n == 5)
    CERASTES_ENSURES(r, r == 7)
{
  t.v[1].y = t.n + 2;
  return t.v[1].y;
}

int use(void)
    CERASTES_ENSURES(r, r == 7)
{
  struct T t = {{{1, 2}, {3, 4}}, 5};
  int w = bump(t);
  CERASTES_CHECK(w == 7 && t.v[1].y == 4);
  struct T u = t;
  u.v[0].x = 9;
  CERASTES_CHECK(u.v[0].x == 9 && t.v[0].x == 1 && u.v[1].y == 4 && u.n == 5);
  return w + u.n - 5;
}

int store_distinct(void)
    CERASTES_ENSURES(r, r == 0)
{
  struct T t = {{{1, 2}, {3, 4}}, 5};
  int j = 0;
  t = mkt(j++);
  CERASTES_CHECK(t.n == 0 && j == 1);
  return t.n;
}

int store_own(void)
    CERASTES_ENSURES(r, r == 0)
{
  struct T t = {{{1, 2}, {3, 4}}, 5};
  t = mkt(t.v[0].x++);
  CERASTES_CHECK(t.n == 1);
  return t.n - 1;
}
