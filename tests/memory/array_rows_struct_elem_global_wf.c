// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 items 7 and 14 (question 376; audit E3a finding 8 (d); the auditor's t06): the two states of g[e].a[e2] of a global array of structures with an array member are tied under a reads site: the guard g[0].a[1] != 0 of the requires makes the division of the ensures safe
// Pre-registered (test-mutator-10, Task 1/2, before any run): verifies (the control of negative/array_rows_struct_elem_global_wf_division.c: the twins fail because the tie is absent under owns(g) or the cell differs; this is the same shape under reads(g) with the same cell)
#include <cerastes.h>

struct P {
  int n;
  int a[3];
};

static struct P g[2] = {{1, {2, 3, 4}}, {5, {6, 7, 8}}};

int tied(void)
    CERASTES_REQUIRES(cerastes_reads(g) && g[0].a[1] != 0)
    CERASTES_ENSURES(r, cerastes_reads(g) && (r == 0 || r == 10 / g[0].a[1]))
{
  return g[1].n * 0;
}
