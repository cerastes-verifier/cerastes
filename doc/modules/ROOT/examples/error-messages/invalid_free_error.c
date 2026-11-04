// expect: error invalid-free @ L15
// mode: c23, c++20
// tag::source[]
#include <cerastes.h>
#include <stdlib.h>

void reset_twice(void)
{
  int *p = (int *)malloc(sizeof(int));
  if (p == NULL) {
    return;
  }
  *p = 0;
  free(p);
  free(p);
}
// end::source[]
