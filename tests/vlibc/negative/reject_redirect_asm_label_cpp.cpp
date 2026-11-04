// expect: rejected cerastes-unsupported_declaration @ L9
// twin: tests/vlibc/abort_shim.c
// Pins: 0046 item 8 (audit B1): the same for abort in C++ with C linkage written
// The label binds abort to my_abort.
#include <stdlib.h>
#include <cerastes.h>

extern "C" void my_abort(void) { }
extern "C" void abort(void) __asm__("my_abort");

int f(int x)
    CERASTES_REQUIRES(0 <= x && x <= 10)
    CERASTES_ENSURES(r, r == 1)
{
  abort();
  return 0;
}
