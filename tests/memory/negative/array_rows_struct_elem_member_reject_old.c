// expect: rejected cerastes-unsupported_annotation @ L21
// mode: c23, c++20
// twin: tests/memory/array_rows_struct_elem_global.c
// Pins: 0064 items 7 and 14 (question 346, listed limit, M4.5e1e2 Part E): cerastes_old of a cell of the member array of an element of a MEMBER array of structures of a global structure, old(gt.v[e].a[e2]) with struct T { struct W v[2]; int k; }, is rejected by the encoder at the ensures' line (edg-backend's probe old_gt.c printed exactly that error, one, at that line, on edg39 and edg40; the probe shows nothing about the plain read outside cerastes_old, and the member form of a LOCAL structure stops at check.cpp's backstop on those builds). The sibling pin is negative/array_rows_struct_elem_reject_old.c (the global array of structures with an array member, old(g[1].a[2])); both rest on the open MISMATCH of edg-backend's report 16 (no record item names these two forms; the record names old(gt.v[e].x) and old(gt.m[e][e2]))
// Pre-registered from edg-backend report 16's probe old_gt.c (edg39 and edg40, both modes: unsupported_annotation at the ensures, exit 2), before any run of this file
#include <cerastes.h>

struct W {
  int a[3];
  int n;
};
struct T {
  struct W v[2];
  int k;
};

static struct T gt = {{{{1, 2, 3}, 4}, {{5, 6, 7}, 8}}, 9};

void bump(void)
    CERASTES_REQUIRES(cerastes_owns(gt) && gt.v[1].a[2] < 100)
    CERASTES_ENSURES(cerastes_owns(gt) && gt.v[1].a[2] == cerastes_old(gt.v[1].a[2]) + 1)  // <- here
{
  gt.v[1].a[2] += 1;
}

int main(void)
{
  bump();
  CERASTES_CHECK(gt.v[1].a[2] == 8);
  return 0;
}
