// expect: rejected cerastes-unsupported_declaration @ L7
// mode: c++20
// twin: tests/memory/not_invalidating_cpp.cpp
// Pins: 0058 item 8 (M7): not_invalidating on a parameter of a member function: the struct with a member function is rejected (unsupported_declaration, M7) before the marker is read
// Pre-registered from record 0058 item 8: rejected unsupported_declaration at the struct (observed with build/cerastes verify)
#include <cerastes.h>
struct S {  // <- here
  int x;
  int *make(int *p CERASTES_NOT_INVALIDATING);
};
