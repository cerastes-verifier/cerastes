// expect: frontend-error cerastes-annotation_macro_changed @ L8
// expect: native-ok
// twin: tests/annotations/spec_functions/spec_calls.cpp
// Mutation of the twin: `reads` is defined as a macro for `owns`, and an annotation calls cerastes::reads.
// Pins: plan R4c: annotation_macro_changed for a spec function name that is a macro at the annotation (reported at the #define, where the IL records it)
#include <cerastes.h>

#define reads owns

struct cell {
  int val;
};

int get(const cell* c) CERASTES_REQUIRES(cerastes::reads(c->val))
{
  return c->val;
}
