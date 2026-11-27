// expect: error uninitialized @ L26
// mode: c23, c++20
// verify-options: --strict-init
// twin: tests/memory/struct_array_strict_init.c
// Pins: 0064 item 15 under --strict-init: the marked heap structure has a[0..2] written and a[3] read: uninitialized at the read (the array leaf's flags under a marked local)
// Pre-registered from record 0064 item 15 and the precedent heap_read_before_write.c, before any run: uninitialized at the read, under --strict-init (unmeasured)
#include <cerastes.h>
#include <stddef.h>
#include <stdlib.h>

struct S {
  int a[4];
  int n;
};

int heap_fill(void)
    CERASTES_ENSURES(r, r <= 2147483647)
{
  CERASTES_REF_TO_UNINIT struct S *p = (struct S *)malloc(sizeof(struct S));
  if (p == NULL) {
    return -1;
  }
  p->a[0] = 1;
  p->a[1] = 2;
  p->a[2] = 3;
  int v = p->a[3];  // <- here
  free(p);
  return v;
}
