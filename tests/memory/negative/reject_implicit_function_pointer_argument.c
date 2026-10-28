// expect: rejected cerastes-call_without_contract @ L11
// expect: frontend-warning implicit_func_decl @ L11
// c-only: C++ makes the undeclared identifier a front-end error
// twin: tests/memory/write_through_pointer.c
// Pins: audit round 1 of M4.5b1a, 0032 item 4: the address of a cell passed to a function with no declaration (C23 implicit declaration) is rejected: no contract, no permission (re-expected by b1b)
#include <cerastes.h>

int implicit_call(void)
{
  int x = 0;
  undeclared_fn(&x);
  return x;
}
