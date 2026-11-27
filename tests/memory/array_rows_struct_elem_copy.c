// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 item 12 (audit E3a finding 8 (a); the auditor's c01, c02_*, wself, weqvac/wnevac programs): v[i] = v[j] of elements with an array member copies every cell of the array leaf from its own cell and the scalar leaf by a store: the copy and the source agree in each cell, the neighbouring element and the source are kept, v[i] = v[i] keeps what was written before it (i == j), two cells of one array and the same cell are told apart by the guard d != s, i != j and i == j symbolically
// Pre-registered (test-mutator-10, Task 1/2, before any run): verifies (each function is the right-value sibling of one wrong twin in negative/array_rows_struct_elem_copy_wrong.c); time it first
#include <cerastes.h>

struct W {
  int a[3];
  int n;
};

int copy_all(void)
    CERASTES_ENSURES(r, r == 0)
{
  struct W v[2] = {{{1, 2, 3}, 4}, {{5, 6, 7}, 8}};
  v[1] = v[0];
  CERASTES_CHECK(v[1].a[0] == 1 && v[1].a[1] == 2 && v[1].a[2] == 3 && v[1].n == 4);
  CERASTES_CHECK(v[0].a[0] == 1 && v[0].a[1] == 2 && v[0].a[2] == 3 && v[0].n == 4);
  return 0;
}

int neighbours(void)
    CERASTES_ENSURES(r, r == 0)
{
  struct W v[3] = {{{1, 2, 3}, 4}, {{5, 6, 7}, 8}, {{9, 10, 11}, 12}};
  v[1] = v[0];
  CERASTES_CHECK(v[2].a[0] == 9 && v[2].a[1] == 10 && v[2].a[2] == 11 && v[2].n == 12);
  return 0;
}

int self(int i)
    CERASTES_REQUIRES(0 <= i && i < 2)
    CERASTES_ENSURES(r, r == 7)
{
  struct W v[2] = {{{1, 2, 3}, 4}, {{5, 6, 7}, 8}};
  v[i].a[1] = 7;
  v[i] = v[i];
  return v[i].a[1];
}

int same(int i, int j)
    CERASTES_REQUIRES(0 <= i && i < 2 && i == j)
    CERASTES_ENSURES(r, r == 70)
{
  struct W v[2] = {{{1, 2, 3}, 4}, {{5, 6, 7}, 8}};
  v[i].a[2] = 70;
  v[i] = v[j];
  return v[j].a[2];
}

int apart(int i, int j)
    CERASTES_REQUIRES(0 <= i && i < 2 && 0 <= j && j < 2 && i != j)
    CERASTES_ENSURES(r, r == 70 + 70)
{
  struct W v[2] = {{{1, 2, 3}, 4}, {{5, 6, 7}, 8}};
  v[j].a[1] = 70;
  v[i] = v[j];
  return v[i].a[1] + v[j].a[1];
}
