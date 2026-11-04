// expect: error leak @ L15
// mode: c23, c++20
// tag::source[]
#include <cerastes.h>
#include <stdlib.h>

int stash(int a)
{
  int *p = (int *)malloc(sizeof(int));
  if (p == NULL) {
    return 0;
  }
  *p = a;
  return *p;
}
// end::source[]
