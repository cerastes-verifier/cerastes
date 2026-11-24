// expect: error permission @ L15
// mode: c23, c++20
// twin: tests/memory/pure_argument_reads_memory.c
// Pins: 0062 item 4 (0010 item 11): the argument a[n] reads past the atom's range: the argument's own framing row is the one error (permission, at the ensures), before the application's rows
// Pre-registered from record 0062 (Part C), before the code: permission
#include <cerastes.h>

CERASTES_PURE bool small(int x)
{
  return -100 < x && x < 100;
}

bool last_small(const int *a, int n)
    CERASTES_REQUIRES(1 <= n && cerastes_reads_range(a, n))
    CERASTES_ENSURES(r, cerastes_reads_range(a, n) && r == small(a[n]));

// a bodiless declaration is read only when a verified function calls it (measured on the frozen base): this caller makes the contract above part of the run
bool last_small_caller(const int *a, int n)
    CERASTES_REQUIRES(1 <= n && cerastes_reads_range(a, n))
    CERASTES_ENSURES(r, cerastes_reads_range(a, n))
{
  return last_small(a, n);
}
