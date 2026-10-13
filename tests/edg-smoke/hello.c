// expect: frontend-ok
// mode: c23
//
// Smoke test: the pinned EDG front end parses a C23 hello world against the
// system SDK headers without diagnostics.

#include <stdio.h>

int main(void) {
  printf("Hello, world!\n");
  return 0;
}
