// expect: error leak @ L29
// c-only: the address of a compound literal is unsupported_expression in C++ (record 0044 item 7), with or without the option
// verify-options: --strict-owner
// twin: tests/memory/strict_owner_p4_compound_literal.c
// Pins: 0058 item 3 (p4): the option only rejects more: the compound literal's block is never freed: the default mode's leak, reported where the literal's lifetime ends
// Pre-registered from record 0058 item 3: one leak at the return of f under --strict-owner (c23); the line is a best reading (question 160)
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

CERASTES_OWNING int *make(void)
{
  CERASTES_OWNING int *a = (int *)malloc(sizeof(int));
  if (a != NULL) {
    *a = 0;
  }
  return a;
}

int f(void)
    CERASTES_ENSURES(r, r == 0)
{
  struct buf *c = &(struct buf){ make(), 0 };
  (void)c;
  return 0;  // <- here
}
