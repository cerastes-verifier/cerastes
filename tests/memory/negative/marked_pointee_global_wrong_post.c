// expect: error postcondition @ L19
// mode: c23, c++20
// twin: tests/memory/marked_pointee_global.c
// Pins: 0058 item 4 (question 202): the same program with a wrong ensures
// Pre-registered from record 0058 item 4 (question 202): one postcondition error at the return (observed on the Part F build with question 202 applied)
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

struct buf *gp CERASTES_OWNING CERASTES_REF_TO_UNINIT;

int f(void)
    CERASTES_ENSURES(r, r == 2)
{
  return 1;  // <- here
}

int main(void)
{
  return 0;
}
