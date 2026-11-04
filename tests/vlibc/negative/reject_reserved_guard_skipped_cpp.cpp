// expect: rejected cerastes-annotation_reserved_library @ third_party/vlibc/include/vlibc/stdlib.h:L30
// twin: tests/vlibc/exit_explicit.c
// Pins: 0046 item 4 (audit round 2, B1): with the platform's guards defined (the SDK's `_STDLIB_H_`, glibc's `_STDLIB_H`, libc++'s `_LIBCPP_STDLIB_H`) vlibc's declaration is the first and has C++ linkage, which is not the library's exit: its contract is rejected, never trusted
// The report is in vlibc's header (where the contract is written), at its `exit` declaration; the program's call is fine in itself.
#define _STDLIB_H_
#define _STDLIB_H
#define _LIBCPP_STDLIB_H
#include <stdlib.h>
#include <cerastes.h>

int f(int x)
    CERASTES_REQUIRES(0 <= x && x <= 10)
    CERASTES_ENSURES(r, r == 1)
{
  exit(x);
  return 0;
}
