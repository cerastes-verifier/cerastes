// expect: rejected cerastes-unsupported_expression @ L21
// mode: c++20
// twin: tests/memory/struct_array_cpp.cpp
// Pins: 0063 item 6 and audit round 4b S2 (M8, decision 0044 item 6): a struct temporary with an array member inside the initializer of a variable (`int v = first(S{{2, 3, 4, 5}, 6});`): the front end folds the temporary and drops its braces, so `S{{1, 2}, 3}` cannot be told from the elided `S{1, 2, 3}`: rejected at the line, naming the statement form that is accepted (`int r; r = first(S{...});`)
// Pre-registered from audit round 4b S2 (edg-backend-report-15), before any run: one rejection, `unsupported_expression`, at the declaration's line
#include <cerastes.h>

struct S {
  int a[4];
  int n;
};

int first(S s)
    CERASTES_ENSURES(r, r == s.a[0])
{
  return s.a[0];
}

int main()
{
  int v = first(S{{2, 3, 4, 5}, 6});  // <- here
  return v;
}
