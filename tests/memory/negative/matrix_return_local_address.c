// expect: rejected cerastes-return_local_address @ L23
// expect: rejected cerastes-return_local_address @ L29
// expect: rejected cerastes-return_local_address @ L35
// expect: rejected cerastes-return_local_address @ L41
// expect: rejected cerastes-return_local_address @ L47
// expect: rejected cerastes-return_local_address @ L53
// expect: rejected cerastes-return_local_address @ L60
// expect: frontend-warning returning_ptr_to_local_variable @ L23
// expect: frontend-warning returning_ptr_to_local_variable @ L29
// expect: frontend-warning expr_has_no_effect @ L47
// expect: frontend-warning returning_ptr_to_local_variable @ L47
// expect: frontend-warning returning_ptr_to_local_variable @ L53
// expect: frontend-warning returning_ptr_to_local_variable @ L60
// mode: c23, c++20
// twin: tests/memory/matrix_return_local_ok.c
// Pins: 0064 item 5 and 0039 item 6 (audit round E1, finding 1; the base's `return a;` and `return &a[1];` of a one-dimensional local, reject_return_local_address.c, for a LOCAL MATRIX): returning a row or the address of a cell of a local matrix is `return_local_address` at the return, one function per form (the first error ends a function): m[1], &m[1][2], a conditional with either arm such an address, a comma, a cast, and a cell at symbolic indexes. The front end prints its own warnings beside it (returning_ptr_to_local_variable at the returns of m[1], &m[1][2], the conditional, the comma and the cast, expr_has_no_effect on the comma's left operand), registered at the first run (question 160, the only correction)
// Pre-registered from record 0064 item 5 (Part E), before any run: return_local_address at each return
#include <cerastes.h>

int *row(int c, int *q)
{
  int m[2][3] = {{1, 2, 3}, {4, 5, 6}};
  return m[1];  // <- here
}

int *cell(void)
{
  int m[2][3] = {{1, 2, 3}, {4, 5, 6}};
  return &m[1][2];  // <- here
}

int *cond_row(int c, int *q)
{
  int m[2][3] = {{1, 2, 3}, {4, 5, 6}};
  return c ? m[1] : q;  // <- here
}

int *cond_cell(int c, int *q)
{
  int m[2][3] = {{1, 2, 3}, {4, 5, 6}};
  return c ? q : &m[1][1];  // <- here
}

int *comma(void)
{
  int m[2][3] = {{1, 2, 3}, {4, 5, 6}};
  return (0, m[0]);  // <- here
}

int *cast(void)
{
  int m[2][3] = {{1, 2, 3}, {4, 5, 6}};
  return (int *)m[1];  // <- here
}

int *symbolic(int i, int j)
    CERASTES_REQUIRES(0 <= i && i < 2 && 0 <= j && j < 3)
{
  int m[2][3] = {{1, 2, 3}, {4, 5, 6}};
  return &m[i][j];  // <- here
}
