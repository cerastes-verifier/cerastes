// expect: frontend-error cerastes-old_outside_ensures @ L12
// expect: native-ok
// twin: tests/annotations/contracts/never_instantiated.cpp
// Mutation of the twin: an unqualified old(v) after a using-directive for cerastes in an enclosing NAMESPACE.
// Pins: CP7 regression — spec functions brought in anywhere but file scope were missed; dependent names fail closed.
#include <cerastes.h>

namespace N {
using namespace cerastes;

template <class T>
T twice(T v) CERASTES_REQUIRES(old(v) > 0) CERASTES_ENSURES(r, r > v)
{
  return v + v;
}
}
