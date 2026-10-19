// expect: frontend-error cerastes-in_class_specialization_contract @ L9
// An explicit specialization of a member template written inside its class: write it at namespace scope.
// tag::bad[]
#include <cerastes.h>

template <class T>
struct Outer {
  template <class U> static int f(U y) CERASTES_REQUIRES(y > 0) { return 0; }
  template <> int f<int>(int y) { return 1; } // <- error
};
// end::bad[]
