// expect: frontend-error cerastes-annotation_side_effect @ L9
// An annotation does not change anything.
// tag::bad[]
#include <cerastes.h>

int f(int x)
{
  int y = x;
  CERASTES_CHECK((y = 5) > 0); // <- error
  return y;
}
// end::bad[]
