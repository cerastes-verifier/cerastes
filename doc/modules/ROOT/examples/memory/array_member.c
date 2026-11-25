// expect: verified
// expect: native-ok
// mode: c23, c++20
// A structure with an array member: a braced initializer with a list of its own, a callee that owns the whole
// structure writes an element and the length, and one that owns the array member alone writes an element.
// tag::source[]
#include <cerastes.h>

struct buffer {
  int data[8];
  int len;
};

void put(struct buffer *b, int i, int v)
    CERASTES_REQUIRES(cerastes_owns(*b) && 0 <= i && i < 8)
    CERASTES_ENSURES(cerastes_owns(*b) && b->data[i] == v && b->len == i + 1)
{
  b->data[i] = v;
  b->len = i + 1;
}

void zero_first(struct buffer *b)
    CERASTES_REQUIRES(cerastes_owns(b->data))
    CERASTES_ENSURES(cerastes_owns(b->data) && b->data[0] == 0)
{
  b->data[0] = 0;
}

int main(void)
{
  struct buffer b = {{1, 2, 3}, 3};
  CERASTES_CHECK(b.data[2] == 3 && b.data[3] == 0 && b.len == 3);
  put(&b, 5, 9);
  CERASTES_CHECK(b.data[5] == 9 && b.len == 6);
  zero_first(&b);
  CERASTES_CHECK(b.data[0] == 0 && b.len == 6);
  return 0;
}
// end::source[]
