// expect: frontend-error attr_redeclaration_mismatch @ L10
// expect: native-ok
// twin: tests/annotations/contracts/specialization_same.cpp
// Mutation of the twin: an explicit specialization whose requires differs from the primary's.
// Pins: only this diagnostic is reported (a one-error file, no incidental noise).
#include <cerastes.h>

template <class T> T twice(T v) CERASTES_REQUIRES(v >= 0) CERASTES_ENSURES(r, r >= v);

template <> int twice<int>(int v) CERASTES_REQUIRES(v > 0) CERASTES_ENSURES(r, r >= v)
{
  return v + v;
}

template <> long twice<long>(long v)
{
  return v + v;
}
