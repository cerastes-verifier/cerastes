// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0022 item 2 and 0010 item 11: unsigned arithmetic in a contract is checked where the clause is: a trusted declaration and a defined function, each with a requires that keeps the sum in range
#include <cerastes.h>

enum color { RED, GREEN, BLUE };

unsigned bump_trusted(unsigned a)
    CERASTES_REQUIRES(a <= 4294967294u)
    CERASTES_ENSURES(r, r == a + 1u);

unsigned use_top(void)
    CERASTES_ENSURES(r, r == 4294967295u)
{
  return bump_trusted(4294967294u);
}

unsigned add_body(unsigned a, unsigned b)
    CERASTES_REQUIRES(a <= 4294967295u - b)
    CERASTES_ENSURES(r, r == a + b)
{
  return a + b;
}

unsigned add_edge(void)
    CERASTES_ENSURES(r, r == 4294967295u)
{
  return add_body(4294967290u, 5u);
}

enum color pick(int x)
    CERASTES_REQUIRES(0 <= x && x <= 3)
    CERASTES_ENSURES(r, (int)r == x)
{
  return (enum color)x;
}
