// expect: verified
// expect: native-ok
// c-only: in C++ {0} eliding the braces of an array member in a structure of two or more members is rejected like clang++'s -Wmissing-braces (record 0063 item 6, question 303)
// Pins: 0063 item 6 (question 303): C's {0} elides the braces of the array member and zeroes every leaf; accepted in C (clang and GCC are silent), rejected in C++ for a structure of two or more members (struct_array_reject_zero_cpp.cpp)
// Pre-registered from record 0063 item 6 (Part C), before any run: verifies
#include <cerastes.h>
#include <stddef.h>

struct S {
  int a[4];
  int n;
};

int f(void)
    CERASTES_ENSURES(r, r == 0)
{
  struct S s = {0};
  CERASTES_CHECK(s.a[0] == 0 && s.a[1] == 0 && s.a[2] == 0 && s.a[3] == 0 && s.n == 0);
  return s.n;
}
