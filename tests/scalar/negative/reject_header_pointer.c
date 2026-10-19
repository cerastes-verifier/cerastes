// expect: rejected cerastes-unsupported_type @ tests/scalar/headers/bad_pointer.h:L3
// mode: c23, c++20
// twin: tests/scalar/header_use.c
// Pins: 0011 item 4: a rejected construct in a user header is reported in the header
#include <cerastes.h>

#include "../headers/bad_pointer.h"

int use(int x)
    CERASTES_REQUIRES(x >= 0)
{
  return x;
}
