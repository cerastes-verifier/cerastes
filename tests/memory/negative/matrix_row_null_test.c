// expect: rejected cerastes-array_address_null_test @ L14
// expect: rejected cerastes-array_address_null_test @ L20
// expect: rejected cerastes-array_address_null_test @ L26
// expect: rejected cerastes-array_address_null_test @ L32
// mode: c23, c++20
// twin: tests/memory/matrix_address.c
// Pins: 0064 item 5 (question 322; audit round E1 via edg-backend's report 9: accepted until the encoder's last snapshot): a truth test or a null comparison of a row of a local matrix is `array_address_null_test`, as `a != 0` of a one-dimensional array is: m[0] == 0, m[0] != 0, 0 != m[0] and m[0] != (int *)0, one function per form
// Pre-registered from record 0064 item 5 (Part E), before any run: array_address_null_test at each comparison
#include <cerastes.h>

int eq(void)
{
  int m[2][3] = {{1, 2, 3}, {4, 5, 6}};
  return m[0] == 0;  // <- here
}

int ne(void)
{
  int m[2][3] = {{1, 2, 3}, {4, 5, 6}};
  return m[0] != 0;  // <- here
}

int ne_left(void)
{
  int m[2][3] = {{1, 2, 3}, {4, 5, 6}};
  return 0 != m[0];  // <- here
}

int ne_cast(void)
{
  int m[2][3] = {{1, 2, 3}, {4, 5, 6}};
  return m[0] != (int *)0;  // <- here
}
