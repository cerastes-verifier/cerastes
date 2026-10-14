int g(int x) __attribute__((cr_req(x > 0)));
int g(int x) __attribute__((cr_req(x < 100)));
int g(int x) { return x; }
template <class T>
T f(T v) __attribute__((cr_req(v > 0)));
template <class T>
T f(T v) __attribute__((cr_req(v < 100))) { return v; }
int use() { return g(1) + f(2); }
