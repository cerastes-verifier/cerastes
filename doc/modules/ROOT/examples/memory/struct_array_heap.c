// expect: verified
// expect: native-ok
// mode: c23, c++20
// A structure with an array member on the heap: malloc gives the whole object with its elements uninitialized, each element is
// written before it is read, and free ends the object.
// tag::source[]
#include <cerastes.h>
#include <stdlib.h>

struct buffer {
  int data[4];
  int len;
};

int main(void)
{
  struct buffer *b = (struct buffer *)malloc(sizeof(struct buffer));
  if (b == NULL) {
    return 1;
  }
  b->data[0] = 10;
  b->data[1] = 20;
  b->data[2] = 30;
  b->data[3] = 40;
  b->len = 4;
  CERASTES_CHECK(b->data[2] == 30 && b->len == 4);
  int sum = b->data[0] + b->data[3];
  free(b);
  return sum == 50 ? 0 : 1;
}
// end::source[]
