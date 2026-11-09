// expect: verified
// expect: native-ok
// mode: c23, c++20
// A global marked ref_to_uninit (trailing position): the marker says the pointee may hold no value yet, and the function writes before it reads.
// tag::source[]
#include <cerastes.h>
#include <stdlib.h>

static int *g CERASTES_REF_TO_UNINIT = NULL;

int main(void)
{
  g = (int *)malloc(sizeof(int));
  if (g == NULL) {
    return 1;
  }
  *g = 3;
  CERASTES_CHECK(*g == 3);
  free(g);
  g = NULL;
  return 0;
}
// end::source[]
