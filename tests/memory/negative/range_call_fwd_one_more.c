// expect: error permission @ L21
// mode: c23, c++20
// twin: tests/memory/range_call_borrowed.c
// Pins: 0060 item 8 (audit S7): forwarding (p, n + 1) of the function's own range: the object may end at p + n, or go on without the callee holding it. The record counts two Silicon errors (the shape row, then the first marked row); the driver prints errors with one text once (report.cpp's unique over rendered errors), and both rows carry the shape row's message, so the test registers ONE permission at the call (to be corrected at the run if the messages differ)
// Pre-registered from record 0060 item 8 (Part C), before the code
#include <cerastes.h>
#include <stddef.h>

void touch(int *p, int n)
    CERASTES_REQUIRES(cerastes_owns_range(p, n))
    CERASTES_ENSURES(cerastes_owns_range(p, n))
{
  (void)p;
  (void)n;
}

void whole(int *p, int n)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && n < 100)
    CERASTES_ENSURES(cerastes_owns_range(p, n))
{
  touch(p, n + 1);
}

void shorter(int *p, int n)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && n > 0)
    CERASTES_ENSURES(cerastes_owns_range(p, n))
{
  touch(p, n - 1);
}

void tail(int *p, int n, int k)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && 0 <= k && k <= n)
    CERASTES_ENSURES(cerastes_owns_range(p, n))
{
  touch(&p[k], n - k);
}
