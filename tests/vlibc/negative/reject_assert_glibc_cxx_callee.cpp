// expect: rejected cerastes-call_without_contract @ L14
// mode: c++20; platform: linux
// twin: tests/scalar/assert_holds.c
// Pins: 0031 item 5: the failing arm of glibc's C++ shape must call __assert_fail; a user's reporting function does not make an assertion
// glibc's C++ expansion is `(static_cast<bool>(e) ? void(0) : __assert_fail(...))`.
#include <assert.h>
#include <cerastes.h>

void report_failure(const char *what);

int shape(int x)
    CERASTES_REQUIRES(0 <= x && x <= 10)
{
  (static_cast<bool>(x >= 0) ? void(0) : report_failure("x >= 0"));
  return x;
}
