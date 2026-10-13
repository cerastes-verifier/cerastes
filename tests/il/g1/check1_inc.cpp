// expect: golden
// Gate G1 check 1: what type_kind do operations on a dependent value have
// (tk_template_param vs tk_unknown), and is the (T) cast its own node?
// PASS: the `v + 1` operation has type_kind tk_template_param and `(T)(...)`
// shows up as a separate cast node around it; (T)100 is reported as it is.
#include <concepts>

template <std::integral T>
T inc(const T v) { return (T)(v + 1); }

template <std::integral T>
T plus_local(const T v) { auto w = v + 1; return (T)w; }

template <std::integral T>
T const_cast_c() { return (T)100; }

template <std::integral T>
T static_cast_v(const T v) { return static_cast<T>(v); }

template <std::integral T>
bool less10(const T v) { return v < 10; }

template <std::integral T>
T sum2(const T a, const T b) { return (T)(a + b); }

int use() {
  int r = inc(5) + plus_local(1) + const_cast_c<int>() + static_cast_v(2);
  return r + (less10(3) ? 1 : 0) + sum2(1, 2);
}
