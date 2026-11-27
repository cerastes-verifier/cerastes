// expect: error out-of-bounds @ L19
// expect: error out-of-bounds @ L19
// expect: error permission @ L19
// mode: c23, c++20
// twin: tests/memory/array_rows_struct_elem_index_one.c
// Pins: 0064 item 3 and plan section 9, Flips A (E): the same program as array_rows_struct_elem_index_one.c with i == 1: the element p[1] of a pointer to ONE structure is outside the object p points into, so the read is `out-of-bounds` twice (the element and the cell's index through it) and `permission` on its line
// Pre-registered (test-mutator-10, Task 1/2, before any run): three rows on one line, registered from a probe of exactly this program in lane edg (a single-file probe outside the budget, both modes: out-of-bounds x2, permission), the right-value sibling being the positive twin
#include <cerastes.h>

struct T {
  int a[4];
  int n;
};

int index_one(struct T *p, int i, int j)
    CERASTES_REQUIRES(cerastes_owns(*p) && i == 1 && j == 0)
    CERASTES_ENSURES(r, cerastes_owns(*p))
{
  return p[i].a[j];  // <- here
}
