// expect: rejected cerastes-unsupported_annotation @ L14
// mode: c23, c++20
// twin: tests/memory/heap_loop_param_block.c
// Pins: 0054 item 10 (0047 item 4): a block site in a loop's termination measure holds no permission and stays rejected ("a measure holds no permission atom")
// Pre-registered from record 0054 item 10 and owner question 182 (8): rejected unsupported_annotation at the measure clause (the in-progress encoder already says so)
#include <cerastes.h>

void f(int *p, int n)
    CERASTES_REQUIRES(cerastes_owns_block(p) && 0 <= n && n <= 100)
    CERASTES_ENSURES(cerastes_owns_block(p))
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i + (cerastes_owns_block(p) ? 1 : 0))  // <- here
  while (i < n) {
    i += 1;
  }
}
