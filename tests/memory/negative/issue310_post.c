// expect: error postcondition @ L19
// mode: c23, c++20
// twin: tests/memory/verifast/issue310.c
// Pins: 0045 item 3 and the port of VeriFast's tests/issue310.c: the ensures says d - 2 where the body subtracts 1: postcondition at the closing brace
#include <cerastes.h>

typedef struct s {
  int data;
} s_data;

void foo(s_data **ps)
    CERASTES_REQUIRES(cerastes_owns(*ps) && cerastes_owns((*ps)->data) && 0 <= (*ps)->data)
    CERASTES_ENSURES(cerastes_owns(*ps) && *ps == cerastes_old(*ps) && cerastes_owns((*ps)->data) && (*ps)->data == cerastes_old((*ps)->data) - 2)
{
  s_data **e;
  e = ps;
  s_data **f = e;
  (*f)->data -= 1;
}
