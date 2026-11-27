// expect: rejected cerastes-unsupported_expression @ L28
// expect: rejected cerastes-unsupported_expression @ L42
// expect: rejected cerastes-unsupported_expression @ L53
// expect: rejected cerastes-unsupported_expression @ L59
// expect: rejected cerastes-unsupported_expression @ L64
// mode: c23
// c-only: a lone zero that elides braces is C's universal zero initializer; in C++ item 16's other rule (clang++ -Wmissing-braces) rejects these shapes for a different reason
// twin: tests/memory/struct_init_literal_zero_member_c.c
// Pins: 0064 item 16 (audit round R, should-fix 3b; question 339's rule at its three call sites, Commit 2): in C a lone zero that is the whole initializer and is not an integer literal is rejected where it elides the braces of an array MEMBER in a parent of two or more members (the third call site, which negative/struct_init_reject_nonliteral_zero_c.c does not hold: that file pins the array-element site and the nested-structure site), here the audit's struct A { int a[4]; int n; } with '\0', and in every storage position the rule's single function reads: a local (local_array_member), a global (global_array_member), a `static` local (static_array_member) and a compound literal (literal_array_member, literal_nested_member: `(struct A){'\0'}` and `(struct T){'\0'}`). {0x0} over the same structure is accepted (struct_init_literal_zero_member_c.c, the twin). clang's C warns -Wmissing-braces at -Wall on each ('\0') and is silent on 0x0, GCC on both: the audit's q339 and q339c cells (16 array-member cells, all matching). A mutant that deletes the clause at the third call site (encode.cpp, the audit's finding 3) survives every other pin
// Pre-registered from record 0064 item 16 (question 339, the rule's three call sites) and the audit's finding 3, before any run: one unsupported_expression row per position, at its initializer. The audit's table and its finding 3 measured the local `struct A t = {'\0'}`, the global `struct A g = {'\0'}`, `static struct T st = {'\0'}` and `use((struct T){'\0'})`; only `static struct A st` and `(struct A){'\0'}` are registered by analogy to those cells and the one rule. A question 160 candidate for the line of a compound literal row (the row may be at the call whose argument it is)
#include <cerastes.h>

struct P {
  int x;
  int y;
};

struct A {
  int a[4];
  int n;
};

struct T {
  struct P p;
  int z;
};

struct A g = {'\0'};  // <- here

int take_a(struct A a)
{
  return a.n;
}

int take_t(struct T t)
{
  return t.z;
}

int local_array_member(void)
{
  struct A t = {'\0'};  // <- here
  return t.n;
}

int global_array_member(void)
{
  return g.n;
}

int static_array_member(void)
{
  static struct A st = {'\0'};  // <- here
  return st.n;
}

int literal_array_member(void)
{
  return take_a((struct A){'\0'});  // <- here
}

int literal_nested_member(void)
{
  return take_t((struct T){'\0'});  // <- here
}
