// expect: error overflow @ L8
// mode: c23, c++20
// twin: tests/scalar/sizeof_value.c
// Pins: 0063 item 14 (flip B of M4.5e1e1, was `rejected unsupported_type` by 0024 item 6): sizeof of an array type in an ensures, its wrapping bound read as written: an overflow at the bound (the front end says 1); Pre-registered from record 0063 item 14 (Part D), before any run
#include <cerastes.h>

int one(void)
    CERASTES_ENSURES(r, r == (int)sizeof(char[65536u * 65536u + 1]))
{
  return 1;
}
