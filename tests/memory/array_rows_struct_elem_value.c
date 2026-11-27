// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 items 12 and 13 (question 309; audit E3a finding 8 (g); the auditor's q28, k08, k10): an element of an array of structures with an array member passed by value (the callee's precondition reads its cells), an element of 7 array cells accepted by value, and the copy v[1] = v[0] of an element of 8 cells (the limit of 7 is for a value passed, returned or written as a literal, not for the copy)
// Pre-registered (test-mutator-10, Task 1/2, before any run): verifies (the right-value siblings of negative/array_rows_struct_elem_value_wrong.c: element passes v[1] (a[2] == 8, n == 5), seven_cells passes v[0] (the listed cells), copy_eight's r == 8 + 9); time it first
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

int element(void)
{
  struct P v[2] = {{1, {2, 3, 4}}, {5, {6, 7, 8}}};
  return byv(v[1]);
}

int seven_cells(void)
{
  struct P7 v[2] = {{{1, 2, 3, 4, 5, 6, 7}, 9}};
  return by7(v[0]);
}

int copy_eight(void)
    CERASTES_ENSURES(r, r == 8 + 9)
{
  struct P8 v[2] = {{{1, 2, 3, 4, 5, 6, 7, 8}, 9}};
  v[1] = v[0];
  return v[1].a[7] + v[1].n;
}
