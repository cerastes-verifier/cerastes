// expect: verified
// expect: native-ok
// mode: c23
// c-only: vlibc_malloc creates no C++ objects; cerastes rejects it in C++ (decision 0050 item 7)
// tag::source[]
#include <cerastes.h>
#include <vlibc/stdlib.h>

int main(void)
{
  int *p = (int *)vlibc_malloc(sizeof(int));   // vlibc's heap
  int *q = (int *)malloc(sizeof(int));         // the platform's heap
  if (p == NULL || q == NULL) {
    vlibc_free(p);
    free(q);
    return 1;
  }
  *p = 1;
  *q = 2;
  int sum = *p + *q;
  vlibc_free(p);   // each block is freed by its own allocator
  free(q);
  return sum == 3 ? 0 : 1;
}
// end::source[]
