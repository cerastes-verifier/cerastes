// expect: frontend-error cerastes-cerastes_header_in_namespace @ L15
// expect: native-ok
// twin: tests/annotations/spec_functions/spec_calls.cpp
// Mutation of the twin: <cerastes.h> is included inside a namespace, so the spec functions are foo::cerastes::owns.
// Pins: plan R4d: cerastes_header_in_namespace (the spec functions of the real header declared outside ::cerastes); assumption: reported at the annotation that names one (INDEX-B A1)
namespace foo {
#include <cerastes.h>
}

struct cell {
  int val;
};

void set(cell* c, int v)
  CERASTES_REQUIRES(foo::cerastes::owns(c->val))
{
  c->val = v;
}
