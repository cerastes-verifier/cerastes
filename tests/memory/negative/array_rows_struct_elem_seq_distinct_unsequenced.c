// expect: error unsequenced @ L17
// mode: c23, c++20
// twin: tests/memory/array_rows_struct_elem_seq_distinct.c
// Pins: 0064 item 11 (question 378; audit E3a finding 8 (i); the auditor's s08e, measured `unsequenced`, both modes): v[i].a[1] = v[0].a[1]++ with i that may be 0 writes and modifies one cell of one element with no sequence point between them (the lowering's path-precise row; the static form v[0].a[1] = v[0].a[1]++ is rejected in negative/array_rows_struct_elem_seq_same_cell.c)
// Pre-registered (test-mutator-10, Task 1/2, before any run): one `unsequenced` at the statement; right-value sibling: symbolic in tests/memory/array_rows_struct_elem_seq_distinct.c (the same statement with i == 1)
#include <cerastes.h>

struct W {
  int a[4];
  int n;
};

int symbolic(int i)
    CERASTES_REQUIRES(0 <= i && i < 2)
{
  struct W v[2] = {{{1, 2, 3, 4}, 5}, {{6, 7, 8, 9}, 10}};
  v[i].a[1] = v[0].a[1]++;  // <- here
  return v[i].a[1] + v[0].a[1];
}
