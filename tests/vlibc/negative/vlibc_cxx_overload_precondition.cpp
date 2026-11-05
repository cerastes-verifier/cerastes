// expect: error precondition @ L17
// twin: tests/vlibc/vlibc_cxx_overload.cpp
// Pins: 0050 item 8 (audit round 2 S3): the overload's own precondition is checked at the call: it is a program function with an ordinary contract
// Pre-registered from record 0050 item 8: precondition at the call of the overload
#include <vlibc/stdlib.h>
#include <cerastes.h>

int vlibc_malloc(int x)
    CERASTES_REQUIRES(x < 100)
    CERASTES_ENSURES(r, r == x + 1)
{
  return x + 1;
}

int f()
{
  return vlibc_malloc(200);  // <- here
}
