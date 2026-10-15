// expect: golden
// expect: native-ok
// What a parameter means in ensures: by value, by reference, through a pointer.
#include <cerastes.h>

// tag::by_value[]
int bump(int x) CERASTES_ENSURES(r, r == x + 1)
{
  x = x + 1; // the contract still talks about the argument that was passed
  return x;
}
// end::by_value[]

// tag::by_reference[]
void incr(int& x) CERASTES_REQUIRES(x < 1000) CERASTES_ENSURES(x == cerastes::old(x) + 1)
{
  x += 1;
}
// end::by_reference[]

// tag::by_pointer[]
void incr_ptr(int* p) CERASTES_REQUIRES(*p < 1000) CERASTES_ENSURES(*p == cerastes::old(*p) + 1)
{
  *p += 1;
}
// end::by_pointer[]
