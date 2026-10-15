// expect: golden
// expect: native-ok
// Pins: a requires-expression with a parameter and a deduction guide with a
// parameter, with no annotations: the places where a contract after the
// parameter list must be rejected, not dropped (CP2a). Twin of the
// reqexpr_param_placement and deduction_guide_placement negatives.
#include <cerastes.h>

template <class T>
struct box {
  box(T v);
  T t;
};

template <class T>
box(T x) -> box<T>;

template <class T>
concept addable = requires(T x) { x + 1; };

static_assert(addable<int>);

box b(1);
