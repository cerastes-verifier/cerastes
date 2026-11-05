// expect: verified
// expect: native-ok
// Pins: 0050 item 8 (audit round 2 S3): a program's own `int vlibc_malloc(int)` has C++ linkage and is an overload, not the library's function: it may carry a contract and be called (the reservation is of the extern "C" function)
// Pre-registered from record 0050 item 8: verifies
#include <vlibc/stdlib.h>
#include <cerastes.h>

int vlibc_malloc(int x)
    CERASTES_REQUIRES(x < 100)
    CERASTES_ENSURES(r, r == x + 1)
{
  return x + 1;
}

int f()
    CERASTES_ENSURES(r, r == 3)
{
  return vlibc_malloc(2);
}
