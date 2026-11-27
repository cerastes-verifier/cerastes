// expect: error permission @ L23
// mode: c23, c++20
// twin: tests/memory/struct_array_owner_param.c
// Pins: 0064 item 15 (0040 item 3): keep consumes the structure (no ensures), so owning's ensures p == NULL || owns(*p) fails at the end of pass_keep: permission at the closing brace (the precedent negative/owning_param_kept.c)
// Pre-registered from record 0064 item 15 and the precedent owning_param_kept.c (permission at the closing brace), before any run: unmeasured for an array leaf
#include <cerastes.h>
#include <stddef.h>
#include <stdlib.h>

struct S {
  int a[4];
  int n;
};

void keep(struct S *p)
    CERASTES_REQUIRES(cerastes_owns(*p));

void pass_keep(CERASTES_OWNING struct S *p)
{
  if (p != NULL) {
    keep(p);
  }
}  // <- here
