// expect: rejected cerastes-unsupported_annotation @ L9
// mode: c++20
// twin: tests/memory/owns_inc.c
// Pins: 0037 items 5 and 15: forall over a pointer's elements is rejected (lifted by M7 for the lambda form written here; M4.5e1c gives C's forall over a pure function, record 0059 item 1)
#include <cerastes.h>
#include <stddef.h>

int first(const int *a, int n)
    CERASTES_REQUIRES(n > 0 && cerastes::forall([](int i, const int *b, int m) { return !(0 <= i && i < m) || b[i] >= 0; }, a, n))
{
  return n;
}
