// expect: error precondition @ L36
// expect: error precondition @ L42
// expect: error postcondition @ L50
// mode: c23, c++20
// twin: tests/memory/array_rows_struct_elem_value.c
// Pins: 0064 items 12 and 13 (audit E3a finding 8 (g); the auditor's q28b, k08b, k10b, measured `precondition` at the call and `postcondition`, both modes): v[0] is not the element the callee's precondition describes (its a[2] is 4, its n 1), v[1] of the seven-cell array is the zero element (its a[6] is 0, not 7), and the copied eighth cell is 8, not 0
// Pre-registered (test-mutator-10, Task 1/2, before any run): two `precondition` rows at the calls, one `postcondition` at the return; right-value siblings: element, seven_cells and copy_eight in tests/memory/array_rows_struct_elem_value.c
#include <cerastes.h>

struct P {
  int n;
  int a[3];
};

struct P7 {
  int a[7];
  int n;
};

struct P8 {
  int a[8];
  int n;
};

int byv(struct P s)
    CERASTES_REQUIRES(s.a[2] == 8 && s.n == 5)
    CERASTES_ENSURES(r, r == 0);

int by7(struct P7 s)
    CERASTES_REQUIRES(s.a[6] == 7 && s.n == 9)
    CERASTES_ENSURES(r, r == 0);

int wrong_element(void)
{
  struct P v[2] = {{1, {2, 3, 4}}, {5, {6, 7, 8}}};
  return byv(v[0]);  // <- here
}

int wrong_seven_cells(void)
{
  struct P7 v[2] = {{{1, 2, 3, 4, 5, 6, 7}, 9}};
  return by7(v[1]);  // <- here
}

int wrong_copy_eight(void)
    CERASTES_ENSURES(r, r == 0)
{
  struct P8 v[2] = {{{1, 2, 3, 4, 5, 6, 7, 8}, 9}};
  v[1] = v[0];
  return v[1].a[7];  // <- here
}
