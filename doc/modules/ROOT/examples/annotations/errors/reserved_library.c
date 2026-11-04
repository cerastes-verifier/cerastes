// expect: rejected cerastes-annotation_reserved_library @ L6
// exit's contract comes from vlibc's <stdlib.h> only: a program cannot give it one of its own, even `ensures(false)`.
// tag::bad[]
#include <cerastes.h>

void exit(int status) CERASTES_ENSURES(false); // <- error
// end::bad[]

int use(int x)
    CERASTES_REQUIRES(0 <= x && x <= 10)
{
  return x;
}
