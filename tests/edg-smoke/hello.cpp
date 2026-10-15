// expect: frontend-ok
// mode: c++20
//
// Smoke test: the pinned EDG front end parses a C++20 hello world against the
// system SDK headers without diagnostics.  It uses <cstdio>; the libc++
// container, string and stream headers are in libcxx_headers.cpp.

#include <cstdio>

int main() {
  std::printf("Hello, world!\n");
  return 0;
}
