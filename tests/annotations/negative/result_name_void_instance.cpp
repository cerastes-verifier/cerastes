// expect: frontend-error attr_result_name_void_function @ L12
// expect: native-ok
// twin: tests/annotations/contracts/template_member_result.cpp
// Mutation of the twin: a named result on the void instance of an out-of-class member template definition (found per instance, at the contract's line, with the instantiation context).
// Pins: only this diagnostic is reported (a one-error file, no incidental noise).
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
  m.make<void>();
  return m.make<int>();
}
