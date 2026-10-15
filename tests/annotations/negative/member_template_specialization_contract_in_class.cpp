// expect: frontend-error attr_member_template_specialization @ L21
// expect: native-ok
// twin: tests/annotations/contracts/class_template_member_template.cpp
// Mutation of the twin: an explicit specialization of a member template of a class template is refused when the member template has a contract (here, in the class).
// Pins: only this diagnostic is reported (a one-error file, no incidental noise).
#include <cerastes.h>

template <class U>
struct box {
  template <class T> T twice(T v) CERASTES_REQUIRES(v >= 0);
};

template <class U>
template <class T>
T box<U>::twice(T v)
{
  return v + v;
}

template <> template <>
int box<int>::twice<int>(int v)
{
  return v + v;
}

int use()
{
  box<int> b;
  return b.twice(3) + static_cast<int>(b.twice(4L));
}
