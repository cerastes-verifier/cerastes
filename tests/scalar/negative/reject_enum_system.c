// expect: rejected cerastes-unsupported_type @ L8
// mode: c23, c++20
// twin: tests/scalar/enum_switch.c
// Pins: 0018 item 2: idtype_t is an enumeration from a system header; system files are used only through contracts (M8 entry), so its use is rejected
#include <sys/wait.h>
#include <cerastes.h>

int f(idtype_t t)
{
  return 0;
}
