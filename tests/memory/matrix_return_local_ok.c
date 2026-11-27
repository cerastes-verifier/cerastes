// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 item 5 and 0039 item 6 (audit round E1, finding 1; the control of the matrix_return_local_address pins): a pointer VARIABLE that holds the address of a row is not looked through, as `int *p = a; return p;` is accepted on the one-dimensional base (question 65, by design): the function verifies and a caller cannot use the pointer (the object is dead)
// Pre-registered from record 0064 item 5 (Part E), before any run: verifies
#include <cerastes.h>

int *via_variable(void)
{
  int m[2][3] = {{1, 2, 3}, {4, 5, 6}};
  int *p = m[1];
  return p;
}
