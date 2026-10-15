// expect: golden
// expect: native-ok
// Pins: a named result on a member function template defined out of class: the
// declaration in the class carries no contract, the definition has ensures(r, p)
// with the declared return type T; the instance for int shows it with int.
// Twin of result_name_void_instance, where make<void>() gives the instance a
// void result.
#include <cerastes.h>

struct maker {
  template <class T> T make();
};

template <class T> T maker::make() CERASTES_ENSURES(r, true)
{
  return T();
}

int use()
{
  maker m;
  return m.make<int>();
}
