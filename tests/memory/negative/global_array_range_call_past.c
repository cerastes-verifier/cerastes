// expect: error permission @ L21
// mode: c23, c++20
// twin: tests/memory/global_array_range_call.c
// Pins: 0063 item 8 (0060 item 8, range_call_reads_too_long.c): a count of 5 for the four-element array global: the shape row fails at the call
// Pre-registered from record 0063 item 8 (Part B), before any run: the one error
#include <cerastes.h>
#include <stddef.h>

static int g[4] = {1, 2, 3, 4};

int peek(const int *p, int n)
    CERASTES_REQUIRES(cerastes_reads_range(p, n) && n > 0)
    CERASTES_ENSURES(r, cerastes_reads_range(p, n) && r == p[0])
{
  (void)n;
  return p[0];
}

int main(void)
{
  return peek(g, 5) - 1;  // <- here
}
