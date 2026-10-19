// expect: frontend-error cerastes-cerastes_header_in_namespace @ L13
// The specification functions must be cerastes::owns and the like: include <cerastes.h> at global scope.
// tag::bad[]
namespace foo {
#include <cerastes.h>
}

struct cell {
  int val;
};

void set(cell* c, int v)
  CERASTES_REQUIRES(foo::cerastes::owns(c->val)) // <- error
{
  c->val = v;
}
// end::bad[]
