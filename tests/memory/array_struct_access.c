// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 items 3 and 5 (Part B): a local array of structures. v[i].x and v[i].y are written and read for symbolic indexes beside each other (the element slots differ), an element of an initialized list is read at a symbolic index (the initializer's values are known per leaf), a nested structure's member v[i].t.x is written and read, and f is the program of the retired pin array_reject_struct_elements.c (a declaration with no use); elem_x and elem_tx are the programs of the staging pin array_reject_struct_elem_use.c
// Pre-registered from record 0064 items 3 and 5 (Part B), before any run: verifies
#include <cerastes.h>
#include <stddef.h>

struct S {
  int x;
};

struct P {
  int x;
  int y;
};

struct Q {
  struct P t;
  int z;
};

int f(void)
{
  [[maybe_unused]] struct S a[2];
  return 0;
}

int rw(int i, int j)
    CERASTES_REQUIRES(0 <= i && i < 3 && 0 <= j && j < 3 && i != j)
    CERASTES_ENSURES(r, r == 7)
{
  struct P v[3];
  v[i].x = 3;
  v[i].y = 4;
  v[j].x = 5;
  v[j].y = 6;
  return v[i].x + v[i].y;
}

int elem_x(int i)
    CERASTES_REQUIRES(0 <= i && i < 3)
    CERASTES_ENSURES(r, 1 <= r && r <= 5)
{
  struct P v[3] = {{1, 2}, {3, 4}, {5, 6}};
  return v[i].x;
}

int elem_tx(int i)
    CERASTES_REQUIRES(0 <= i && i < 2)
    CERASTES_ENSURES(r, 1 <= r && r <= 4)
{
  struct Q v[2] = {{{1, 2}, 3}, {{4, 5}, 6}};
  return v[i].t.x;
}

int nested(int i)
    CERASTES_REQUIRES(0 <= i && i < 2)
    CERASTES_ENSURES(r, r == 7)
{
  struct Q v[2];
  v[i].t.x = 7;
  v[i].t.y = 8;
  v[i].z = 9;
  return v[i].t.x;
}
