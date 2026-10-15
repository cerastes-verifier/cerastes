// expect: frontend-error wrong_entity_for_attribute @ L18
// expect: native-ok
// twin: tests/annotations/contracts/concept_and_guide.cpp
// Mutation of the twin: a contract after a requires-expression's parameters must be rejected, not silently dropped (CP2a 4; verify the tag).
// Pins: only this diagnostic is reported (a one-error file, no incidental noise).
#include <cerastes.h>

template <class T>
struct box {
  box(T v);
  T t;
};

template <class T>
box(T x) -> box<T>;

template <class T>
concept addable = requires(T x) CERASTES_REQUIRES(x > 0) { x + 1; };

static_assert(addable<int>);

box b(1);
