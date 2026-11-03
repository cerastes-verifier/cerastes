// expect: rejected cerastes-unsequenced_modification @ L31
// mode: c23, c++20
// twin: tests/memory/struct_value_param.c
// Pins: 0044 item 5 (question 70): h2(s, s = t): the argument s and the assignment name the whole object (clang -Wunsequenced and GCC -Wsequence-point warn, P68 Q4): rejected like the compilers or by 0039 item 3's static rule
#include <cerastes.h>

struct S {
  int x;
  int y;
};

int get(struct S s)
{
  return s.x;
}

int h(struct S a, int b)
{
  return a.x + b;
}

int h2(struct S a, struct S b)
{
  return a.x + b.x;
}

int f(void)
{
  struct S s = {0, 0};
  struct S t = {1, 1};
  int r = h2(s, s = t);  // <- here
  return r + t.x;
}
