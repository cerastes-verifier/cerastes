// expect: golden
// Annotation-free copy of tests/cxx/templates/integral.cpp from
// verifast branch generic2, commit 0fb63506a51cae559ae92348eb54ba7f7be0bf6d.
// Whole-line //@ annotations are replaced by empty lines and trailing
// //~ should_fail markers are removed; no other change.
// Line offset: line N here is line N-6 in the original (header is 6 lines).
// A function template whose type parameter T is constrained by std::integral
// is verified once, for every integer type other than bool. In that proof, T
// is an integer type with unknown limits. The body can do arithmetic on values
// of type T, which the integer promotions convert to an unknown type of at
// least the rank of int.

#include <concepts>
#include <stdint.h>

template <std::integral T>
T clamp100(const T v)


{
  if (v > 100 && v != 0) {
    // Every type argument can represent 100.
    return (T)100;
  }
  return v;
}

template <std::integral T>
T dec(const T v)


{
  return (T)(v - 1);
}

template <std::integral T>
T count_to(const T n)


{
  T i = (T)0;
  while (i < n)

  {
    i = (T)(i + 1);
  }
  return i;
}

template <std::integral T>
bool is_digit(const T v)


{
  bool b = !(v < 0) && v < 10;
  return b;
}

template <std::integral T, std::integral U>
U convert(const T v)


{
  return static_cast<U>(v);
}

// The requires-clause form of the constraint.
template <typename T>
  requires std::integral<T>
T twice(const T v)


{

  return (T)(v * 2);
}

template <std::integral T>
T declared_first(const T v);



template <std::integral T>
T declared_first(const T v)


{
  return (T)(1 + v);
}

// Only T is constrained. U is verified as any scalar type.
template <std::integral T, typename U>
U pick(const T v, const U u)


{
  return u;
}

// Verified even though nothing instantiates it.
template <std::integral T>
T uninstantiated(const T v)


{
  return (T)(v + v + v);
}

int main()


{
  unsigned char c = clamp100<unsigned char>(200);

  long long ll = clamp100(5LL);

  long d = dec(5L);

  uint16_t n = count_to<uint16_t>(3);

  bool digit = is_digit(7);

  short s = convert<long, short>(42);

  char32_t t = twice<char32_t>(21);

  int f = declared_first(3);

  long *p = 0;
  long *q = pick(1, p);

  return 0;
}
