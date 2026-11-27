// expect: rejected cerastes-unsupported_type @ L13
// mode: c23, c++20
// twin: tests/memory/array_rows_struct_elem_value.c
// Pins: 0064 item 12 and 0063 item 13 (question 309; audit E3a finding 8 (g); the auditor's k09, measured `cerastes-unsupported_type` at the declaration, both modes): a structure of 8 array cells passed by value is rejected (the limit is 7: array values are M4.5f3), at the declaration of the function whose parameter it is
// Pre-registered (test-mutator-10, Task 1/2, before any run): one rejection at the declaration of by8; twin: seven_cells in tests/memory/array_rows_struct_elem_value.c (7 cells accepted)
#include <cerastes.h>

struct P8 {
  int a[8];
  int n;
};

int by8(struct P8 s)  // <- here
    CERASTES_REQUIRES(s.n == 9)
    CERASTES_ENSURES(r, r == 0);

int eight_cells(void)
{
  struct P8 v[2] = {{{1, 2, 3, 4, 5, 6, 7, 8}, 9}};
  return by8(v[0]);
}
