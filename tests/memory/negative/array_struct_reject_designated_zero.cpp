// expect: rejected cerastes-unsupported_expression @ L44
// expect: rejected cerastes-unsupported_expression @ L45
// expect: rejected cerastes-unsupported_expression @ L46
// expect: rejected cerastes-unsupported_expression @ L47
// expect: rejected cerastes-unsupported_expression @ L48
// expect: rejected cerastes-unsupported_expression @ L49
// expect: rejected cerastes-unsupported_expression @ L50
// expect: rejected cerastes-unsupported_expression @ L51
// expect: rejected cerastes-unsupported_expression @ L55
// mode: c++20
// twin: tests/memory/array_struct_cpp.cpp
// Pins: 0064 item 16, "Designated lists": in C++ an unbraced designated member that is a structure or an array ({.p = 0}, {.p = 1}, {.a = 0}, {.a = 1}) is rejected whatever its parent (one member or two) and its value (clang++ warns -Wc99-designator on each without a flag, and -Wmissing-braces at -Wall where the parent has two or more members; g++ is silent), and a braced designated value whose 0 elides a structure inside a two-member parent ({.t = {0}} over W, whose T has two members) is the nested list the predicate flags. Rows, in order: a structure member over a one-member parent ({.p = 0}, {.p = 1}), over a two-member parent (the same two), an array member over a one-member parent ({.a = 0}, {.a = 1}), over a two-member parent (the same two), and the braced form as a local. The positive twin holds the forms C++ accepts (the braced value over an only member). The frozen build accepted every row (question 327, for the owner's review)
// Pre-registered from record 0064 item 16, "Designated lists", before any run: rejected (tag as array_struct_reject_zero_cpp.cpp)
#include <cerastes.h>

struct P {
  int x;
  int y;
};

struct T1 {
  P p;
};

struct T {
  P p;
  int z;
};

struct A1 {
  int a[4];
};

struct S {
  int a[4];
  int n;
};

struct W {
  T t;
  int k;
};

T1 g1 = {.p = 0};  // <- here
T1 g2 = {.p = 1};  // <- here
T g3 = {.p = 0};  // <- here
T g4 = {.p = 1};  // <- here
A1 g5 = {.a = 0};  // <- here
A1 g6 = {.a = 1};  // <- here
S g7 = {.a = 0};  // <- here
S g8 = {.a = 1};  // <- here

int braced()
{
  W w = {.t = {0}};  // <- here
  return w.k;
}
