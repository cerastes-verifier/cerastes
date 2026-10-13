// expect: golden
// Gate G1 check 3: instance routines carry assoc_template and
// template_arg_list with the deduced and explicit arguments.
// PASS: each instance below links to its template and lists its arguments
// (deduced: long; explicit: unsigned char; mixed: T explicit, U deduced;
// nested: instance created from inside another template's prototype).
#include <concepts>

template <std::integral T>
T inc(const T v) { return (T)(v + 1); }

template <typename T, typename U>
U pick(U u) { return u; }

template <std::integral T>
T twice_inc(const T v) { return inc(inc(v)); }

long a() { return inc(5L); }
unsigned char b() { return inc<unsigned char>(3); }
char c() { return pick<int>('x'); }
short d() { return twice_inc((short)1); }
