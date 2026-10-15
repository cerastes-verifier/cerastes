// expect: frontend-error unattached_attribute @ L15
// expect: native-ok
// twin: tests/annotations/contracts/concept_and_guide.cpp
// Mutation of the twin: an attribute between the template header and the guide: EDG rejects it itself (probed against the fork: no placement of a registered attribute on a deduction guide reaches the back end, so there is no cerastes tag for it; `box(T x) [[...]] -> box<T>` is deduction_guide_placement).
// Pins: only this diagnostic is reported (a one-error file, no incidental noise).
#include <cerastes.h>

template <class T>
struct box {
  box(T v);
  T t;
};

#ifdef __CERASTES__
template <class T> [[cerastes::predicate]] box(T x) -> box<T>;
#else
template <class T>
box(T x) -> box<T>;
#endif

template <class T>
concept addable = requires(T x) { x + 1; };

static_assert(addable<int>);

box b(1);
