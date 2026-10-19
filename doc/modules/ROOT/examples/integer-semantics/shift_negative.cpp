// expect: error shift @ L9
// Left shift of a negative value is an error in both modes.
// tag::source[]
#include <cerastes.h>

int shift_up(int a)
    CERASTES_REQUIRES(a == -1)
{
  return a << 1;
}
// end::source[]
