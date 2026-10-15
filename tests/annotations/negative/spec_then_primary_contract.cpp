// expect: frontend-error attr_redeclaration_mismatch @ L15
// expect: native-ok
// twin: tests/annotations/contracts/specialization_same.cpp
// Mutation of the twin: the other declaration order: the primary gets a DIFFERENT contract after a specialization already has one (the error is at the later declaration, which names the specialization's line). If the later contract matched the specialization's it would be accepted, so the mutation changes it.
// Pins: only this diagnostic is reported (a one-error file, no incidental noise).
#include <cerastes.h>

template <class T> T twice(T v);

template <> int twice<int>(int v) CERASTES_REQUIRES(v >= 0) CERASTES_ENSURES(r, r >= v)
{
  return v + v;
}

template <class T> T twice(T v) CERASTES_REQUIRES(v > 0);

template <> long twice<long>(long v)
{
  return v + v;
}
