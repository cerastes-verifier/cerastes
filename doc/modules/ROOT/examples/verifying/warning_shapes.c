// expect: rejected cerastes-logical_operand_parentheses @ L9
// mode: c23, c++20
// An unparenthesized && inside a ||: Clang and GCC warn (-Wlogical-op-parentheses, -Wparentheses), so cerastes rejects it. Write (a && b) || c or a && (b || c).
// tag::source[]
#include <cerastes.h>

int pick(int a, int b, int c)
{
  return a && b || c;
}
// end::source[]
