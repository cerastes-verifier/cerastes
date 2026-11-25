// expect: verified
// expect: native-ok
// mode: c23, c++20
// A structure with an array member is copied whole (`b = a`: a copy has no limit on the elements), and a smaller one is passed by
// value (at most 7 array elements): a change to the copy (`b`, or the callee's parameter) leaves the original alone.
// tag::source[]
#include <cerastes.h>

struct buffer {
  int data[8];
  int len;
};

struct small {
  int data[2];
  int len;
};

int zero_first(struct small s)
    CERASTES_REQUIRES(s.data[0] == 1 && s.len == 2)
    CERASTES_ENSURES(r, r == 0)
{
  s.data[0] = 0;
  return s.data[0];
}

int main(void)
{
  struct buffer a = {{1, 2, 3}, 3};
  struct buffer b;
  b = a;
  CERASTES_CHECK(b.data[2] == 3 && b.data[7] == 0 && b.len == 3);
  b.data[2] = 9;
  CERASTES_CHECK(a.data[2] == 3);
  struct small m = {{1, 2}, 2};
  int t = zero_first(m);
  CERASTES_CHECK(m.data[0] == 1 && t == 0);
  return b.len - 3 + t;
}
// end::source[]
