// expect: error check @ L17
// mode: c23, c++20
// twin: tests/memory/array_rows_struct_elem_start.c
// Pins: 0064 item 9 (audit E3a finding 8 (e); the auditor's z03c with z02's list, measured `check`, both modes): the cells a partial list leaves out of an element's array member are zero, so g[1].a[2] == 5 is wrong
// Pre-registered (test-mutator-10, Task 1/2, before any run): one `check`; right-value sibling: g2's check of tests/memory/array_rows_struct_elem_start.c (g2[1].a[2] == 0, the same list)
#include <cerastes.h>

struct P {
  int n;
  int a[3];
};

static struct P g[3] = {{7, {1}}, {8, {4, 5}}};

int main(void)
{
  CERASTES_CHECK(g[1].a[2] == 5);  // <- here
  return g[0].n - 7;
}
