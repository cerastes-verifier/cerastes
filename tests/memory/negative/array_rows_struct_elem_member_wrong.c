// expect: error postcondition @ L25
// mode: c23, c++20
// twin: tests/memory/array_rows_struct_elem_member.c
// Pins: 0064 item 5: the cell read back is the cell written, seven, so a result of eight fails the postcondition at the return
// Pre-registered from record 0064 item 5 (Part E), before any run: the one error
#include <cerastes.h>

struct W {
  int a[4];
  int n;
};

struct T {
  struct W v[2];
  int k;
};

int rw(int i, int j)
    CERASTES_REQUIRES(0 <= i && i < 2 && 0 <= j && j < 4)
    CERASTES_ENSURES(r, r == 7)
{
  struct T t;
  t.k = 0;
  t.v[i].a[j] = 7;
  return t.v[i].a[j] + 1;  // <- here
}
