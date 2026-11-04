// expect: verified
// expect: native-ok
// mode: c23, c++20
// Port of VeriFast's tests/issue310.c (13 lines; ~/Documents/verifast at 0fb63506) to cerastes (decision 0045, M4.5c2b).
// Adaptation: `//@ requires *ps |-> ?s &*& s->data |-> ?d &*& 0 <= d; ensures *ps |-> s &*& s->data |-> d - 1;` becomes `cerastes_owns(*ps) && cerastes_owns((*ps)->data) && 0 <= (*ps)->data` and `cerastes_owns(*ps) && *ps == cerastes_old(*ps) && cerastes_owns((*ps)->data) && (*ps)->data == cerastes_old((*ps)->data) - 1` (the ghost `s` is the tie `*ps == old(*ps)`, `d` is `old((*ps)->data)`); the typedef, the locals `e` and `f` and the body are as written.
// Left out: nothing.
// Line map (this file : VeriFast's):  10 : 1   11 : 2   12 : 3   14 : 5   15 : 6   16 : 7   17 : 8   18 : 9   19 : 10   20 : 11   21 : 12   22 : 13
#include <cerastes.h>

typedef struct s {
  int data;
} s_data;

void foo(s_data **ps)
    CERASTES_REQUIRES(cerastes_owns(*ps) && cerastes_owns((*ps)->data) && 0 <= (*ps)->data)
    CERASTES_ENSURES(cerastes_owns(*ps) && *ps == cerastes_old(*ps) && cerastes_owns((*ps)->data) && (*ps)->data == cerastes_old((*ps)->data) - 1)
{
  s_data **e;
  e = ps;
  s_data **f = e;
  (*f)->data -= 1;
}
