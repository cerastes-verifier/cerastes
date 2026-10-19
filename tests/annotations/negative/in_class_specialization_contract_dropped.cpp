// expect: frontend-error cerastes-in_class_specialization_contract @ L10
// twin: tests/annotations/contracts/class_template_member_template.cpp
// Mutation of the twin: an in-class explicit specialization has no contract while the primary has one; class template never instantiated.
// Pins: plan R2: in_class_specialization_contract (auditor probe k6)
#include <cerastes.h>


template <class T> struct Outer {
  template <class U> static int f(U y) CERASTES_REQUIRES(y > 0) { return 0; }
  template <> int f<int>(int y) { return 1; }
};
