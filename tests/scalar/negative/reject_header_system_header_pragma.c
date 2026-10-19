// expect: rejected cerastes-system_header_pragma_in_verify @ tests/scalar/headers/system_header_pragma.h:L2
// mode: c23, c++20
// twin: tests/scalar/header_use.c
// Pins: 0011 item 4: the pragma in a user header is rejected, reported in the header
#include <cerastes.h>

#include "../headers/system_header_pragma.h"

int use(int x)
    CERASTES_REQUIRES(x >= 0)
{
  return x;
}
