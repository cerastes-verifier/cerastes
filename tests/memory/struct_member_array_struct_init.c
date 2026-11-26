// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 items 6 and 12 (audit round C2, finding 3, uninitialized leaves of a member array of structures): a structure local with an array member of structures that every leaf of every element is written before it is read is a value that can be returned, passed, copied, assigned to another structure and stored through a pointer (each leaf has its own flag), and an element is a structure value too (copied to a local, passed, stored into another element) once its leaves are written. The twin negative/struct_member_array_struct_uninit.c leaves one leaf unwritten in each shape. Cost exception to the plan's rule 9 (at most one structure with a member array per function): `assign` holds two, as the audit's probe c07 g4 did (the whole file ran in 18 s there)
// Registered from audit-C2 probes c07 (g6, `return f(t)` with every leaf written, measured on snapshot a044aff629b8: verified, no row at its line) and g07 (the element shapes): each function is the probe's with the missing write added, which makes it verified by the flag rules but was not itself run; sym_index (a write and a read at one symbolic index) has no probe, registered from record 0064 item 6
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
  t.n = 1; t.v[0].x = 2; t.v[0].y = 3; t.v[1].x = 4; t.v[1].y = 5; t.m = 8;
  return t;
}

int pass(void)
{
  struct T2 t;
  t.n = 1; t.v[0].x = 2; t.v[0].y = 3; t.v[1].x = 4; t.v[1].y = 5; t.m = 8;
  return f(t);
}

int copy(void)
{
  struct T2 t;
  t.n = 1; t.v[0].x = 2; t.v[0].y = 3; t.v[1].x = 4; t.v[1].y = 5; t.m = 8;
  struct T2 u = t;
  return u.n;
}

int assign(void)
{
  struct T2 t;
  struct T2 u = {1, {{2, 3}, {4, 5}}, 8};
  t.n = 1; t.v[0].x = 2; t.v[0].y = 3; t.v[1].x = 4; t.v[1].y = 5; t.m = 8;
  u = t;
  return u.n;
}

void store(struct T2 *d)
    CERASTES_REQUIRES(cerastes_owns(*d))
    CERASTES_ENSURES(cerastes_owns(*d))
{
  struct T2 t;
  t.n = 1; t.v[0].x = 2; t.v[0].y = 3; t.v[1].x = 4; t.v[1].y = 5; t.m = 8;
  *d = t;
}

int elem_copy(void)
{
  struct T t;
  t.v[0].x = 1; t.v[0].y = 2;
  struct P e = t.v[0];
  return e.x;
}

int elem_pass(void)
{
  struct T t;
  t.v[1].x = 1; t.v[1].y = 2;
  return fp(t.v[1]);
}

int elem_set(void)
{
  struct T t;
  t.v[0].x = 1; t.v[0].y = 1; t.n = 3;
  struct P e = t.v[0];
  t.v[1] = e;
  return t.v[1].x + t.v[1].y + t.n;
}

int sym_index(int i)
    CERASTES_REQUIRES(0 <= i && i < 2)
{
  struct T t;
  t.v[i].x = 1;
  return t.v[i].x;
}
