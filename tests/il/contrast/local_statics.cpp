// expect: golden
// Contrast probe: initializers of local static variables in a function
// template's body.  `stat` has statics with a dependent constant initializer
// (`(T)5 * 2`), a sizeof initializer, a non-dependent constant one
// (`9 - 2`) and a dynamic one (`v + 1`), and the same four again in a nested
// block.  Static locals are initialized separately from the statements around
// them, so an initializer can be recorded outside the statement tree.  The
// `op` lines of the prototype must be exactly the operations of the
// `enk_operation` nodes in its il_display function region: none missing, none
// from elsewhere.

template <typename T>
T stat(T v) {
  static T st = (T)5 * 2;
  static int si = sizeof(T) + 1;
  static const int s = 9 - 2;
  static T dyn = v + 1;
  {
    static T st2 = (T)5 * 2;
    static int si2 = sizeof(T) + 1;
    static const int s2 = 9 - 2;
    static T dyn2 = v + 1;
    st2 = st2 + si2 + s2 + dyn2;
  }
  return st + si + s + dyn;
}
