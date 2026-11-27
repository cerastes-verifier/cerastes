// expect: error uninitialized @ L19
// mode: c23, c++20
// An element of the array member is read before anything wrote it: malloc leaves the elements uninitialized.
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
  int first = b->data[1];
  free(b);
  return first;
}
// end::source[]
