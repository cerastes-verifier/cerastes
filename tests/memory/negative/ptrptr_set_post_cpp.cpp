// expect: error postcondition @ L13
// mode: c++20
// twin: tests/memory/ptrptr_set_cpp.cpp
// Pins: 0040 item 2: the contract says *pp keeps its old value, the body stores q
#include <cerastes.h>
#include <stddef.h>

void set(int **pp, int *q)
    CERASTES_REQUIRES(cerastes::owns(*pp) && cerastes::live(q))
    CERASTES_ENSURES(cerastes::owns(*pp) && *pp == cerastes::old(*pp) && cerastes::live(q))
{
  *pp = q;
}
