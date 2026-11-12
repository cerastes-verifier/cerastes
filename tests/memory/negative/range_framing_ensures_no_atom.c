// expect: error permission @ L11
// expect: error postcondition @ L14
// mode: c23, c++20
// twin: tests/memory/range_framing_range.c
// Pins: 0060 item 5: the post-state read p[0] is framed only by an atom of the ensures: the requires' atom is not enough (0037 item 7, audit B2's shape for a range); in a function with a body the unframed read's dummy value also fails the postcondition at the return (0040 item 4's precedent)
// Pre-registered from record 0060 item 5, before the code
#include <cerastes.h>

int first(int *p, int n)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && n > 0 && p[0] > 0)
    CERASTES_ENSURES(r, r == p[0])
{
  (void)n;
  return p[0];
}

int last(int *p, int n)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && n > 0 && p[n - 1] > 0)
    CERASTES_ENSURES(r, cerastes_owns_range(p, n) && r == p[n - 1])
{
  return p[n - 1];
}
