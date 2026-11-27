// expect: error check @ L18
// mode: c23, c++20
// twin: tests/memory/array_rows_struct_elem_start_two_members.c
// Pins: 0064 item 9 and 0063 item 9 (initial values by linkage; audit E3a finding 8 (e); the auditor's y03, measured `check`, both modes): an array of structures with external linkage and no initializer is not known to be zero at the start of main (the unit is not the whole program), so the check of g[1].a[0] == 0 fails; the same array with internal linkage is zero (the control)
// Pre-registered (test-mutator-10, Task 1/2, before any run): one `check`, unknown by design; control: g4 (the same type, static) in tests/memory/array_rows_struct_elem_start_two_members.c verifies its zero cells
#include <cerastes.h>

struct Q {
  int a[2];
  int n;
  int b[2];
};

struct Q g[2];

int main(void)
{
  CERASTES_CHECK(g[1].a[0] == 0);  // <- here
  return g[0].n;
}
