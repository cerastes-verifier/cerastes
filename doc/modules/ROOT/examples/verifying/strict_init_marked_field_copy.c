// expect: verified
// expect: native-ok
// mode: c23, c++20
// verify-options: --strict-init
// A copy and a by-value pass move a marked member into the same marked member of the same type, which the rule accepts.
// tag::source[]
#include <cerastes.h>

struct cursor {
  int *target CERASTES_REF_TO_UNINIT;
  int n;
};

int count(struct cursor s)
{
  return s.n;
}

int main(void)
{
  struct cursor a = { 0, 1 };
  struct cursor b = a;
  struct cursor c = { 0, 2 };
  c = b;
  int k = count(c);
  (void)k;
  return 0;
}
// end::source[]
