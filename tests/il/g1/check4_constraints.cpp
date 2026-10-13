// expect: golden
// Gate G1 check 4: std::integral is reachable from all four constraint
// locations.
// PASS: each of f1..f4 shows the std::integral concept in the IL at the
// location named in its comment; f5 shows a conjunction of two.
#include <concepts>

// type-constraint
template <std::integral T> T f1(T x) { return x; }

// requires-clause
template <typename T> requires std::integral<T> T f2(T x) { return x; }

// trailing requires-clause
template <typename T> T f3(T x) requires std::integral<T> { return x; }

// abbreviated (constrained auto parameter)
auto f4(std::integral auto x) { return x; }

// conjunction in a requires-clause
template <typename T, typename U> requires std::integral<T> && std::integral<U>
T f5(T x, U) { return x; }

int use() { return f1(1) + f2(2) + f3(3) + f4(4) + f5(5, 6); }
