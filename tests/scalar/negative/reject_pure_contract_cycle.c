// expect: rejected cerastes-unsupported_annotation @ L12
// mode: c23, c++20
// twin: tests/scalar/pure_tri.c
// Pins: 0062 item 10: a pure function that reaches itself through its own ensures is rejected (M4.5f2), at the application in the clause
// Pre-registered from record 0062 (Part B), before the code: rejected. Corrected before any run (frozen-base probe, build/m4.5e1d/tm2/fr): without a prior declaration the front end refuses f inside its own trailing attributes (error #20 in C++, an implicit declaration in C), so the prototype and a use function were added and the line moved from L10 to L12
#include <cerastes.h>

CERASTES_PURE int f(int n);

CERASTES_PURE int f(int n)
    CERASTES_REQUIRES(0 <= n)
    CERASTES_ENSURES(r, r <= f(n))
{
  return n;
}

int use(int y)
{
  return y;
}
