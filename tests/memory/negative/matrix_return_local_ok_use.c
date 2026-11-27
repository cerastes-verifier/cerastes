// expect: error lifetime @ L18
// expect: error null-dereference @ L19
// mode: c23, c++20
// twin: tests/memory/matrix_return_local_ok.c
// Pins: 0064 item 5 and 0039 item 6 (audit round E1, finding 1; the error twin of the control matrix_return_local_ok.c, the auditor's l09): the pointer a function returns through a variable is not rejected at its return, but it points into a dead matrix: a caller fails `lifetime` where it receives it (the call, L18) and, the callee promising no non-null result, `null-dereference` at the read (two real errors, question 277)
// Pre-registered from record 0064 item 5 (Part E), before any run: lifetime at the read through the returned pointer (first run: lifetime at the call, plus null-dereference at the read: one question-160 correction)
#include <cerastes.h>

int *via_variable(void)
{
  int m[2][3] = {{1, 2, 3}, {4, 5, 6}};
  int *p = m[1];
  return p;
}

int use(void)
{
  int *q = via_variable();  // <- here
  return *q;  // <- here
}
