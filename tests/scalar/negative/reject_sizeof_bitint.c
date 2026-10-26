// expect: rejected cerastes-unsupported_type @ L10
// mode: c23
// twin: tests/scalar/sizeof_value.c
// c-only: _BitInt is C23
// Pins: 0024 item 6 (cut 3 candidate): a _BitInt type in a type operand; reject_bitint.c covers a _BitInt declaration
#include <cerastes.h>

int bits(void)
{
  return (int)sizeof(unsigned _BitInt(8));
}
