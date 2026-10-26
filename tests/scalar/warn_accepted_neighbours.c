// expect: verified
// expect: native-ok
// mode: c23, c++20
// twin-exempt: the must-fail counterparts are the rejected reject_warn_* tests (a rejected twin does not count for the twin rule)
// Pins: 0025 item 6: the neighbours of the rejected -Wall shapes that every compiler is silent about verify (invariant 2's positive side); GCC's -Wtype-limits shapes (`u >= 0`) are left out, they need -Wextra
#include <assert.h>
#include <stdbool.h>
#include <cerastes.h>

// R1: parenthesized or same-operator logical operands

int or_of_and(int a, int b, int c)
    CERASTES_ENSURES(r, r == ((a && b) || c))
{
  return (a && b) || c;
}

int and_of_or(int a, int b, int c)
    CERASTES_ENSURES(r, r == (a && (b || c)))
{
  return a && (b || c);
}

int and_and(int a, int b, int c)
    CERASTES_ENSURES(r, r == (a && b && c))
{
  return a && b && c;
}

int or_or(int a, int b, int c)
    CERASTES_ENSURES(r, r == (a || b || c))
{
  return a || b || c;
}

int or_paren_and(int a, int b, int c)
    CERASTES_ENSURES(r, r == (a || (b && c)))
{
  return a || (b && c);
}

// R2: shifts with parenthesized sums, products, a negation, other shifts

int paren_shl(int a, int b, int c)
    CERASTES_REQUIRES(0 <= a && a <= 100 && 0 <= b && b <= 100 && 0 <= c && c <= 4)
    CERASTES_ENSURES(r, r == ((a + b) << c))
{
  return (a + b) << c;
}

int product_shl(int a, int b, int c)
    CERASTES_REQUIRES(0 <= a && a <= 100 && 0 <= b && b <= 100 && 0 <= c && c <= 4)
    CERASTES_ENSURES(r, r == (a * b << c))
{
  return a * b << c;
}

int shl_paren_sum(int a, int b, int c)
    CERASTES_REQUIRES(0 <= a && a <= 100 && 0 <= b && b <= 2 && 0 <= c && c <= 2)
    CERASTES_ENSURES(r, r == (a << (b + c)))
{
  return a << (b + c);
}

int shl_product(int a, int b, int c)
    CERASTES_REQUIRES(0 <= a && a <= 100 && 0 <= b && b <= 2 && 0 <= c && c <= 2)
    CERASTES_ENSURES(r, r == (a << b * c))
{
  return a << b * c;
}

int shl_shl(int a, int b, int c)
    CERASTES_REQUIRES(0 <= a && a <= 100 && 0 <= b && b <= 2 && 0 <= c && c <= 2)
    CERASTES_ENSURES(r, r == (a << b << c))
{
  return a << b << c;
}

int paren_shr(int a, int b, int c)
    CERASTES_REQUIRES(0 <= a && a <= 100 && 0 <= b && b <= 100 && 0 <= c && c <= 4)
    CERASTES_ENSURES(r, r == ((a + b) >> c))
{
  return (a + b) >> c;
}

// R3: a negation that is parenthesized, or compared with a truth value

int not_eq_not(int x, int y)
    CERASTES_ENSURES(r, r == (!x == !y))
{
  return !x == !y;
}

int not_eq_cmp(int x, int y)
    CERASTES_ENSURES(r, r == (!x == (y > 0)))
{
  return !x == (y > 0);
}

int not_eq_bool(int x, bool b)
    CERASTES_ENSURES(r, r == (!x == b))
{
  return !x == b;
}

int not_eq_and_bools(int x, bool p, bool q)
    CERASTES_ENSURES(r, r == (!x == (p & q)))
{
  return !x == (p & q);
}

int not_eq_plus_cmp(int x, int a, int b)
    CERASTES_ENSURES(r, r == (!x == (a > b)))
{
  return !x == +(a > b);
}

int paren_not_eq(int x, int y)
    CERASTES_ENSURES(r, r == ((!x) == y))
{
  return (!x) == y;
}

int eq_not(int x, int y)
    CERASTES_ENSURES(r, r == (x == !y))
{
  return x == !y;
}

int not_of_eq(int x, int y)
    CERASTES_ENSURES(r, r == (!(x == y)))
{
  return !(x == y);
}

// R4: parenthesized comparisons

int paren_lt_lt(int a, int b, int c)
    CERASTES_ENSURES(r, r == ((a < b) < c))
{
  return (a < b) < c;
}

int paren_eq_eq(int a, int b, int c)
    CERASTES_ENSURES(r, r == ((a == b) == c))
{
  return (a == b) == c;
}

int paren_lt_eq_one(int a, int b)
    CERASTES_ENSURES(r, r == ((a < b) == 1))
{
  return (a < b) == 1;
}

int lt_paren_lt(int a, int b, int c)
    CERASTES_ENSURES(r, r == (a < (b < c)))
{
  return a < (b < c);
}

int paren_cmp_eq_paren_cmp(int a, int b, int c, int d)
    CERASTES_ENSURES(r, r == ((a < b) == (c < d)))
{
  return (a < b) == (c < d);
}

// R5: a cast, other operands, and a macro or an assert that holds the comparison

int cast_cmp(int x)
    CERASTES_ENSURES(r, r == ((int)x == x))
{
  return (int)x == x;
}

int cast_other_side(int x)
    CERASTES_ENSURES(r, r == (x == (int)x))
{
  return x == (int)x;
}

int long_cast(int x)
    CERASTES_ENSURES(r, r == ((long)x == x))
{
  return (long)x == x;
}

int sum_diff(int x)
    CERASTES_REQUIRES(x < 100)
    CERASTES_ENSURES(r, r == (x + 1 == x + 2))
{
  return x + 1 == x + 2;
}

int long_casts(int x)
    CERASTES_ENSURES(r, r == 1)
{
  return (long)x == (long)x;
}

int constants_alike(void)
    CERASTES_ENSURES(r, r == 1)
{
  return 1 + 2 == 1 + 2;
}

int two_vars(int x, int y)
    CERASTES_ENSURES(r, r == (x == y))
{
  return x == y;
}

#define SAME(v) ((v) == (v))
#define ID(e) e

int macro_body(int x)
    CERASTES_ENSURES(r, r == 1)
{
  return SAME(x);
}

int macro_argument(int x)
    CERASTES_ENSURES(r, r == 1)
{
  return ID(x == x);
}

int assert_same(int x)
    CERASTES_ENSURES(r, r == x)
{
  assert(x == x);
  return x;
}

// R6: comparisons of a truth value with 0 or 1, or with another value

int truth_eq_one(int a, int b)
    CERASTES_ENSURES(r, r == ((a > b) == 1))
{
  return (a > b) == 1;
}

int truth_eq_zero(int a, int b)
    CERASTES_ENSURES(r, r == ((a > b) == 0))
{
  return (a > b) == 0;
}

int truth_ge_one(int a, int b)
    CERASTES_ENSURES(r, r == ((a > b) >= 1))
{
  return (a > b) >= 1;
}

int truth_lt_one(int a, int b)
    CERASTES_ENSURES(r, r == ((a > b) < 1))
{
  return (a > b) < 1;
}

int truth_gt_zero(int a, int b)
    CERASTES_ENSURES(r, r == ((a > b) > 0))
{
  return (a > b) > 0;
}

int truth_le_zero(int a, int b)
    CERASTES_ENSURES(r, r == ((a > b) <= 0))
{
  return (a > b) <= 0;
}

int truth_ne_zero(int a, int b)
    CERASTES_ENSURES(r, r == ((a > b) != 0))
{
  return (a > b) != 0;
}

int truth_ne_one(int a, int b)
    CERASTES_ENSURES(r, r == ((a > b) != 1))
{
  return (a > b) != 1;
}

int truth_and_eq_var(int a, int b, int c)
    CERASTES_ENSURES(r, r == ((a && b) == c))
{
  return (a && b) == c;
}

int truth_bool_eq_one(bool p)
    CERASTES_ENSURES(r, r == (p == 1))
{
  return p == 1;
}

int truth_bool_eq_bool(bool p, bool q)
    CERASTES_ENSURES(r, r == (p == q))
{
  return p == q;
}

int truth_cmp_eq_bool(int a, int b, bool p)
    CERASTES_ENSURES(r, r == ((a > b) == p))
{
  return (a > b) == p;
}

// R7: a shift of an unsigned value, `>>`, a shifted value compared with 0, `?:` with arms 1 and 0, in a truth context

int truth_ctx_unsigned_shl(unsigned u, unsigned v)
    CERASTES_REQUIRES(u < 1000 && v < 3)
    CERASTES_ENSURES(r, r == ((u << v) != 0))
{
  if (u << v) {
    return 1;
  }
  return 0;
}

int truth_ctx_shr(int a, int b)
    CERASTES_REQUIRES(0 <= a && a <= 100 && 0 <= b && b <= 4)
    CERASTES_ENSURES(r, r == ((a >> b) != 0))
{
  if (a >> b) {
    return 1;
  }
  return 0;
}

int truth_ctx_shl_ne_zero(int a, int b)
    CERASTES_REQUIRES(0 <= a && a <= 100 && 0 <= b && b <= 4)
    CERASTES_ENSURES(r, r == ((a << b) != 0))
{
  if (a << b != 0) {
    return 1;
  }
  return 0;
}

int truth_ctx_cond_arm_mul(int a, int b, int c, int x)
    CERASTES_REQUIRES(-1000 <= a && a <= 1000 && -1000 <= b && b <= 1000)
    CERASTES_ENSURES(r, r == ((c != 0 ? a * b : x) != 0))
{
  if (c ? a * b : x) {
    return 1;
  }
  return 0;
}

int truth_ctx_cond_arm_mul_second(int a, int b, int c, int x)
    CERASTES_REQUIRES(-1000 <= a && a <= 1000 && -1000 <= b && b <= 1000)
    CERASTES_ENSURES(r, r == ((c != 0 ? x : a * b) != 0))
{
  if (c ? x : a * b) {
    return 1;
  }
  return 0;
}

int truth_ctx_cond_both_mul(int a, int b, int c)
    CERASTES_REQUIRES(-1000 <= a && a <= 1000 && -1000 <= b && b <= 1000)
    CERASTES_ENSURES(r, r == ((a * b) != 0))
{
  if (c ? a * b : a * b) {
    return 1;
  }
  return 0;
}

int truth_ctx_cond_one_zero(int c)
    CERASTES_ENSURES(r, r == (c != 0 ? 1 : 0))
{
  if (c ? 1 : 0) {
    return 1;
  }
  return 0;
}

int truth_ctx_cond_zero_one(int c)
    CERASTES_ENSURES(r, r == (c != 0 ? 0 : 1))
{
  if (c ? 0 : 1) {
    return 1;
  }
  return 0;
}

// shift assignment with an unparenthesized sum, and a void ?: as the right operand of a comma

int shift_assign(int x, int a, int b)
    CERASTES_REQUIRES(0 <= x && x <= 100 && 0 <= a && a <= 2 && 0 <= b && b <= 2)
    CERASTES_ENSURES(r, r == (x << (a + b)))
{
  x <<= a + b;
  return x;
}

int comma_void(int x, int c)
    CERASTES_REQUIRES(0 <= x && x < 100)
    CERASTES_ENSURES(r, r == x + 1)
{
  int t = x;
  t++, c ? (void)0 : (void)0;
  return t;
}

// R8: enumerators of one enumeration, casts to int, a comparison with an enumerator

enum color { RED, GREEN };
enum nat { NA, NB };

int same_enum_sum(void)
    CERASTES_ENSURES(r, r == 1)
{
  return RED + GREEN;
}

int cast_sum(void)
    CERASTES_ENSURES(r, r == 0)
{
  return (int)RED + NA;
}

int cast_both_arms(int x)
    CERASTES_ENSURES(r, r == 0)
{
  return x ? (int)RED : NA;
}

int compare_enumerator(enum color c)
    CERASTES_ENSURES(r, r == (c == GREEN))
{
  return c == GREEN;
}

// R9: a constant on the left, a bool on the left, a truth value on the right

int const_left(int x)
    CERASTES_ENSURES(r, r == (4 && x))
{
  return 4 && x;
}

int bool_left_const(bool b)
    CERASTES_ENSURES(r, r == (b && 4))
{
  return b && 4;
}

int not_const_right(int x)
    CERASTES_ENSURES(r, r == 0)
{
  return x && !4;
}

int cmp_const_right(int x)
    CERASTES_ENSURES(r, r == (x && 2 > 1))
{
  return x && 2 > 1;
}

// R10: constants that fit the type of the other operand, and a promoted sum

int uc_eq_255(unsigned char uc)
    CERASTES_ENSURES(r, r == (uc == 255))
{
  return uc == 255;
}

int uc_plus_eq_300(unsigned char uc)
    CERASTES_ENSURES(r, r == (uc + 1 == 300))
{
  return uc + 1 == 300;
}

int sc_gt_126(signed char sc)
    CERASTES_ENSURES(r, r == (sc > 126))
{
  return sc > 126;
}

int x_lt_int_max(int x)
    CERASTES_ENSURES(r, r == (x < 2147483647LL))
{
  return x < 2147483647LL;
}

// R11: satisfiable overlaps, and a comparison not adjacent to its partner

int ge_and_le(int x)
    CERASTES_ENSURES(r, r == (x >= 5 && x <= 5))
{
  return x >= 5 && x <= 5;
}

int eq_or_eq(int x)
    CERASTES_ENSURES(r, r == (x == 1 || x == 2))
{
  return x == 1 || x == 2;
}

int third_first(int x, int z)
    CERASTES_ENSURES(r, r == (z && x > 5 && x < 3))
{
  return z && x > 5 && x < 3;
}

int range(int x)
    CERASTES_ENSURES(r, r == (x > 5 && x < 10))
{
  return x > 5 && x < 10;
}

// R12: braces around the inner if, around the then-part, or an outer else; R13: an empty block, an empty then-part with an else

int braced_inner(int a, int b)
    CERASTES_ENSURES(r, r == (a ? (b ? 1 : 2) : 0))
{
  int x = 0;
  if (a) {
    if (b)
      x = 1;
    else
      x = 2;
  }
  return x;
}

int braced_then(int a, int b)
    CERASTES_ENSURES(r, r == (a ? (b ? 1 : 0) : 2))
{
  int x = 0;
  if (a) {
    if (b)
      x = 1;
  } else
    x = 2;
  return x;
}

int two_elses(int a, int b)
    CERASTES_ENSURES(r, r == (a ? (b ? 1 : 2) : 3))
{
  int x = 0;
  if (a)
    if (b)
      x = 1;
    else
      x = 2;
  else
    x = 3;
  return x;
}

int else_if(int a, int b)
    CERASTES_ENSURES(r, r == (a ? 1 : (b ? 2 : 3)))
{
  int x = 0;
  if (a)
    x = 1;
  else if (b)
    x = 2;
  else
    x = 3;
  return x;
}

int empty_then_else(int x)
    CERASTES_REQUIRES(0 <= x && x < 100)
    CERASTES_ENSURES(r, r == (x != 0 ? x : 1))
{
  if (x)
    ;
  else
    x = 1;
  return x;
}

int empty_block(int x)
    CERASTES_REQUIRES(0 <= x && x < 100)
    CERASTES_ENSURES(r, r == x)
{
  if (x) {
  }
  return x;
}

// R13: loop bodies that are null statements (rejected by clang's -Wempty-body only with the ; on the line of the condition), a constant in a macro, a named constant

int drain(int x)
    CERASTES_REQUIRES(0 <= x && x < 100)
    CERASTES_ENSURES(r, r == -1)
{
  CERASTES_INVARIANT(x >= 0)
  CERASTES_DECREASES(x)
  while (x--)
    ;
  return x;
}

int count_up(int n)
    CERASTES_REQUIRES(0 <= n && n < 100)
    CERASTES_ENSURES(r, r == n)
{
  int i;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  for (i = 0; i < n; i++)
    ;
  return i;
}

#define AND4(v) ((v) && 4)
#define OVL(v) ((v) > 5 && (v) < 3)

int macro_constant(int x)
    CERASTES_ENSURES(r, r == (x != 0))
{
  return AND4(x);
}

int macro_overlap(int x)
    CERASTES_ENSURES(r, r == 0)
{
  return OVL(x);
}

int assert_constant(int x)
    CERASTES_REQUIRES(x != 0)
    CERASTES_ENSURES(r, r == x)
{
  assert(x && 4);
  return x;
}

int assert_tautology(int x)
    CERASTES_ENSURES(r, r == x)
{
  assert(x > 5 || x < 6);
  return x;
}

const int K = 5;

int named_overlap(int x)
    CERASTES_ENSURES(r, r == 0)
{
  return x > K && x < 3;
}
