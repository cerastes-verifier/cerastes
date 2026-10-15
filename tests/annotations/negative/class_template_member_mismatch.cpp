// expect: frontend-error attr_redeclaration_mismatch @ L16
// expect: native-ok
// twin: tests/annotations/contracts/class_template_member.cpp
// Mutation of the twin: an out-of-class class-template member definition whose ensures differs from the in-class one.
// Pins: only this diagnostic is reported (a one-error file, no incidental noise).
#include <cerastes.h>

template <class T>
struct box {
  T v;
  T get() const CERASTES_REQUIRES(v >= 0) { return v; }
  T twice() const CERASTES_ENSURES(r, r >= v);
};

template <class T>
T box<T>::twice() const CERASTES_ENSURES(r, r > v)
{
  return v + v;
}

int use(const box<int>& b)
{
  return b.get() + b.twice();
}
