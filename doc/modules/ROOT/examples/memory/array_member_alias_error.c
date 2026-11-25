// expect: error permission @ L24
// mode: c23, c++20
// An element of the array member is not the structure: a function that owns both cannot be given a structure
// and the address of one of its elements.
// tag::source[]
#include <cerastes.h>

struct buffer {
  int data[8];
  int len;
};

void both(struct buffer *b, int *x)
    CERASTES_REQUIRES(cerastes_owns(*b) && cerastes_owns(*x))
    CERASTES_ENSURES(cerastes_owns(*b) && cerastes_owns(*x))
{
  b->len = 0;
  *x = 0;
}

int main(void)
{
  struct buffer b = {{1, 2, 3}, 3};
  both(&b, &b.data[1]);
  return b.len;
}
// end::source[]
