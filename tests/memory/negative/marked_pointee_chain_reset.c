// expect: error permission @ L27
// mode: c23, c++20
// twin: tests/memory/marked_pointee_chain_reset_must_init.c
// Pins: 0058 item 4 (audit round 3 B1): correct C that cerastes cannot prove: reset frees the old chain and calls a ref_to_uninit initializer that sets a->b = NULL; the initializer's ensures may not name a->b, so after the call the field is unknown and the closure of a->b is not held: a user error at the return (permission), never exit 3 (the return rows read the nested pointer through a block the function may not hold: B1's fix orders the own rows first)
// Pre-registered from audit round 3 (build/m4.5d2b2d/auditor-report-3.md), NOT RUN on the pre-fix build: one permission error at the closing brace of reset, exit 1 (audit3/q7_correct.c printed an internal error and exit 3 before the fix)
#include <cerastes.h>
#include <stdlib.h>
struct leaf { int *x CERASTES_OWNING; int k; };
struct mid { struct leaf *b CERASTES_OWNING; int m; };
void init(struct mid *a CERASTES_REF_TO_UNINIT)
    CERASTES_REQUIRES(cerastes_owns(*a))
    CERASTES_ENSURES(cerastes_owns(*a))
{
  a->b = NULL;
  a->m = 0;
}
void reset(struct mid *a)
    CERASTES_REQUIRES(cerastes_owns(*a))
    CERASTES_ENSURES(cerastes_owns(*a))
{
  struct leaf *t = a->b;
  if (t != NULL) {
    free(t->x);
    free(t);
  }
  init(a);
}  // <- here
