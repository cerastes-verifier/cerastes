// expect: error division-by-zero @ L22
// expect: error division-by-zero @ L30
// expect: error division-by-zero @ L37
// expect: error division-by-zero @ L44
// expect: error division-by-zero @ L51
// mode: c23, c++20
// twin: tests/memory/array_rows_struct_elem_global_wf.c
// Pins: 0064 items 7 and 14 (question 376; audit E3a finding 8 (d); the auditor's t06b, t06c, t06d, t06e, t06f, measured `division-by-zero` at the ensures, both modes): the tie of g[e].a[e2] is only under a reads site (under owns(g) the post-state cell is not the entry cell, written or not: written and untouched; the second is question 345's listed false alarm shape for owns), its key holds the element and the cell (g[1].a[1] is not g[0].a[1]), and another global's or a parameter's range's cell is not tied to it
// Pre-registered (test-mutator-10, Task 1/2, before any run): `division-by-zero` at the ensures line of each function (no verdict is about a value: the control is tied in tests/memory/array_rows_struct_elem_global_wf.c, the same shape under reads(g) and the same cell)
#include <cerastes.h>

struct P {
  int n;
  int a[3];
};

static struct P g[2] = {{1, {2, 3, 4}}, {5, {6, 7, 8}}};
static struct P h[2] = {{1, {2, 3, 4}}, {5, {6, 7, 8}}};

int written(void)
    CERASTES_REQUIRES(cerastes_owns(g) && g[0].a[1] != 0)
    CERASTES_ENSURES(r, cerastes_owns(g) && (r == 0 || r == 10 / g[0].a[1]))  // <- here
{
  g[0].a[1] = 0;
  return 0;
}

int untouched(void)
    CERASTES_REQUIRES(cerastes_owns(g) && g[0].a[1] != 0)
    CERASTES_ENSURES(r, cerastes_owns(g) && (r == 0 || r == 10 / g[0].a[1]))  // <- here
{
  return g[1].n * 0;
}

int other_element(void)
    CERASTES_REQUIRES(cerastes_reads(g) && g[0].a[1] != 0)
    CERASTES_ENSURES(r, cerastes_reads(g) && (r == 0 || r == 10 / g[1].a[1]))  // <- here
{
  return g[1].n * 0;
}

int other_global(void)
    CERASTES_REQUIRES(cerastes_reads(g) && cerastes_owns(h) && g[0].a[1] != 0)
    CERASTES_ENSURES(r, cerastes_reads(g) && cerastes_owns(h) && (r == 0 || r == 10 / h[0].a[1]))  // <- here
{
  return g[1].n * 0 + h[1].n * 0;
}

int other_range(struct P *p, int n)
    CERASTES_REQUIRES(cerastes_reads(g) && cerastes_owns_range(p, n) && n > 1 && g[0].a[1] != 0)
    CERASTES_ENSURES(r, cerastes_reads(g) && cerastes_owns_range(p, n) && (r == 0 || r == 10 / p[0].a[1]))  // <- here
{
  (void)n;
  return g[1].n * 0 + p[0].n * 0;
}
