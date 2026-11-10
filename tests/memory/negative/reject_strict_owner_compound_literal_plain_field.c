// expect: rejected cerastes-owner_result_unowned @ L26
// c-only: the address of a compound literal is unsupported_expression in C++ (record 0044 item 7), with or without the option
// verify-options: --strict-owner
// twin: tests/memory/strict_owner_p4_compound_literal.c
// Pins: 0058 item 3 (p4): a call that gives its block in the leaf of a compound literal whose field is not owning: owner_result_unowned at the call
// Pre-registered from record 0058 item 3: rejected owner_result_unowned at the call of make (c23)
#include <cerastes.h>
#include <stdlib.h>

CERASTES_OWNING int *make(void)
{
  CERASTES_OWNING int *a = (int *)malloc(sizeof(int));
  if (a != NULL) {
    *a = 0;
  }
  return a;
}

struct plain {
  int *data;
  int n;
};

int f(void)
{
  struct plain *c = &(struct plain){ make(), 0 };  // <- here
  return c->n;
}
