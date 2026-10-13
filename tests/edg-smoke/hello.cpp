// expect: frontend-ok
// mode: c++20
//
// Smoke test: the pinned EDG front end parses a C++20 hello world against the
// system SDK headers without diagnostics.  It uses <cstdio> rather than
// <iostream>; see docs/status.md for the libc++ <iostream> issue.

#include <cstdio>

int main() {
  std::printf("Hello, world!\n");
  return 0;
}
