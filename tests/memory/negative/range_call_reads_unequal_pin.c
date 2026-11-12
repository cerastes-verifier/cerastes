// expect: error permission @ L29
// mode: c23, c++20
// twin: tests/memory/range_call_reads.c
// Pins: PIN of question 238 (0060 item 6, the remaining limit; audit round 2, N3): keep_slot keeps a fraction of its element (reads(*x)) and gives the liveness cell back (a partnerless live(x); it needs a body, 0052 item 2a). Called twice on &p[1], it leaves slot 1 at a quarter of what the function holds while the liveness cell is whole again, so the amount the minimum takes (min of the first slot, the liveness cell and the function's own amount, halved) is more than slot 1 holds: the marked row of the reads_range call fails, a false permission at the call (a smaller amount would pass). The function promises nothing back (no ensures), so the program is otherwise correct. With a local array the same calls end in a lifetime error at the scope exit (a kept fraction), which is a real error: only the parameter form shows the limit. Found with a probe at the joined proof of Part C: the first accepted program that reaches it
// Pre-registered from record 0060 item 6 (Part C), before the code
#include <cerastes.h>
#include <stddef.h>

int peek(const int *p, int n)
    CERASTES_REQUIRES(cerastes_reads_range(p, n) && n > 0)
    CERASTES_ENSURES(r, cerastes_reads_range(p, n) && r == p[0])
{
  (void)n;
  return p[0];
}

void keep_slot(const int *x)
    CERASTES_REQUIRES(cerastes_reads(*x))
    CERASTES_ENSURES(cerastes_live(x))
{
  (void)x;
}

void pinned(const int *p, int n)
    CERASTES_REQUIRES(cerastes_reads_range(p, n) && n > 1)
{
  keep_slot(&p[1]);
  keep_slot(&p[1]);
  peek(p, n);
}
