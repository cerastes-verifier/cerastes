// expect: rejected cerastes-unsupported_expression @ L18
// mode: c23, c++20
// twin: tests/memory/array_rows_struct_elem.c
// Pins: 0064 item 10 and question 377 (M4.5e2): a loop invariant that reads an element's array member of a range, `p[0].a[1] == 4` under `cerastes_owns_range(p, n)` with `struct P { int n; int a[3]; }`, is rejected by the encoder at the invariant ("the element of a pointer to a structure in a loop invariant or measure other than the read of one of its members through a local pointer, p[e].x"); the accepted form is the read through a local pointer (array_rows_struct_elem.c, `loop_local`). Moved here from negative/array_rows_reject_forms.c, whose four front-end rows stop the run before the encoder
// Pre-registered from edg-backend report 15's probe b8_inv_range.c (unsupported_expression at the invariant, both modes) and from this instance's single-file probe on j4a (lane work, build/tm8probe/inv_a.c: the same one error at the invariant line, both modes, 0 s), before any run of this file
#include <cerastes.h>

struct P {
  int n;
  int a[3];
};

int inv_range(struct P *p, int n)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && n > 1 && p[0].a[1] == 4)
    CERASTES_ENSURES(r, cerastes_owns_range(p, n))
{
  int k = 0;
  CERASTES_INVARIANT(0 <= k && k <= 2 && p[0].a[1] == 4)  // <- here
  CERASTES_DECREASES(2 - k)
  while (k < 2) {
    k = k + 1;
  }
  return k;
}
