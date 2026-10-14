// A standard-syntax attribute with an expression argument on a null statement
// (C++23 [[assume]]), in a template: is the argument dependent in the prototype?
template <class T>
T tf(T v) {
  [[assume(v + 1 > 0)]];
  return v;
}
int use() { return tf(2); }
