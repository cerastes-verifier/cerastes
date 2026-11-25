// expect: rejected cerastes-unsupported_type @ L33
// expect: rejected cerastes-unsupported_type @ L39
// expect: rejected cerastes-unsupported_annotation @ L45
// mode: c23, c++20
// twin: tests/memory/array_struct_access.c
// Pins: 0064 items 1 and 2 (lifted by M4.8): an array whose element structure has an owning member and one whose element structure has a marked (ref_to_uninit) member are rejected as unsupported_type (item 2), and owning, ref_to_uninit and uninit on an array of aggregates as unsupported_annotation (item 2 as revised by audit round 1: uninit stays accepted on a local array of scalars, 0059 item 11); the first two tags are item 2's wording, the third's is the revised item's
// Pre-registered from record 0064 items 1 and 2 (Part B), before any run: rejected
#include <cerastes.h>
#include <stddef.h>

struct S {
  int x;
  int y;
};

struct N {
  CERASTES_OWNING struct S *p;
  int v;
};

struct R {
  int a;
  int *p CERASTES_REF_TO_UNINIT;
};

struct P {
  int x;
  int y;
};

int owning_element(void)
{
  [[maybe_unused]] struct N v[2];  // <- here
  return 0;
}

int marked_element(void)
{
  [[maybe_unused]] struct R v[2];  // <- here
  return 0;
}

int uninit_array(void)
{
  [[maybe_unused]] CERASTES_UNINIT struct P v[2];  // <- here
  return 0;
}
