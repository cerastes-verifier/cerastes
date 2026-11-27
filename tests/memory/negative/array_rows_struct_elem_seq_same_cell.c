// expect: rejected cerastes-unsequenced_modification @ L27
// expect: rejected cerastes-unsequenced_modification @ L33
// expect: rejected cerastes-unsequenced_modification @ L41
// expect: rejected cerastes-unsequenced_modification @ L47
// mode: c23, c++20
// twin: tests/memory/array_rows_struct_elem_seq_distinct.c
// Pins: 0064 item 11 and 0039 item 3 (question 378; the Sequencer's key gains the element's index, the member and the cell's index; the il-analyst's rejection cases e1e2e_sarr_rej_seq_same_cell, e1e2e_rej_gsarr_seq_same_cell, e1e2e_rej_tsarr_seq_same_cell and e1e2e_rej_gtsarr_seq_same_cell; audit E3a finding 8 (i)): v[0].a[1] = v[0].a[1]++ writes and reads one cell of one element of a named array with no sequence point between them: rejected like its siblings (a[1] = a[1]++, s.a[1] = s.a[1]++), for a local array, a global array, a member array of a local structure and of a global structure (the range form p[0].a[1] keeps the lowering's `unsequenced` verification error, whose pin is the existing array_rows_struct_elem_seq_unsequenced.c)
// Pre-registered (test-mutator-10, Task 1/2, before any run): one rejection per function at the statement; twin: array_rows_struct_elem_seq_distinct.c (the same forms with two different elements, which verify); one probe on the final build confirms the tag and the lines
#include <cerastes.h>

struct W {
  int a[4];
  int n;
};

struct T {
  struct W v[2];
  int k;
};

static struct W gv[2];
static struct T gt;

int local(void)
{
  struct W v[2] = {{{1, 2, 3, 4}, 5}, {{6, 7, 8, 9}, 10}};
  v[0].a[1] = v[0].a[1]++;  // <- here
  return v[0].a[1];
}

int global(void)
{
  gv[0].a[1] = gv[0].a[1]++;  // <- here
  return gv[0].a[1];
}

int member(void)
{
  struct T t;
  t.k = 0;
  t.v[0].a[1] = t.v[0].a[1]++;  // <- here
  return t.v[0].a[1];
}

int global_member(void)
{
  gt.v[0].a[1] = gt.v[0].a[1]++;  // <- here
  return gt.v[0].a[1];
}
