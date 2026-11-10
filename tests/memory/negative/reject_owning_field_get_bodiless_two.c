// expect: rejected cerastes-unsupported_annotation @ L14
// mode: c23, c++20
// twin: tests/memory/owning_field_get_bodiless.c
// Pins: 0055 items 3, 11 (S2): the same with two owning fields: nothing proves the two fresh blocks distinct, and the contract has no body in this unit, so it is rejected, naming the contract manifest (M6)
// Pre-registered from audit round 1 (record 0055 as revised, 78838ca), rejected unsupported_annotation at the declaration (a one-line declaration, so the line is the clause's; a caller, `user`, is there because a bodiless declaration nothing calls is never encoded); the message is the bodiless-result rule's (as reject_owning_field_bodiless_result.c)
#include <cerastes.h>
#include <stdlib.h>

struct two {
  int *a CERASTES_OWNING;
  int *b CERASTES_OWNING;
};

struct two *get(void) CERASTES_ENSURES(r, r == NULL || cerastes_owns(*r));  // <- here

void user(void)
{
  struct two *s = get();
  if (s != NULL) {
    free(s->a);
    free(s->b);
  }
}
