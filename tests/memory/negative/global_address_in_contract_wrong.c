// expect: error postcondition @ L13
// mode: c23, c++20
// twin: tests/memory/global_address_in_contract.c
// Pins: 0041 item 7: &g and &h are different addresses: r == &h fails for return &g
#include <cerastes.h>

static int g = 3;
static int h = 7;

int *addr(void)
    CERASTES_ENSURES(r, r == &h)
{
  return &g;
}

int main(void)
{
  return h - h;
}
