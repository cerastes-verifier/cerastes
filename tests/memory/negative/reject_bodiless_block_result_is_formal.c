// expect: rejected cerastes-unsupported_annotation @ L9
// mode: c23, c++20
// twin: tests/memory/result_bodiless.c
// Pins: 0052 item 2a (audit B3, the d2a hole): ensures(r, owns_block(r) && r == p) in a bodiless contract ties the result to a pointer the caller already holds, so the block is held twice and the double free after the call would verify: the whole program is rejected at the declaration
// Pre-registered from record 0052 item 2a: rejected unsupported_annotation at the declaration (no verification happens: the double free must never be accepted)
#include <cerastes.h>
#include <stdlib.h>

int *again(int *p) CERASTES_REQUIRES(cerastes_live(p)) CERASTES_ENSURES(r, cerastes_owns_block(r) && r == p);  // <- here

int twice(void)
{
  int *p = (int *)malloc(sizeof(int));
  if (p == NULL) {
    return -1;
  }
  int *q = again(p);
  free(q);
  free(p);
  return 0;
}
