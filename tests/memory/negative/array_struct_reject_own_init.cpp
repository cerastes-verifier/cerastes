// expect: rejected cerastes-unsupported_declaration @ L15
// mode: c++20
// twin: tests/memory/array_struct_loop.c
// Pins: 0064 item 6 and the handoff's untaken should-fix: an element of the initializer that reads the array being declared is rejected in C++ with the message of the rule (a local variable used in its own initializer, as C prints), not with the brace-elision message the front end gives it; the tag and the line (the declaration) are those of array_reject_self_reading_init.c and struct_array_reject_own_init.c, to confirm at the first run (the tag is a question to the orchestrator)
// Pre-registered from record 0064 item 6 (Part C1), before any run: rejected
#include <cerastes.h>
#include <stddef.h>

struct S {
  int x;
};

int f(void)
{
  struct S v[2] = {{1}, {v[0].x}};  // <- here
  return v[1].x;
}
