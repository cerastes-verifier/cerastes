// expect: rejected cerastes-annotation_reserved_library @ L8
// twin: tests/vlibc/vlibc_cxx_overload.cpp
// Pins: 0050 item 8 (audit round 2 S3): an `extern "C"` declaration of vlibc_free with a contract is the library's function with a program's contract: rejected like the C one
// Pre-registered from record 0050 item 8: rejected annotation_reserved_library at the declaration

#include <cerastes.h>

extern "C" void vlibc_free(void *ptr) CERASTES_REQUIRES(ptr != 0);  // <- here

int f(int x)
    CERASTES_REQUIRES(0 <= x && x <= 10)
{
  return x;
}
