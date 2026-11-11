// expect: rejected cerastes-unsupported_annotation @ L8
// mode: c23, c++20
// ref_to_uninit on a member that is not a pointer to an object is rejected.
// tag::source[]
#include <cerastes.h>

struct cursor {
  int target CERASTES_REF_TO_UNINIT;
  int n;
};

int main(void)
{
  return 0;
}
// end::source[]
