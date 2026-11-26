// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 item 15 (Part D, step B; audit round D2, question 348): the two written atoms next to the rejected ones: owns(p->n) on a CELL of a structure whose array member holds structures (accepted, as on the base), and a guard that reads NO memory, p == NULL || owns(*p), on a moved pointer
// Pre-registered from the auditor's probe audit-D2/q03_348_cell.c (accepted; only its own check(0) fails) and edg-backend report 10 (fx10/ok_guard_noread.c verified); the guarded function is the shape of the lint probe g1_cond (accepted by the encoder, test-mutator report 9), unmeasured under Silicon: verifies in both modes
#include <cerastes.h>
#include <stddef.h>
#include <stdlib.h>

struct S {
  int a[4];
  int n;
};

struct E {
  int x;
  int y;
};

struct SA {
  struct E v[2];
  int n;
};

void cell(struct SA *p)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p))
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 2 && cerastes_owns(p->n))
  CERASTES_DECREASES(2 - i)
  while (i < 2) {
    p->n = i;
    i = i + 1;
  }
}

void guarded(struct S *p, int n)
    CERASTES_REQUIRES((p == NULL || cerastes_owns(*p)) && 0 <= n && n <= 100)
    CERASTES_ENSURES(p == NULL || cerastes_owns(*p))
{
  struct S *q = p;
  p = q;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && (p == NULL || cerastes_owns(*p)))
  CERASTES_DECREASES(n - i)
  while (i < n) {
    if (p != NULL) {
      p->a[1] = i;
    }
    i += 1;
  }
}
