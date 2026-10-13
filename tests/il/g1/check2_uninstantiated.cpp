// expect: golden
// Gate G1 check 2: a template that nothing instantiates still has a
// prototype routine with a full body.
// PASS: `never_used` has a prototype-instantiation routine whose body
// (the loop and the conditional) is present, just like the instantiated
// contrast `used`.
#include <concepts>

template <std::integral T>
T never_used(const T v) {
  T acc = 0;
  for (T i = 0; i < v; ++i) {
    if (i < 10) acc = (T)(acc + i);
    else acc = (T)(acc - 1);
  }
  return acc;
}

template <std::integral T>
T used(const T v) {
  T acc = 0;
  while (acc < v) acc = (T)(acc + 1);
  return acc;
}

int main() { return used(3); }
