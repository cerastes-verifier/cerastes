// expect: golden
// mode: c++20
// Contrast probe: constraint lines must name the concept that was written, not
// a fixed one.  `only_floats` is constrained by std::floating_point and
// `only_even` by a user-defined concept; neither may be reported as
// `integral`.  `free_for_all` has no constraint and must have no
// `constraint` line.  `req_float` and `trailing_float` use a requires-clause
// and a trailing requires-clause with a different concept, and `abbrev`
// uses an abbreviated parameter, to cover the other three locations.

#include <concepts>

template <typename T>
concept Small = sizeof(T) <= 2;

template <std::floating_point T>
T only_floats(T v) { return v + v; }

template <Small T>
T only_small(T v) { return v + v; }

template <typename T>
T free_for_all(T v) { return v + v; }

template <typename T>
  requires std::floating_point<T>
T req_float(T v) { return v + v; }

template <typename T>
T trailing_float(T v) requires Small<T> { return v + v; }

auto abbrev(std::floating_point auto v) { return v + v; }
