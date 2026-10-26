// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0025 item 3 (question 30): a void ?: in statement position (statement, discarded comma operand, arms of every shape) is an if; each arm's checks hold on its own path
#include <assert.h>
#include <cerastes.h>

void pos(int x)
    CERASTES_REQUIRES(x > 0)
{
  (void)x;
}

void neg(int y)
    CERASTES_REQUIRES(y < 0)
{
  (void)y;
}

int id(int a)
    CERASTES_ENSURES(r, r == a)
{
  return a;
}

// a conditional expression statement with void arms is the if it abbreviates
int statement(int c, int x, int y)
    CERASTES_REQUIRES(c ? x > 0 : y < 0)
    CERASTES_ENSURES(r, r == c)
{
  c ? pos(x) : neg(y);
  return c;
}

// the same as the discarded operand of a comma
int in_comma(int c, int x, int y)
    CERASTES_REQUIRES(c ? x > 0 : y < 0)
    CERASTES_ENSURES(r, r == 1)
{
  int z = 0;
  (c ? pos(x) : neg(y)), z = 1;
  return z;
}

// an arm that is (void)0 is an empty statement
int empty_arm(int c, int x)
    CERASTES_REQUIRES(c == 0 || x > 0)
    CERASTES_ENSURES(r, r == c)
{
  c ? pos(x) : (void)0;
  return c;
}

int empty_first_arm(int c, int y)
    CERASTES_REQUIRES(c != 0 || y < 0)
    CERASTES_ENSURES(r, r == c)
{
  c ? (void)0 : neg(y);
  return c;
}

// an arm that is a conditional of void arms again
int nested(int c, int d, int x, int y)
    CERASTES_REQUIRES(0 <= c && c <= 1000 && 0 <= d && d <= 1000 && (c == 0 || (d ? x > 0 : y < 0)))
    CERASTES_ENSURES(r, r == c + d)
{
  c ? (d ? pos(x) : neg(y)) : (void)0;
  return c + d;
}

// the condition holds a call, evaluated before either arm
int condition_call(int a, int x, int y)
    CERASTES_REQUIRES(a > 0 ? x > 0 : y < 0)
    CERASTES_ENSURES(r, r == a)
{
  id(a) > 0 ? pos(x) : neg(y);
  return a;
}

// an assert in an arm is a check on its own path
int assert_arm(int c, int x)
    CERASTES_REQUIRES(c == 0 || x > 0)
    CERASTES_ENSURES(r, r == c)
{
  c ? assert(x > 0) : (void)0;
  return c;
}
