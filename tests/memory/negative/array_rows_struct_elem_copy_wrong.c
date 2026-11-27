// expect: error check @ L25
// expect: error check @ L33
// expect: error check @ L41
// expect: error check @ L49
// expect: error check @ L57
// expect: error check @ L65
// expect: error postcondition @ L76
// expect: error postcondition @ L86
// expect: error postcondition @ L96
// mode: c23, c++20
// twin: tests/memory/array_rows_struct_elem_copy.c
// Pins: 0064 item 12 (audit E3a finding 8 (a); the auditor's c02_w0, w1, w2, wn, ws, wnb, wself and the i == j and i != j programs): after v[1] = v[0] each cell of the copy equals the source's cell (the first claim of each function is wrong by one cell or the scalar, or about the source, or about the neighbouring element, which the copy does not touch), and the postconditions of v[i] = v[i], i == j and i != j that state the value written before the copy (the right values 7, 70 and 140 are in the positive twin) fail at the return
// Pre-registered (test-mutator-10, Task 1/2, before any run): one error per function, the check or the return; values taken from the auditor's measured probes (every row `check` or `postcondition`, both modes); right-value siblings: the same function in tests/memory/array_rows_struct_elem_copy.c states the right value (copy_all: 1, 2, 3, 4 and v[0].a[0] == 1; neighbours: 10; self: 7; same: 70; apart: 140)
#include <cerastes.h>

struct W {
  int a[3];
  int n;
};

int wrong_first_cell(void)
{
  struct W v[2] = {{{1, 2, 3}, 4}, {{5, 6, 7}, 8}};
  v[1] = v[0];
  CERASTES_CHECK(v[1].a[0] == 5);  // <- here
  return v[0].n;
}

int wrong_second_cell(void)
{
  struct W v[2] = {{{1, 2, 3}, 4}, {{5, 6, 7}, 8}};
  v[1] = v[0];
  CERASTES_CHECK(v[1].a[1] == 6);  // <- here
  return v[0].n;
}

int wrong_third_cell(void)
{
  struct W v[2] = {{{1, 2, 3}, 4}, {{5, 6, 7}, 8}};
  v[1] = v[0];
  CERASTES_CHECK(v[1].a[2] == 7);  // <- here
  return v[0].n;
}

int wrong_scalar(void)
{
  struct W v[2] = {{{1, 2, 3}, 4}, {{5, 6, 7}, 8}};
  v[1] = v[0];
  CERASTES_CHECK(v[1].n == 8);  // <- here
  return v[0].n;
}

int wrong_source(void)
{
  struct W v[2] = {{{1, 2, 3}, 4}, {{5, 6, 7}, 8}};
  v[1] = v[0];
  CERASTES_CHECK(v[0].a[0] == 5);  // <- here
  return v[0].n;
}

int wrong_neighbour(void)
{
  struct W v[3] = {{{1, 2, 3}, 4}, {{5, 6, 7}, 8}, {{9, 10, 11}, 12}};
  v[1] = v[0];
  CERASTES_CHECK(v[2].a[1] == 2);  // <- here
  return v[0].n;
}

int wrong_self(int i)
    CERASTES_REQUIRES(0 <= i && i < 2)
    CERASTES_ENSURES(r, r == 2)
{
  struct W v[2] = {{{1, 2, 3}, 4}, {{5, 6, 7}, 8}};
  v[i].a[1] = 7;
  v[i] = v[i];
  return v[i].a[1];  // <- here
}

int wrong_same(int i, int j)
    CERASTES_REQUIRES(0 <= i && i < 2 && i == j)
    CERASTES_ENSURES(r, r == 3)
{
  struct W v[2] = {{{1, 2, 3}, 4}, {{5, 6, 7}, 8}};
  v[i].a[2] = 70;
  v[i] = v[j];
  return v[j].a[2];  // <- here
}

int wrong_apart(int i, int j)
    CERASTES_REQUIRES(0 <= i && i < 2 && 0 <= j && j < 2 && i != j)
    CERASTES_ENSURES(r, r == 70 + 2)
{
  struct W v[2] = {{{1, 2, 3}, 4}, {{5, 6, 7}, 8}};
  v[j].a[1] = 70;
  v[i] = v[j];
  return v[i].a[1] + v[j].a[1];  // <- here
}
