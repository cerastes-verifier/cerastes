// expect: error conversion @ L35
// mode: c23, c++20
// twin: tests/scalar/char_literal_const.c
// Pins: 0024 item 5 and 0017 item 5: char_literal_const.c with classify's requires x == 97 and the label '\x80': the case values are checked in the prologue, not on the dispatch path
#include <cerastes.h>

const char K = '\x7f';
const int LF = '\n';

enum { TAB = '\t', DEL = '\x7f' };

int k_value(void)
    CERASTES_ENSURES(r, r == 127)
{
  return K;
}

int lf_value(void)
    CERASTES_ENSURES(r, r == 10)
{
  return LF;
}

int enumerator_sum(void)
    CERASTES_ENSURES(r, r == 136)
{
  return TAB + DEL;
}

int classify(int x)
    CERASTES_REQUIRES(x == 97)
    CERASTES_ENSURES(r, r == 2)
{
  switch (x) {
  case '\x80':
    return 1;
  case 'a':
    return 2;
  default:
    return 0;
  }
}

int pinned(int x)
    CERASTES_REQUIRES(x == '\x7f')
    CERASTES_ENSURES(r, r == 127)
{
  return x;
}
