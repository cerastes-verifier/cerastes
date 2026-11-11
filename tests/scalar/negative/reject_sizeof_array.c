// expect: rejected cerastes-unsupported_type @ L9
// mode: c23, c++20
// twin: tests/scalar/sizeof_value.c
// Pins: 0024 item 6 (owner, 2026-10-26): array types inside a type operand are rejected until M4.5e1e (array types in sizeof(type), record 0059 item 1)
#include <cerastes.h>

int three_ints(void)
{
  return (int)sizeof(int[3]);
}
