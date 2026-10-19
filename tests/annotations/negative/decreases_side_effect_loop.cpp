// expect: frontend-error cerastes-annotation_side_effect @ L12
// expect: native-ok
// twin: tests/annotations/statements/decreases_while.cpp
// Mutation of the twin: the loop measure assigns.
// Pins: 0006 item 10: an annotation has no side effects (also decreases)
#include <cerastes.h>


int count_up(int n) CERASTES_REQUIRES(n >= 0)
{
  int i = 0;
  CERASTES_DECREASES(i = n - i) while (i < n) {
    i += 1;
  }
  return i;
}
