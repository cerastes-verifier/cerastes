// expect: golden
// expect: native-ok
// Pins: a member template of a class template with a contract in the class and
// an out-of-class definition that has none (it has the declaration's); the
// instances for int and long carry it. Twin of the member_template_specialization_*
// negatives, where an explicit specialization of that member template is
// refused (decision 0007 item 4).
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

int use()
{
  box<int> b;
  return b.twice(3) + static_cast<int>(b.twice(4L));
}
