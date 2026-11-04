// expect: rejected cerastes-annotation_reserved_library @ L8
// twin: tests/vlibc/exit_explicit.c
// Pins: 0046 item 6 (audit S2): `namespace n { extern "C" void exit(int); }` declares ::exit ([dcl.link]/6), so its contract is rejected
// The namespace does not make it an ordinary function: C language linkage names the library's.
#include <cerastes.h>

namespace n {
extern "C" void exit(int status) CERASTES_ENSURES(false);
}

int f(int x)
    CERASTES_REQUIRES(0 <= x && x <= 10)
{
  return x;
}
