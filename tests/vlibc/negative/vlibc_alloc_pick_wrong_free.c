// expect: error invalid-free @ L25
// mode: c23
// c-only: vlibc_malloc creates no C++ objects (record 0050 item 7)
// twin: tests/vlibc/vlibc_alloc_pick.c
// Pins: 0050 item 5 (audit round 2 B1): the block reached through a bodiless function's result is a vlibc block, and the platform's free of it is invalid-free: the value row of the free names the allocator
// Pre-registered from il-analyst's B1 fix (M4.5d2v): exactly one invalid-free at the free(r) ("frees `r` with `free`, but it may be a block of vlibc's allocator")
#include <vlibc/stdlib.h>
#include <cerastes.h>

int *pick(int *a, int *b, int c)
    CERASTES_ENSURES(r, (c != 0 && r == a) || (c == 0 && r == b));

int f(int c)
{
  int *p = (int *)vlibc_malloc(sizeof(int));
  if (p == NULL) {
    return -1;
  }
  int *q = (int *)vlibc_malloc(sizeof(int));
  if (q == NULL) {
    vlibc_free(p);
    return -1;
  }
  int *r = pick(p, q, c);
  free(r);  // <- here
  if (c != 0) {
    vlibc_free(q);
  } else {
    vlibc_free(p);
  }
  return 0;
}
