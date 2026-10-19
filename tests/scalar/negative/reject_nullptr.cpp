// expect: rejected cerastes-unsupported_type @ L9
// twin: tests/scalar/if_else.c
// Pins: 0012 item 12: nullptr has a type that is not an integer or bool
#include <cerastes.h>

int nothing(int x)
    CERASTES_REQUIRES(x >= 0 && x < 100)
{
  return x + (nullptr == nullptr);
}
