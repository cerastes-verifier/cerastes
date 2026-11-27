// expect: error check @ L17
// mode: c23, c++20
// twin: tests/memory/array_rows_struct_elem_start.c
// Pins: 0064 item 9 (audit E3a finding 8 (e); the auditor's z01a, measured `check`, both modes): the element the list does not reach, g[1], is zero, so g[1].a[1] == 2 is wrong
// Pre-registered (test-mutator-10, Task 1/2, before any run): one `check`; right-value sibling: g1's checks of tests/memory/array_rows_struct_elem_start.c (g1[1].a[0] == 0 and the rest zero)
#include <cerastes.h>

struct P {
  int n;
  int a[3];
};

static struct P g[3] = {{7, {1, 2, 3}}};

int main(void)
{
  CERASTES_CHECK(g[1].a[1] == 2);  // <- here
  return g[0].n - 7;
}
