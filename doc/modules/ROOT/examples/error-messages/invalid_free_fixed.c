// expect: verified
// expect: native-ok
// mode: c23, c++20
// The same program, fixed: the allocation is freed once.
// tag::source[]
#include <cerastes.h>
#include <stdlib.h>

void reset_once(void)
{
  int *p = (int *)malloc(sizeof(int));
  if (p == NULL) {
    return;
  }
  *p = 0;
  free(p);
}
// end::source[]
