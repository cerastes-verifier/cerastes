// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 item 11 and handoff section 3 (the sequencing rows of a call on a heap structure with an array member have nothing behind them): the call that owns the structure, then a read in the next statement; and a reads call beside a read of an element it reads, both accepted. The twins in negative/struct_array_heap_seq_unsequenced.c put a read of the element, of another member, or a second call into the expression
// Pre-registered from record 0064 item 11 and item 15 (Part D), before any run: verifies (unmeasured)
#include <cerastes.h>
#include <stddef.h>
#include <stdlib.h>

struct S {
  int a[4];
  int n;
};

int wr(struct S *p)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(r, cerastes_owns(*p) && p->a[1] == 7 && r == 0)
{
  p->a[1] = 7;
  return 0;
}

int rd(struct S *p)
    CERASTES_REQUIRES(cerastes_reads(*p))
    CERASTES_ENSURES(r, cerastes_reads(*p) && (r == 0 || r == 1))
{
  return p->a[1] == p->a[2];
}

int sequenced(void)
    CERASTES_ENSURES(r, r == 7 || r == -1)
{
  struct S *p = (struct S *)malloc(sizeof(struct S));
  if (p == NULL) {
    return -1;
  }
  p->a[0] = 1;
  p->a[1] = 2;
  p->a[2] = 3;
  p->a[3] = 4;
  p->n = 4;
  int s = wr(p);
  s = s + p->a[1];
  free(p);
  return s;
}

int reads_beside(void)
    CERASTES_ENSURES(r, (0 <= r && r <= 2) || r == -1)
{
  struct S *p = (struct S *)malloc(sizeof(struct S));
  if (p == NULL) {
    return -1;
  }
  p->a[0] = 1;
  p->a[1] = 2;
  p->a[2] = 3;
  p->a[3] = 4;
  p->n = 4;
  int s = rd(p) + (p->a[1] == p->a[2]);
  free(p);
  return s;
}
