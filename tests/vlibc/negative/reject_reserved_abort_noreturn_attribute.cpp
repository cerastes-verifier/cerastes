// expect: rejected cerastes-annotation_reserved_library @ L7
// twin: tests/vlibc/abort_shim.c
// Pins: 0046 item 6: a written `[[noreturn]]` on a reserved name with C linkage, in C++, is rejected
// abort()'s noreturn is vlibc's `ensures(false)`.
#include <cerastes.h>

extern "C" [[noreturn]] void abort();

int f(int x)
    CERASTES_REQUIRES(0 <= x && x <= 10)
{
  return x;
}
