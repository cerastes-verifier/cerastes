// expect: rejected cerastes-unsupported_annotation @ L9
// mode: c++20
// twin: tests/memory/owns_inc.c
// Pins: 0037 items 5 and 15: owns_range over a pointer is rejected (re-expected by M4.5e1)
#include <cerastes.h>
#include <stddef.h>

int first(const int *p)
    CERASTES_REQUIRES(cerastes::owns_range(p, 2))
{
  return *p;
}
