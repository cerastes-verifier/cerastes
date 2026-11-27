// expect: error uninitialized @ L40
// expect: error uninitialized @ L47
// expect: error uninitialized @ L54
// expect: error uninitialized @ L63
// expect: error uninitialized @ L73
// expect: error uninitialized @ L80
// expect: error uninitialized @ L88
// expect: error uninitialized @ L95
// expect: error uninitialized @ L104
// expect: error uninitialized @ L112
// mode: c23, c++20
// twin: tests/memory/struct_member_array_struct_init.c
// Pins: 0064 items 6 and 12 (audit round C2, finding 3): a structure whose array member of structures has one leaf never written (v[1].y of t, the first five functions; v[0].x, v[1].y, v[0].y, t.n and v[1].x in the element shapes of the last five) is `uninitialized` at the first use that reads the leaf: when it is returned, passed, copied, assigned to another structure or stored through a pointer, and when an element is copied, passed, read, stored into another element or read at a symbolic index. One reason in every row: a flag of a leaf of an element of the member array is not set
// Registered from audit-C2 probes c07 (g1 to g5: `uninitialized` for t.v[1].y at each) and g07 (a to e: `uninitialized` for t.v[0].x, t.v[1].y, t.v[0].y, t.n, t.v[1].x), measured on snapshot a044aff629b8, the probes' bodies laid out as this file; the rows' lines are inferred from the layout, the categories measured; g4's `return u.n` is added (the probe warned #550 for u), unmeasured
#include <cerastes.h>

struct P {
  int x;
  int y;
};

struct T {
  struct P v[2];
  int n;
};

struct T2 {
  int n;
  struct P v[2];
  int m;
};

int f(struct T2 t);
int fp(struct P a);

struct T2 ret(void)
{
  struct T2 t;
  t.n = 1; t.v[0].x = 2; t.v[0].y = 3; t.v[1].x = 4; t.m = 8;
  return t;  // <- here
}

int pass(void)
{
  struct T2 t;
  t.n = 1; t.v[0].x = 2; t.v[0].y = 3; t.v[1].x = 4; t.m = 8;
  return f(t);  // <- here
}

int copy(void)
{
  struct T2 t;
  t.n = 1; t.v[0].x = 2; t.v[0].y = 3; t.v[1].x = 4; t.m = 8;
  struct T2 u = t;  // <- here
  return u.n;
}

int assign(void)
{
  struct T2 t;
  struct T2 u = {1, {{2, 3}, {4, 5}}, 8};
  t.n = 1; t.v[0].x = 2; t.v[0].y = 3; t.v[1].x = 4; t.m = 8;
  u = t;  // <- here
  return u.n;
}

void store(struct T2 *d)
    CERASTES_REQUIRES(cerastes_owns(*d))
    CERASTES_ENSURES(cerastes_owns(*d))
{
  struct T2 t;
  t.n = 1; t.v[0].x = 2; t.v[0].y = 3; t.v[1].x = 4; t.m = 8;
  *d = t;  // <- here
}

int elem_copy(void)
{
  struct T t;
  t.n = 1;
  struct P e = t.v[0];  // <- here
  return e.x;
}

int elem_pass(void)
{
  struct T t;
  t.n = 1; t.v[1].x = 1;
  return fp(t.v[1]);  // <- here
}

int leaf_read(void)
{
  struct T t;
  t.v[0].x = 1;
  return t.v[0].y;  // <- here
}

int elem_set(void)
{
  struct T t;
  t.v[0].x = 1; t.v[0].y = 1;
  struct P e = t.v[0];
  t.v[1] = e;
  return t.v[1].x + t.v[1].y + t.n;  // <- here
}

int sym_index(int i)
    CERASTES_REQUIRES(0 <= i && i < 2)
{
  struct T t;
  t.v[i].x = 1;
  return t.v[1].x;  // <- here
}
