// expect: rejected cerastes-sizeof_array_argument @ L13
// expect: rejected cerastes-unsupported_annotation @ L17
// expect: rejected cerastes-unsupported_type @ L24
// expect: rejected cerastes-unsupported_type @ L29
// (the former expect row for L44, the loop invariant p[0].a[1] == 4, moved to negative/array_rows_reject_inv_range.c after the first run on j4a: the four rows below are front-end errors that stop the run before the encoder, which draws the L44 row; this comment keeps every line number)
// mode: c23, c++20
// twin: tests/memory/array_rows_param.c
// Pins: 0064 items 1, 2, 10 and 13 (and question 377: a loop invariant that reads an element's array member under a range, p[0].a[1], is rejected by the encoder at the invariant, M4.5e2): sizeof of a row parameter is the size of a pointer (both compilers warn: sizeof_array_argument), a row m[i] as the base of a range in a contract (owns_range(a[1], 3)) is a range whose base is not a pointer parameter (M4.5e2), a leaf path of depth 4 (int d[2][2][2][2]) is rejected (M8), and sizeof(a[0]) of a row parameter inside a CONTRACT is rejected by the encoder's older rule for an array type under sizeof in an annotation (unsupported_type; record item 13 says "in code": it verifies in code, array_rows_spellings.c)
// Pre-registered from record 0064 items 1, 2 and 13 (Part E), before any run (the L44 row from edg-backend report 15's probe b8_inv_range.c: unsupported_expression at the invariant, both modes): rejected (tags by array_param_reject_sizeof.c, range_struct_reject_later.c and item 2; to confirm at the first run); corrected once after the first run on j4a (question 160): the L44 row (question 377, the invariant p[0].a[1] == 4 under owns_range) is the ENCODER's rejection and never runs beside the front end's four errors, so it moved with its function to negative/array_rows_reject_inv_range.c; on j4a the other four rows matched in both modes
#include <cerastes.h>
int param_size(int a[][3])
{
  return (int)sizeof(a);  // <- here
}

void row_base(int a[][3], int n)
    CERASTES_REQUIRES(cerastes_owns_range(a[1], 3) && n > 1)  // <- here
{
  (void)n;
}

int cube(void)
{
  [[maybe_unused]] int d[2][2][2][2] = {};  // <- here
  return 0;
}

int sizeof_contract(int a[][3])
    CERASTES_REQUIRES(sizeof(a[0]) == 12)  // <- here
{
  return 0;
}
