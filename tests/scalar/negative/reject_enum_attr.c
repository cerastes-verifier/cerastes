// expect: rejected cerastes-unsupported_attribute @ L7
// mode: c23, c++20
// twin: tests/scalar/enum_fixed.c
// Pins: 0018 item 2: an attribute outside the allow-list (0011 item 7) on an enumeration is rejected
#include <cerastes.h>

enum __attribute__((packed)) small { SA, SB };

int ident(int a)
{
  return a;
}
