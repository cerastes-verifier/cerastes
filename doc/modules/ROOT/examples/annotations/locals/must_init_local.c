// expect: rejected wrong_entity_for_attribute @ L10
// mode: c23, c++20
// Rejected by the front end: must_init is a parameter attribute only.
// tag::source[]
#include <cerastes.h>
#include <stdlib.h>

int f(void)
{
  CERASTES_MUST_INIT int *p = NULL;
  return p == NULL;
}
// end::source[]
