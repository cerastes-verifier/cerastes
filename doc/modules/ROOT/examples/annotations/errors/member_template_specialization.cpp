// expect: frontend-error attr_member_template_specialization @ L19
// An explicit specialization of a member template whose primary has a contract.
// tag::bad[]
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
int box<int>::twice<int>(int v) // <- error
{
  return v + v;
}
// end::bad[]
