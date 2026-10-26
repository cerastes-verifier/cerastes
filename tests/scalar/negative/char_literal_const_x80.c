// expect: error conversion @ L7
// mode: c23, c++20
// twin: tests/scalar/char_literal_const.c
// Pins: 0024 item 5: char_literal_const.c with K = '\x80': the constant's initializer is checked in the prologue of every method, reported once at the declaration
#include <cerastes.h>

const char K = '\x80';
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
    CERASTES_ENSURES(r, (x == 127 && r == 1) || (x == 97 && r == 2) || (x != 127 && x != 97 && r == 0))
{
  switch (x) {
  case '\x7f':
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
