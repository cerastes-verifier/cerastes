// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 item 11 (question 378; audit E3a finding 8 (i); the auditor's s08b): v[0].a[1] = v[1].a[1]++ writes one cell of one element and modifies a cell of another, which is sequenced enough: local, global array and the member array of a global structure verify with the value the increment returned, and so does v[i].a[1] = v[0].a[1]++ when i is known to be 1 (the error twin is negative/array_rows_struct_elem_seq_distinct_unsequenced.c, where i may be 0)
// Pre-registered (test-mutator-10, Task 1/2, before any run): verifies (the positive beside the rejections of negative/array_rows_struct_elem_seq_same_cell.c: the same forms with two elements); time it first
#include <cerastes.h>

struct W {
  int a[4];
  int n;
};

struct T {
  struct W v[2];
  int k;
};

static struct W gv[2] = {{{1, 2, 3, 4}, 5}, {{6, 7, 8, 9}, 10}};
static struct T gt = {{{{1, 2, 3, 4}, 5}, {{6, 7, 8, 9}, 10}}, 11};

int local(void)
    CERASTES_ENSURES(r, r == 7 + 8)
{
  struct W v[2] = {{{1, 2, 3, 4}, 5}, {{6, 7, 8, 9}, 10}};
  v[0].a[1] = v[1].a[1]++;
  return v[0].a[1] + v[1].a[1];
}

int symbolic(int i)
    CERASTES_REQUIRES(i == 1)
{
  struct W v[2] = {{{1, 2, 3, 4}, 5}, {{6, 7, 8, 9}, 10}};
  v[i].a[1] = v[0].a[1]++;
  return v[i].a[1] + v[0].a[1];
}

int main(void)
{
  gv[0].a[1] = gv[1].a[1]++;
  gt.v[0].a[1] = gt.v[1].a[1]++;
  CERASTES_CHECK(gv[0].a[1] == 7 && gv[1].a[1] == 8 && gt.v[0].a[1] == 7 && gt.v[1].a[1] == 8);
  return gv[0].a[1] + gt.v[0].a[1] - 14;
}
