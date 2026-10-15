// expect: frontend-error cerastes-old_outside_ensures @ L11
// expect: native-ok
// twin: tests/annotations/contracts/never_instantiated.cpp
// Mutation of the twin: X::old(v) where namespace X has `using namespace cerastes;`.
// Pins: CP7 regression — qualified lookup through a using-directive reaches cerastes::old.
#include <cerastes.h>

namespace X { using namespace cerastes; }

template <class T>
T twice(T v) CERASTES_REQUIRES(X::old(v) > 0) CERASTES_ENSURES(r, r > v)
{
  return v + v;
}
