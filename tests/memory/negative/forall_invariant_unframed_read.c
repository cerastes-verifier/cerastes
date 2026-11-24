// expect: error permission @ L18
// mode: c23, c++20
// twin: tests/memory/forall_fill_range.c
// Pins: 0061 item 9 (twin): the invariant reads b[k], which this function owns no part of (no atom names b): the framing row of the quantified read fails (permission: this loop invariant reads `b[i]`, which this loop does not hold); the entry and back-edge rows are the one line, printed once
// Pre-registered from record 0061 item 9 (Part C), before the code. Re-expected on the orchestrator's instruction (audit round 3 note 7, il-analyst-3's flag line guarded by the framing): one error, `permission`, as its scalar analogue range_loop_invariant_element_unframed.c; the first version also expected `uninitialized` (the flag row of the unframed read), question 254's second case, which the guard removes
#include <cerastes.h>
#include <stddef.h>

CERASTES_PURE bool zero_at(int k, const int *p, int m)
{
  return !(0 <= k && k < m) || p[k] == 0;
}

void unframed(const int *b, int n)
    CERASTES_REQUIRES(n >= 0)
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && cerastes_forall(zero_at, b, i))
  CERASTES_DECREASES(n - i)
  while (i < n) {
    i++;
  }
}
