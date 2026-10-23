// expect: rejected cerastes-case_not_in_enumeration @ L13
// mode: c23, c++20
// tag::source[]
#include <cerastes.h>

enum color { RED, GREEN, BLUE };

int classify(enum color c)
{
  switch (c) {
  case RED:
    return 1;
  case 4:
    return 2;
  default:
    return 0;
  }
}
// end::source[]
