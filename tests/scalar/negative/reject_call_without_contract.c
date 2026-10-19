// expect: rejected cerastes-call_without_contract @ L12
// mode: c23, c++20
// twin: tests/scalar/call_precondition.c
// Pins: 0011 item 5: a call of a function with no body and no contract is rejected
// The twin gives the callee a contract.
#include <cerastes.h>

int external(int x);

int use(int y)
{
  return external(y);
}
