// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0043 item 8, 0041 item 4, audit round 2 S2: a struct-pointer global of unknown value (external linkage): program_start's row states WF and the guarded WF_S; main assigns it and never reads the unknown value
#include <cerastes.h>
#include <stddef.h>

struct S {
  int x;
  int y;
};

struct S *gp;

int main(void)
{
  gp = NULL;
  return 0;
}
