// expect: verified
// expect: native-ok
// mode: c23, c++20
// verify-options: --strict-init
// An initializer list or a compound literal gives a marked member a null pointer or an allocation, checked per member.
// tag::source[]
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING CERASTES_REF_TO_UNINIT;
  int n;
};

struct cursor {
  int *target CERASTES_REF_TO_UNINIT;
  int n;
};

int main(void)
{
  struct buf a = { NULL, 1 };
  struct buf b = { (int *)malloc(sizeof(int)), 2 };
  struct buf c = { 0, 3 };
  struct cursor d = (struct cursor){ NULL, 3 };
  free(a.data);
  free(b.data);
  free(c.data);
  return a.n + b.n + c.n + d.n - 9;
}
// end::source[]
