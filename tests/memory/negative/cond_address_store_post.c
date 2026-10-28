// expect: error postcondition @ L13
// mode: c23, c++20
// twin: tests/memory/cond_address_store.c
// Pins: audit round 2 of M4.5b1a, 0032 items 6, 9: the write goes to the cell the condition names, so the result is 21 when c == 0, not 22
#include <cerastes.h>

int cond_store(int c)
    CERASTES_ENSURES(r, r == (c != 0 ? 11 : 22))
{
  int x = 10;
  int y = 20;
  *(c != 0 ? &x : &y) += 1;
  return c != 0 ? x : y;
}
