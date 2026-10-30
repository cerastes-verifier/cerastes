// (included by selftest.cpp) Mutable globals and `static` locals (decision 0041, M4.5b2a): `program_start` and the entry (item 4: the facts, one row
// per global, `main` holds every global from the start), a global read and written under `owns(g)` and `reads(g)` (items 6-9), `old(g)` and the frame of
// a callee that names no global, a call that needs `owns(g)` without it (item 8: `permission`), the aliasing trap `two(&g)` with `owns(*p) && owns(g)`
// (item 8), a loop over a global in `main` and in a function that also owns `*p` (item 10), a `static` local with its implicit atoms through a recursive
// function (item 12), `g + f()` with `f` owning `g` (item 11: `unsequenced`), an unknown initial value (item 3), a pointer global initialized with `&h`,
// and `&g` compared with a parameter (item 6: the liveness disjunct).
// PRE-REGISTERED from the record before lowering ran the cases: each expectation names the category and the VIR node whose position the record gives (the
// read for `unsequenced`, the store for `permission`, the call for a call's checks, the check, the return or the closing brace for a postcondition), and
// each rule has a case that verifies and a twin that fails for one reason (invariant 2).  A case that says `fail_only(c, loc, n, i)` has n checks of that
// category at that line, and only the i-th (in Viper line order) fails.
namespace {

void define_globals() {
  auto PT = [](Kit& k) { return k.ptr(k.i32); };
  auto N = [](Kit& k, const char* v) { return k.num(k.i32, v); };
  auto FP = [](Kit& k, Function* f, int i) { return k.b.text(k.F(f, i), f->params[static_cast<size_t>(i)].name); };
  // a global of type int with its initial value, or an unknown one (external linkage in C++, question 84)
  auto glob = [](Kit& k, const char* name, const char* v, const Function* owner = nullptr, bool unknown = false) -> Global* {
    Global* g = k.prog.add_global(name, k.i32, k.at(), owner);
    if (unknown) {
      g->init_kind = Global::Init::Unknown;
    } else {
      g->init = k.num(k.i32, v);
      g->value = v;
    }
    return g;
  };
  auto ga = [](Kit& k, const Global* g) { return k.b.global_addr(g, k.at()); };
  auto rd = [=](Kit& k, const Global* g) { return k.deref(ga(k, g)); };  // a read of g
  auto gatom = [=](Kit& k, AtomKind kind, const Global* g, int amount, bool implicit = false) {
    return k.b.global_atom(kind, ga(k, g), amount, k.at(), implicit);
  };
  auto wr = [=](Kit& k, const Global* g, const Expr* v) { return k.store(ga(k, g), v); };
  auto pown = [=](Kit& k, Function* f, int i, int amount) { return k.b.atom(AtomKind::Owns, FP(k, f, i), amount, k.at()); };
  auto plive = [=](Kit& k, Function* f, int i, int amount) { return k.b.atom(AtomKind::Live, FP(k, f, i), amount, k.at()); };
  auto call0 = [=](Kit& k, Function* f) { return k.es(k.call(f, {})); };

  // ---- item 4: program_start and the entry ----
  // static int g = 3, h = 7; int main() { check(g == 3 && h == 7); g = 10; check(g == [10|11] && &g != &h); return 0; }
  const auto start_main = [=](Kit& k, bool wrong) -> SrcLoc {
    Global* g = glob(k, "g", "3");
    Global* h = glob(k, "h", "7");
    Function* m = k.fn("main", k.i32);
    const Stmt* c1 = k.chk(k.land(k.eq_c(rd(k, g), k.i32, "3"), k.eq_c(rd(k, h), k.i32, "7")));
    const Stmt* w = wr(k, g, N(k, "10"));
    const Stmt* c2 = k.chk(k.land(k.eq_c(rd(k, g), k.i32, wrong ? "11" : "10"), k.ne(ga(k, g), ga(k, h))));
    m->body = k.blk({c1, w, c2, k.ret(N(k, "0"))});
    k.done(m);
    k.prog.set_entry(m);
    return c2->loc;
  };
  lc("glob_start_main", "program_start gives main g == 3 and h == 7 with their permissions: main reads and writes them, and &g != &h", [=](Kit& k) { start_main(k, false); });
  lc("glob_start_main_check", "after g = 10 the check g == 11 fails", [=](Kit& k) { k.fail("check", start_main(k, true)); });

  // int distinct() ensures(r == [1|0]) { return &g != &h; }   (holds no permission: the facts, unique blocks, the liveness disjunct)
  const auto distinct = [=](Kit& k, bool wrong) -> SrcLoc {
    Global* g = glob(k, "g", "3");
    Global* h = glob(k, "h", "7");
    Function* f = k.fn("distinct", k.i32);
    k.ens(f, k.eq_c(k.R(f), k.i32, wrong ? "0" : "1"));
    const Stmt* r = k.ret(k.bi(k.i32, k.ne(ga(k, g), ga(k, h))));
    f->body = k.blk({r});
    k.done(f);
    return r->loc;
  };
  lc("glob_distinct", "&g != &h in a function that holds nothing: two unique blocks, and a global's liveness never ends", [=](Kit& k) { distinct(k, false); });
  lc("glob_distinct_wrong", "the same function claiming r == 0", [=](Kit& k) { k.fail("postcondition", distinct(k, true)); });

  // ---- items 6-9: owns(g), old(g), a call needing it ----
  // void bump() requires(owns(g) && g < 100) ensures(owns(g) && g == old(g) [+ 1]) { g += 1; }
  const auto make_bump = [=](Kit& k, Global* g, bool wrong) -> Function* {
    Function* f = k.fn("bump", k.vd);
    k.req(f, k.land(gatom(k, AtomKind::Owns, g, 0), k.lt(rd(k, g), N(k, "100"))));
    const Expr* want = wrong ? k.old(rd(k, g)) : k.add(k.old(rd(k, g)), N(k, "1"));
    k.ens(f, k.land(gatom(k, AtomKind::Owns, g, 0), k.eq(rd(k, g), want)));
    f->body = k.blk({k.cstore(ga(k, g), BinaryOp::Add, N(k, "1"))});
    k.done(f);
    return f;
  };
  // int main() { check(g == 3); bump(); check(g == [4|3]); g = [10|100]; bump(); check(g == 11); }   mode 0 verifies, 1: the wrong check, 3: g = 100 first
  const auto owns_call = [=](Kit& k, int mode) -> SrcLoc {
    Global* g = glob(k, "g", "3");
    Function* bump = make_bump(k, g, false);
    Function* m = k.fn("main", k.i32);
    const Stmt* c0 = k.chk(k.eq_c(rd(k, g), k.i32, "3"));
    const Stmt* b1 = call0(k, bump);
    const Stmt* c1 = k.chk(k.eq_c(rd(k, g), k.i32, mode == 1 ? "3" : "4"));
    const Stmt* w = wr(k, g, N(k, mode == 3 ? "100" : "10"));
    const Expr* c2e = k.call(bump, {});
    const Stmt* b2 = k.es(c2e);
    const Stmt* c2 = k.chk(k.eq_c(rd(k, g), k.i32, "11"));
    m->body = k.blk({c0, b1, c1, w, b2, c2, k.ret(N(k, "0"))});
    k.done(m);
    k.prog.set_entry(m);
    return mode == 1 ? c1->loc : c2e->loc;
  };
  lc("glob_owns_call", "main owns g from program_start: bump() with owns(g) and old(g) across two calls, g written between them", [=](Kit& k) { owns_call(k, 0); });
  lc("glob_owns_call_check", "bump's ensures say g == old(g) + 1: g == 3 after the first call fails", [=](Kit& k) { k.fail("check", owns_call(k, 1)); });
  lc("glob_owns_call_precondition", "main sets g = 100, bump requires g < 100: precondition at the call", [=](Kit& k) { k.fail("precondition", owns_call(k, 3)); });
  lc("glob_old_post", "bump claims g == old(g) but adds 1: postcondition at the closing brace", [=](Kit& k) {
    Global* g = glob(k, "g", "3");
    Function* f = make_bump(k, g, true);
    k.fail("postcondition", f->end_loc);
  });
  // void helper() { bump(); }   (names no global: it holds no permission to g)
  lc("glob_owns_call_unheld", "helper's contract names no global, so it does not own g: the call of bump needs owns(g): permission at the call", [=](Kit& k) {
    Global* g = glob(k, "g", "3");
    Function* bump = make_bump(k, g, false);
    Function* f = k.fn("helper", k.vd);
    const Expr* c = k.call(bump, {});
    f->body = k.blk({k.es(c)});
    k.done(f);
    k.fail_only("permission", c->loc, 11, 3);  // the write permission of g: the checks before it pass
  });
  lc("glob_unowned_write", "a function whose contract names no global writes g: permission at the store", [=](Kit& k) {
    Global* g = glob(k, "g", "3");
    Function* f = k.fn("poke", k.vd);
    const Stmt* w = wr(k, g, N(k, "5"));
    f->body = k.blk({w});
    k.done(f);
    k.fail("permission", w->loc);
  });
  lc("glob_unowned_read", "a function whose contract names no global reads g: permission at the read", [=](Kit& k) {
    Global* g = glob(k, "g", "3");
    Function* f = k.fn("peek", k.i32);
    const Expr* r = rd(k, g);
    f->body = k.blk({k.ret(r)});
    k.done(f);
    k.fail("permission", r->loc);
  });

  // ---- reads(g) and the frame ----
  // int peek() requires(reads(g)) ensures(reads(g) && r == g) { return g; }   void frame() {}   void clobber() requires(owns(g)) ensures(owns(g)) {}
  // int main() { int a = peek(); frame(); check(a == 3 && g == 3 && h == 7); [clobber(); check(g == 3);] }
  const auto reads_frame = [=](Kit& k, bool clobber) -> SrcLoc {
    Global* g = glob(k, "g", "3");
    Global* h = glob(k, "h", "7");
    Function* peek = k.fn("peek", k.i32);
    k.req(peek, gatom(k, AtomKind::Reads, g, 0));
    k.ens(peek, k.land(gatom(k, AtomKind::Reads, g, 0), k.eq(k.R(peek), rd(k, g))));
    peek->body = k.blk({k.ret(rd(k, g))});
    k.done(peek);
    Function* frame = k.fn("frame", k.vd);
    frame->body = k.blk({});
    k.done(frame);
    Function* cl = k.fn("clobber", k.vd);
    k.req(cl, gatom(k, AtomKind::Owns, g, 0));
    k.ens(cl, gatom(k, AtomKind::Owns, g, 0));
    cl->body = k.blk({});
    k.done(cl);
    Function* m = k.fn("main", k.i32);
    Var* a = k.local(m, "a", k.i32);
    std::vector<const Stmt*> s = {k.decl(a, k.call(peek, {})), call0(k, frame),
                                  k.chk(k.land(k.land(k.eq_c(k.v(a), k.i32, "3"), k.eq_c(rd(k, g), k.i32, "3")), k.eq_c(rd(k, h), k.i32, "7")))};
    const Stmt* last = nullptr;
    if (clobber) {
      s.push_back(call0(k, cl));
      last = k.chk(k.eq_c(rd(k, g), k.i32, "3"));
      s.push_back(last);
    }
    s.push_back(k.ret(N(k, "0")));
    m->body = k.blk(s);
    k.done(m);
    k.prog.set_entry(m);
    return last != nullptr ? last->loc : SrcLoc{};
  };
  lc("glob_reads_frame", "a reads(g) callee leaves g as it was, a callee that names no global changes neither g nor h", [=](Kit& k) { reads_frame(k, false); });
  lc("glob_reads_havoc", "clobber owns g and says nothing of its value: the caller's check g == 3 after it fails", [=](Kit& k) { k.fail("check", reads_frame(k, true)); });
  lc("glob_reads_write", "a callee with only reads(g) writes g: permission at the store", [=](Kit& k) {
    Global* g = glob(k, "g", "3");
    Function* f = k.fn("poke", k.vd);
    k.req(f, gatom(k, AtomKind::Reads, g, 0));
    k.ens(f, gatom(k, AtomKind::Reads, g, 0));
    const Stmt* w = wr(k, g, N(k, "5"));
    f->body = k.blk({w});
    k.done(f);
    k.fail("permission", w->loc);
  });

  // ---- item 8: owns(*p) && owns(g) and the aliasing trap ----
  // void two(int *p) requires(owns(*p) && owns(g)) ensures(owns(*p) && owns(g) && *p == 1 && g == 2) { *p = 1; g = 2; }
  // int main() { int x = 0; two(&[x|g]); check(...); }
  const auto two = [=](Kit& k, bool alias) -> const Expr* {
    Global* g = glob(k, "g", "3");
    Function* f = k.fn("two", k.vd, {{"p", PT(k)}});
    k.req(f, k.land(pown(k, f, 0, 0), gatom(k, AtomKind::Owns, g, 1)));
    k.ens(f, k.land(k.land(pown(k, f, 0, 0), gatom(k, AtomKind::Owns, g, 1)),
                    k.land(k.eq_c(k.deref(FP(k, f, 0)), k.i32, "1"), k.eq_c(rd(k, g), k.i32, "2"))));
    f->body = k.blk({k.store(k.pn(f->param_var(0)), N(k, "1")), wr(k, g, N(k, "2"))});
    k.done(f);
    Function* m = k.fn("main", k.i32);
    Var* x = k.cell(m, "x", k.i32);
    const Expr* c = k.call(f, {alias ? ga(k, g) : k.addr(x)});
    if (alias) {
      m->body = k.blkc({k.decl(x, N(k, "0")), k.es(c), k.ret(N(k, "0"))});
    } else {
      m->body = k.blkc({k.decl(x, N(k, "0")), k.es(c), k.chk(k.land(k.eq_c(rd(k, g), k.i32, "2"), k.eq_c(k.deref(k.addr(x)), k.i32, "1"))), k.ret(N(k, "0"))});
    }
    k.done(m);
    k.prog.set_entry(m);
    return c;
  };
  lc("glob_two_atoms", "two(&x) with owns(*p) && owns(g): both full, p != &g inside the callee, the caller learns *p == 1 and g == 2", [=](Kit& k) { two(k, false); });
  lc("glob_two_atoms_alias", "two(&g) passes g's cell as *p and as g: the separation p != &g fails at the call (the aliasing trap)", [=](Kit& k) {
    const Expr* c = two(k, true);
    k.fail_only("permission", c->loc, 23, 16);  // the separation row: the others pass
  });

  // ---- item 10: loops ----
  // int main() { int i = 0; while (i < 10) inv 0 <= i <= 10 [&& h == 7 + i | && h <= 7 + i] { h++; i++; } check(g == 3 && h == 17); }
  const auto loop_main = [=](Kit& k, bool weak) -> SrcLoc {
    Global* g = glob(k, "g", "3");
    Global* h = glob(k, "h", "7");
    Function* m = k.fn("main", k.i32);
    Var* i = k.local(m, "i", k.i32);
    const Expr* bound = k.range(k.v(i), k.i32, "0", "10");
    const Expr* hv = weak ? k.le(rd(k, h), k.add(N(k, "7"), k.v(i))) : k.eq(rd(k, h), k.add(N(k, "7"), k.v(i)));
    const Stmt* body = k.blk({k.idstore(ga(k, h), true), k.asg(i, k.add(k.v(i), N(k, "1")))});
    const Stmt* lp = k.whl(k.lt(k.v(i), N(k, "10")), {k.cl(bound), k.cl(hv)}, {k.cl(k.sub(N(k, "10"), k.v(i)))}, body);
    const Stmt* c = k.chk(k.land(k.eq_c(rd(k, g), k.i32, "3"), k.eq_c(rd(k, h), k.i32, "17")));
    m->body = k.blk({k.decl(i, N(k, "0")), lp, c, k.ret(N(k, "0"))});
    k.done(m);
    k.prog.set_entry(m);
    return c->loc;
  };
  lc("glob_loop_main", "in main every global is an automatic place of the loop: g, which nothing writes, keeps its value; h's value is the user's invariant", [=](Kit& k) { loop_main(k, false); });
  lc("glob_loop_main_weak", "with h <= 7 + i the value of h after the loop is not 17: the check fails (g is still 3)", [=](Kit& k) { k.fail("check", loop_main(k, true)); });

  // void fill(int *p) requires(owns(*p) && owns(g) && g == 5) ensures(owns(*p) && owns(g) && g == 5)
  // { int i = 0; while (i < 10) inv 0 <= i <= 10 [&& g == 5] { *p = i; i++; } }
  const auto loop_ptr = [=](Kit& k, bool invariant) -> SrcLoc {
    Global* g = glob(k, "g", "5");
    Function* f = k.fn("fill", k.vd, {{"p", PT(k)}});
    k.req(f, k.land(k.land(pown(k, f, 0, 0), gatom(k, AtomKind::Owns, g, 1)), k.eq_c(rd(k, g), k.i32, "5")));
    k.ens(f, k.land(k.land(pown(k, f, 0, 0), gatom(k, AtomKind::Owns, g, 1)), k.eq_c(rd(k, g), k.i32, "5")));
    Var* i = k.local(f, "i", k.i32);
    const Expr* bound = k.range(k.v(i), k.i32, "0", "10");
    const Expr* inv = invariant ? k.land(bound, k.eq_c(rd(k, g), k.i32, "5")) : bound;
    const Stmt* body = k.blk({k.store(k.pn(f->param_var(0)), k.v(i)), k.asg(i, k.add(k.v(i), N(k, "1")))});
    f->body = k.blk({k.decl(i, N(k, "0")), k.whl(k.lt(k.v(i), N(k, "10")), {k.cl(inv)}, {k.cl(k.sub(N(k, "10"), k.v(i)))}, body)});
    k.done(f);
    return f->end_loc;
  };
  lc("glob_loop_ptr", "a store through p can reach g (the same field type): g's value across the loop is the user's invariant g == 5", [=](Kit& k) { loop_ptr(k, true); });
  lc("glob_loop_ptr_no_invariant", "without g == 5 in the invariant the loop may have changed g: the postcondition fails at the closing brace", [=](Kit& k) { k.fail("postcondition", loop_ptr(k, false)); });

  // ---- item 12: a static local, its implicit atoms, a recursive function ----
  // void f() { static int count = 0; [if (count < 100)] count++; }   void r(int n) requires(0 <= n <= 10) decreases(n) { if (n > 0) { f(); r(n - 1); } }
  // int main() { f(); r(3); }      mode 0 verifies, 1: f without its bound (overflow), 2: r without the implicit atom (permission at its call of f)
  const auto statics = [=](Kit& k, int mode) -> SrcLoc {
    Function* f = k.fn("f", k.vd);
    Global* s = glob(k, "count", "0", f);
    k.req(f, gatom(k, AtomKind::Owns, s, 0, true));
    k.ens(f, gatom(k, AtomKind::Owns, s, 0, true));
    SrcLoc bad;
    if (mode == 1) {
      const Stmt* inc = k.idstore(ga(k, s), true);
      bad = inc->loc;
      f->body = k.blk({inc});
    } else {
      f->body = k.blk({k.iff(k.lt(rd(k, s), N(k, "100")), k.blk({k.idstore(ga(k, s), true)}))});
    }
    k.done(f);
    Function* r = k.fn("r", k.vd, {{"n", k.i32}});
    k.req(r, k.range(k.F(r, 0), k.i32, "0", "10"));
    if (mode != 2) {
      k.req(r, gatom(k, AtomKind::Owns, s, 0, true));
      k.ens(r, gatom(k, AtomKind::Owns, s, 0, true));
    }
    k.dec(r, k.F(r, 0));
    const Expr* cf = k.call(f, {});
    if (mode == 2) bad = cf->loc;
    r->body = k.blk({k.iff(k.gt(k.pv(r, 0), N(k, "0")), k.blk({k.es(cf), k.es(k.call(r, {k.sub(k.pv(r, 0), N(k, "1"))}))}))});
    k.done(r);
    Function* m = k.fn("main", k.i32);
    m->body = k.blk({call0(k, f), k.es(k.call(r, {N(k, "3")})), k.ret(N(k, "0"))});
    k.done(m);
    k.prog.set_entry(m);
    return bad;
  };
  lc("glob_static_recursion", "the static local count of f is held by f and by r (which calls f and itself) as an implicit owns, and by main from program_start", [=](Kit& k) { statics(k, 0); });
  lc("glob_static_overflow", "the value of a static local is unknown on entry: count++ without a bound may overflow", [=](Kit& k) { k.fail("overflow", statics(k, 1)); });
  lc("glob_static_unheld", "r without the implicit atom holds no permission to count: permission at its call of f", [=](Kit& k) { k.fail_only("permission", statics(k, 2), 11, 3); });

  // ---- item 11: g + f() with f owning g ----
  // int bumpr() requires(owns(g) && g < 100) ensures(owns(g)) { return 1; }   int main() { int s = [g|h] + bumpr(); return 0; }
  const auto beside_call = [=](Kit& k, bool same) -> const Expr* {
    Global* g = glob(k, "g", "3");
    Global* h = glob(k, "h", "7");
    Function* f = k.fn("bumpr", k.i32);
    k.req(f, k.land(gatom(k, AtomKind::Owns, g, 0), k.lt(rd(k, g), N(k, "100"))));
    k.ens(f, k.land(gatom(k, AtomKind::Owns, g, 0), k.eq_c(k.R(f), k.i32, "1")));
    f->body = k.blk({k.ret(N(k, "1"))});
    k.done(f);
    Function* m = k.fn("main", k.i32);
    Var* s = k.local(m, "s", k.i32);
    const Expr* r = rd(k, same ? g : h);
    m->body = k.blk({k.decl(s, k.add(r, k.call(f, {}))), k.ret(N(k, "0"))});
    k.done(m);
    k.prog.set_entry(m);
    return r;
  };
  lc("glob_seq_other_global", "h + bumpr() where bumpr owns g: the call is a memory call on a different place than the read of h", [=](Kit& k) { beside_call(k, false); });
  lc("glob_seq_call_owns", "g + bumpr() with bumpr owning g: C does not order the read and the call, the places are equal: unsequenced at the read", [=](Kit& k) { k.fail("unsequenced", beside_call(k, true)->loc); });

  // ---- item 3: an unknown initial value ----
  // int g (external linkage in C++: unknown); int main() { [check(g == 0);] g = 6; check(g == 6); }
  const auto unknown_init = [=](Kit& k, bool relies) -> SrcLoc {
    Global* g = glob(k, "g", "0", nullptr, true);
    Function* m = k.fn("main", k.i32);
    std::vector<const Stmt*> s;
    const Stmt* c0 = k.chk(k.eq_c(rd(k, g), k.i32, "0"));
    if (relies) s.push_back(c0);
    s.push_back(wr(k, g, N(k, "6")));
    s.push_back(k.chk(k.eq_c(rd(k, g), k.i32, "6")));
    s.push_back(k.ret(N(k, "0")));
    m->body = k.blk(s);
    k.done(m);
    k.prog.set_entry(m);
    return c0->loc;
  };
  lc("glob_unknown_init", "an unknown initial value is initialized and in range: main may write it and then know it", [=](Kit& k) { unknown_init(k, false); });
  lc("glob_unknown_init_check", "main must not rely on the initial value: check(g == 0) fails", [=](Kit& k) { k.fail("check", unknown_init(k, true)); });

  // ---- a pointer global initialized with &h ----
  // static int h = 7; static int *gp = &h; int main() { if (gp) *gp = 5; check(h == [5|7]); }
  const auto ptr_global = [=](Kit& k, bool wrong) -> SrcLoc {
    Global* h = glob(k, "h", "7");
    Global* gp = k.prog.add_global("gp", PT(k), k.at());
    gp->init_kind = Global::Init::Addr;
    gp->target = h;
    Function* m = k.fn("main", k.i32);
    const Stmt* st = k.store(rd(k, gp), N(k, "5"));
    const Stmt* c = k.chk(k.eq_c(rd(k, h), k.i32, wrong ? "7" : "5"));
    m->body = k.blk({k.iff(k.ne(rd(k, gp), k.nul(PT(k))), k.blk({st})), c, k.ret(N(k, "0"))});
    k.done(m);
    k.prog.set_entry(m);
    return c->loc;
  };
  lc("glob_ptr_global", "gp == &h at program start: main's *gp = 5 writes h", [=](Kit& k) { ptr_global(k, false); });
  lc("glob_ptr_global_check", "the write through gp is the write of h: h == 7 fails afterwards", [=](Kit& k) { k.fail("check", ptr_global(k, true)); });

  // ---- item 6: the liveness disjunct ----
  // int cmp(int *p) requires(owns(*p)) ensures(owns(*p)) { return &g == p; }   (nothing about g is held)   mode 1: no atom for p, 2: live(p) only
  const auto cmp_param = [=](Kit& k, int mode) -> std::pair<SrcLoc, SrcLoc> {  // (the use of p, the comparison)
    Global* g = glob(k, "g", "3");
    glob(k, "h", "7");
    Function* f = k.fn("cmp", k.i32, {{"p", PT(k)}});
    if (mode == 0) {
      k.req(f, pown(k, f, 0, 0));
      k.ens(f, pown(k, f, 0, 0));
    } else if (mode == 2) {
      k.req(f, plive(k, f, 0, 0));
      k.ens(f, plive(k, f, 0, 0));
    }
    const Expr* pe = k.pn(f->param_var(0));
    const Expr* e = k.eq(ga(k, g), pe);
    f->body = k.blk({k.ret(k.bi(k.i32, e))});
    k.done(f);
    return {pe->loc, e->loc};
  };
  lc("glob_cmp_param", "&g == p with owns(*p) and nothing held of g: &g's liveness passes by the disjunct, p is a cell", [=](Kit& k) { cmp_param(k, 0); });
  lc("glob_cmp_param_unlive", "the same with no atom for p: p may have ended: lifetime at its use (and the comparison, which Silicon goes on to check, may compare a one-past pointer)", [=](Kit& k) {
    const auto at = cmp_param(k, 1);
    k.fail("lifetime", at.first);
    k.fail("pointer-comparison", at.second);
  });
  lc("glob_cmp_param_live_only", "the same with live(p) only: p may be one past its object, equal to &g: pointer-comparison at the comparison", [=](Kit& k) { k.fail("pointer-comparison", cmp_param(k, 2).second); });
}

}  // namespace
