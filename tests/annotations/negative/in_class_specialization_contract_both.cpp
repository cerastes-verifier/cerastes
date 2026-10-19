// expect: frontend-error cerastes-in_class_specialization_contract @ L10
// twin: tests/annotations/contracts/class_template_member_template.cpp
// Mutation of the twin: an in-class explicit specialization of a member template, in a class template that is never instantiated; the primary and the specialization carry different contracts.
// Pins: plan R2: in_class_specialization_contract (auditor probe k4); assumption: reported at the specialization (INDEX-B A2)
#include <cerastes.h>


template <class T> struct Outer {
  template <class U> static int f(U y) CERASTES_REQUIRES(y > 0) { return 0; }
  template <> int f<int>(int y) CERASTES_REQUIRES(y > 5) { return 1; }
};
