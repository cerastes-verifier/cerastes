// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 item 3 and plan section 9, Flips A (E) (the program of the row L40 of the retired staging pin array_reject_matrix_forms.c, which lifted on j4: `return p[i].a[j];` through a pointer to ONE structure with an array member under owns(*p), i == 0 and j == 0): the element p[0] of a pointer to one structure is the structure itself, so its cell reads back the value the requires states
// Pre-registered (test-mutator-10, Task 1/2, before any run): verifies (the program of the retired row; the error twin negative/array_rows_struct_elem_index_one_wrong.c has i == 1; both probed in lane edg, the twin's three rows read from that probe, not predicted)
#include <cerastes.h>

struct T {
  int a[4];
  int n;
};

int index_one(struct T *p, int i, int j)
    CERASTES_REQUIRES(cerastes_owns(*p) && i == 0 && j == 0 && p->a[0] == 7)
    CERASTES_ENSURES(r, cerastes_owns(*p) && r == 7)
{
  return p[i].a[j];
}
