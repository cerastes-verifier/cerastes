// Where does an unrecognized [[cerastes::...]] attribute survive in the IL?
// Each entity or statement kind below carries one attribute named a_<kind>.
[[cerastes::a_var]] int gv = 0;                       // namespace-scope variable
typedef int [[cerastes::a_typedef]] my_int;           // typedef: this form attaches to the type (see typedef_forms.cpp)
enum [[cerastes::a_enum]] E { e0 [[cerastes::a_enumerator]] };
namespace [[cerastes::a_ns]] N { int in_ns; }
struct [[cerastes::a_class]] S {
  [[cerastes::a_field]] int f;
  [[cerastes::a_method]] int m() const;
  [[cerastes::a_ctor]] S() : f(0) {}
  [[cerastes::a_dtor]] ~S() {}
  [[cerastes::a_static]] static int sv;
};
struct [[cerastes::a_derived]] D : [[cerastes::a_base]] S {};
int param_fn([[cerastes::a_param]] int p) { return p; }
template <class T> T tf(T v) { return v; }   // (attributes on template parameters: see template_param_attrs.cpp)
int body_fn(int n) {
  [[cerastes::a_local]] int local = n;                // local variable declaration
  [[cerastes::a_return]] return local;                // return statement
}
int lambda_fn() {
  auto l = [](int x) [[cerastes::a_lambda]] { return x; };
  return l(1);
}
int use() { return param_fn(1) + tf(2) + body_fn(3) + lambda_fn(); }
