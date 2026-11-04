// expect: rejected cerastes-unsupported_attribute @ L9
// twin: tests/vlibc/abort_shim.c
// Pins: 0046 item 7 (question 118): the same for abort in C++ (C linkage written, the alias target too): a user-written alias on a routine the SDK declares is outside the allow-list
// abort's calls would reach my_abort, which returns.
#include <stdlib.h>
#include <cerastes.h>

extern "C" void my_abort(void) { }
extern "C" void abort(void) __attribute__((alias("my_abort")));

int f(int x)
    CERASTES_REQUIRES(0 <= x && x <= 10)
    CERASTES_ENSURES(r, r == 1)
{
  abort();
  return 0;
}
