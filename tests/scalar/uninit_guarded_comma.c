// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0025 item 1 (finding A): the neighbours of the uninit_guarded_* tests that verify (the comma, && and ?: shapes): x holds a value when the guarded operand reads it, and the value decides g's argument (g accepts only 0)
#include <cerastes.h>

// the guard of the discarded call: g accepts only 0, and the guard is false because x is 0 where it is read
int g(int v)
    CERASTES_REQUIRES(v == 0)
    CERASTES_ENSURES(r, r == v);

int f(int a)
    CERASTES_ENSURES(r, r == a);

// x is initialized, so reading it in the guarded operand is fine
int initialized(int a)
    CERASTES_ENSURES(r, r == 4)
{
  int x = 0;
  int y = (x = (g(f(a) > 0 && x > 0), 2));
  return y + x;
}

// x is written before the guarded read
int written_first(int a)
    CERASTES_ENSURES(r, r == 4)
{
  int x;
  x = 0;
  int y = (x = (g(f(a) > 0 && x > 0), 2));
  return y + x;
}

// the read follows the write in the same expression
int written_in_comma(int a)
    CERASTES_ENSURES(r, r == 2)
{
  int x;
  int y = (x = 0, g(f(a) > 0 && x > 0), x + 2);
  return y;
}

// the && shape of uninit_guarded_and_write.c with x initialized: the guard reads 0 and stays false
int and_initialized(int a)
    CERASTES_ENSURES(r, r == 11)
{
  int x = 0;
  int y = ((g(f(a) > 0 && x > 0), 1) && ((x = 1) == 1));
  return 10 * y + x;
}

// the ?: shape of uninit_guarded_cond_write.c with x initialized: the arm reads 0, which g accepts
int cond_initialized(int a)
    CERASTES_ENSURES(r, r == 4)
{
  int x = 0;
  int y = (x = (g(f(a) > 0 ? x : 0), 2));
  return y + x;
}
