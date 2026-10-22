// expect: rejected cerastes-unsupported_expression @ L10
// c-only: in C++ P_ALL has the system enumeration's type, so the use is unsupported_type (reject_enum_system.c)
// twin: tests/scalar/enum_switch.c
// Pins: 0018 item 2: P_ALL is an enumerator from a system header; its use in code is rejected
#include <sys/wait.h>
#include <cerastes.h>

int f(void)
{
  return P_ALL;
}
