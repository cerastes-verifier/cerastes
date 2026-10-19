// expect: frontend-error cerastes-annotation_macro_changed @ L13
// expect: native-ok
// twin: tests/annotations/statements/fold_unfold.cpp
// Mutation of the twin: `fold` is defined as a macro for `unfold`, and CERASTES_FOLD expands through it.
// Pins: plan R4c: annotation_macro_changed for an attribute name that is a macro at the annotation (reported at the #define)
#include <cerastes.h>

CERASTES_PREDICATE bool nonneg(int x)
{
  return x >= 0;
}

#define fold unfold

int use(int i)
{
  CERASTES_FOLD(nonneg(i));
  return i;
}
