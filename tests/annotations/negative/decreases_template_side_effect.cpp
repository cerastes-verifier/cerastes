// expect: frontend-error cerastes-annotation_side_effect @ L12
// expect: native-ok
// twin: tests/annotations/contracts/decreases_template.cpp
// Mutation of the twin: the measure of a function template decrements its parameter.
// Pins: 0006 item 10 in a template: reported where it is decidable (the prototype), once
#include <cerastes.h>


template <class T>
T down(T n)
    CERASTES_REQUIRES(n >= 0)
    CERASTES_DECREASES(n--)
{
  return n == 0 ? n : down(n - 1);
}

int use(int n) CERASTES_REQUIRES(n >= 0) { return down(n); }
