// expect: rejected cerastes-unsupported_annotation @ L17
// mode: c23, c++20
// twin: tests/memory/array_rows_struct_elem_global.c
// Pins: 0064 items 7 and 14 (question 346, listed limit, M4.5e1e2 Part E): cerastes_old of a cell of a member array of an element of a global array of structures, old(g[1].a[2]), is rejected by the encoder at the ensures' line, although the same read outside cerastes_old is accepted (array_rows_struct_elem_global.c). The row for old(gt.v[e].a[e2]), an element of a member array of a global structure, is added once Part E's member form (3c-4) is built: edg-backend report 16 measured it rejected at the same kind of line with the declaration accepted on that build
// Pre-registered from edg-backend report 16's probe old_g.c (edg39, both modes: unsupported_annotation at the ensures, exit 2), before any run of this file
#include <cerastes.h>

struct P {
  int n;
  int a[3];
};

static struct P g[2] = {{7, {1, 2, 3}}, {9, {4, 5, 6}}};

void bump(void)
    CERASTES_REQUIRES(cerastes_owns(g) && g[1].a[2] < 100)
    CERASTES_ENSURES(cerastes_owns(g) && g[1].a[2] == cerastes_old(g[1].a[2]) + 1)  // <- here
{
  g[1].a[2] += 1;
}

int main(void)
{
  bump();
  CERASTES_CHECK(g[1].a[2] == 7);
  return 0;
}
