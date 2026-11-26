// expect: rejected cerastes-unsupported_expression @ L21
// expect: rejected cerastes-unsupported_expression @ L25
// expect: rejected cerastes-unsupported_expression @ L31
// expect: rejected cerastes-unsupported_expression @ L37
// mode: c++20
// twin: tests/memory/array_struct_init_zero.c
// Pins: 0064 item 16 (question 304: "Follow the rejection like the compiler warning"; question 321: the measured rule): in C++ a {0} whose 0 elides the braces of a nested structure in an aggregate of two or more members, or of the elements of an array of structures, is rejected like clang++'s -Wmissing-braces at -Wall, as a global, a local, a compound literal value and an array (C accepts every one: the twin array_struct_init_zero.c). The tag is the existing elision pins', to confirm at the joined run
// Pre-registered from record 0064 item 16 (Part B), before any run: rejected (new behaviour: the frozen build accepted the first three)
#include <cerastes.h>

struct P {
  int x;
  int y;
};

struct T {
  P p;
  int z;
};

T g = {0};  // <- here

int local()
{
  T t = {0};  // <- here
  return t.z;
}

int value()
{
  T t = (T){0};  // <- here
  return t.z;
}

int array()
{
  P v[2] = {0};  // <- here
  return v[0].x;
}
