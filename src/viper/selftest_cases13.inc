// (included by selftest.cpp) Initialization through pointers (decision 0042, M4.5b2b): `[[cerastes::must_init]]` and `[[cerastes::ref_to_uninit]]` on a
// pointer parameter (item 5: the `requires` atom without its flag, the return rows `postcondition` and `old(flag) ==> flag`, a loop over the place, a
// pointer cell), the call's flag row as `uninitialized` (item 7, questions 82 and 85: an owns or reads parameter, a global's atom), an object that was
// initialized stays initialized after a `ref_to_uninit` call (question 87), a marked parameter passed on, and the backstop rejections of `check_program`
// (items 4 and 6).
// PRE-REGISTERED from the record before lowering ran the cases: each expectation names the category and the VIR node whose position the record gives (the
// closing brace for `must_init`'s `postcondition`, the read for `uninitialized`, the call for a call's flag row), and each rule has a case that
// verifies and a twin that fails for one reason (invariant 2).
namespace {

void define_init_markers() {
  auto PT = [](Kit& k) { return k.ptr(k.i32); };
  auto PP = [](Kit& k) { return k.ptr(k.ptr(k.i32)); };
  auto N = [](Kit& k, const char* v) { return k.num(k.i32, v); };
  auto FP = [](Kit& k, Function* f, int i) { return k.b.text(k.F(f, i), f->params[static_cast<size_t>(i)].name); };
  auto owns = [=](Kit& k, Function* f, int i, int amount) { return k.b.atom(AtomKind::Owns, FP(k, f, i), amount, k.at()); };
  auto reads = [=](Kit& k, Function* f, int i, int amount) { return k.b.atom(AtomKind::Reads, FP(k, f, i), amount, k.at()); };
  auto star = [=](Kit& k, Function* f, int i) { return k.deref(FP(k, f, i)); };  // `*p` in a contract
  auto old_star = [=](Kit& k, Function* f, int i) { return k.old(star(k, f, i)); };
  auto mark = [](Function* f, int i, InitMarker m) { f->params[static_cast<size_t>(i)].init = m; };
  // A cell `x` of a caller with and without an initializer.
  auto cell_init = [=](Kit& k, Function* f, const char* name, const char* v) {
    Var* x = k.cell(f, name, k.i32);
    return std::make_pair(x, k.decl(x, N(k, v)));
  };
  auto cell_bare = [=](Kit& k, Function* f, const char* name) {
    Var* x = k.cell(f, name, k.i32);
    return std::make_pair(x, k.decl(x, nullptr));
  };

  // ---- item 5: the callee of a `must_init` parameter ----
  // void init(int *p [must_init][, int c]) requires(owns(*p)) ensures(owns(*p) [&& *p == 0]) { [if (c != 0)] *p = 0; }   (mode 1: the write is conditional)
  const auto make_init = [=](Kit& k, int mode) -> Function* {
    std::vector<Kit::P> ps = {{"p", PT(k)}};
    if (mode == 1) ps.push_back({"c", k.i32});
    Function* f = k.fn("init", k.vd, ps);
    mark(f, 0, InitMarker::MustInit);
    k.req(f, owns(k, f, 0, 0));
    k.ens(f, mode == 1 ? owns(k, f, 0, 0) : k.land(owns(k, f, 0, 0), k.eq_c(star(k, f, 0), k.i32, "0")));
    const Stmt* w = k.store(k.pn(f->param_var(0)), N(k, "0"));
    f->body = k.blk({mode == 1 ? k.iff(k.ne(k.pv(f, 1), N(k, "0")), w) : w});
    k.done(f);
    return f;
  };
  lc("init_must_init", "void init(int *p [must_init]) requires(owns(*p)) ensures(owns(*p) && *p == 0) { *p = 0; }: the requires has no flag, the write gives it", [=](Kit& k) { make_init(k, 0); });
  lc("init_must_init_missing_write", "the write is under `if (c != 0)`: the flag row at the closing brace is the postcondition `must_init` promises", [=](Kit& k) {
    Function* f = make_init(k, 1);
    k.fail("postcondition", f->end_loc);
  });

  // ---- item 7: the caller of a `must_init` callee ----
  // int use([int c]) ensures(r == 0) { int x; [if (c != 0)] init(&x); return x; }   (mode 1: the call is conditional)
  const auto use_init = [=](Kit& k, int mode) -> const Expr* {
    Function* init = make_init(k, 0);
    Function* f = mode == 1 ? k.fn("use", k.i32, {{"c", k.i32}}) : k.fn("use", k.i32);
    if (mode == 0) k.ens(f, k.eq_c(k.R(f), k.i32, "0"));
    auto [x, dx] = cell_bare(k, f, "x");
    const Stmt* call = k.es(k.call(init, {k.addr(x)}));
    const Expr* rd = k.v(x);
    f->body = k.blkc({dx, mode == 1 ? k.iff(k.ne(k.pv(f, 0), N(k, "0")), call) : call, k.ret(rd)});
    k.done(f);
    return rd;
  };
  lc("init_must_init_caller", "int x; init(&x); return x: no flag row at the call (the parameter is must_init), and x has a value after it", [=](Kit& k) { use_init(k, 0); });
  lc("init_must_init_caller_maybe", "init(&x) only when c != 0: the other path reads x uninitialized", [=](Kit& k) { k.fail("uninitialized", use_init(k, 1)->loc); });

  // ---- item 5: ref_to_uninit alone ----
  // int fill(int *p [ref_to_uninit]) requires(owns(*p)) ensures(r, owns(*p) && r == 7) { [*p = 7;] return *p; }   (mode 1: no write, the read is first)
  const auto make_fill = [=](Kit& k, int mode) -> const Expr* {
    Function* f = k.fn("fill", k.i32, {{"p", PT(k)}});
    mark(f, 0, InitMarker::RefToUninit);
    k.req(f, owns(k, f, 0, 0));
    k.ens(f, mode == 1 ? owns(k, f, 0, 0) : k.land(owns(k, f, 0, 0), k.eq_c(k.R(f), k.i32, "7")));
    const Expr* rd = k.deref(k.pn(f->param_var(0)));
    f->body = k.blk({mode == 1 ? k.ret(rd) : k.blk({k.store(k.pn(f->param_var(0)), N(k, "7")), k.ret(rd)})});
    k.done(f);
    return rd;
  };
  lc("init_ref_to_uninit", "ref_to_uninit alone: the callee writes *p and then reads it", [=](Kit& k) { make_fill(k, 0); });
  lc("init_ref_to_uninit_read_first", "the callee reads *p first: its flag is not required on entry, so uninitialized at the read", [=](Kit& k) { k.fail("uninitialized", make_fill(k, 1)->loc); });

  // void leave(int *p [ref_to_uninit]) requires(owns(*p)) ensures(owns(*p)) { }   and its caller `int x; leave(&x); [return x;]`
  const auto make_leave = [=](Kit& k) -> Function* {
    Function* f = k.fn("leave", k.vd, {{"p", PT(k)}});
    mark(f, 0, InitMarker::RefToUninit);
    k.req(f, owns(k, f, 0, 0));
    k.ens(f, owns(k, f, 0, 0));
    f->body = k.blk({});
    k.done(f);
    return f;
  };
  const auto use_leave = [=](Kit& k, bool read) -> const Expr* {
    Function* leave = make_leave(k);
    Function* f = read ? k.fn("use", k.i32) : k.fn("use", k.vd);
    auto [x, dx] = cell_bare(k, f, "x");
    const Expr* rd = read ? k.v(x) : nullptr;
    f->body = k.blkc({dx, k.es(k.call(leave, {k.addr(x)})), read ? k.ret(rd) : k.es(N(k, "0"))});
    k.done(f);
    return rd;
  };
  lc("init_ref_leave_fresh", "int x; leave(&x): a fresh cell to a ref_to_uninit parameter has no flag row and its range row holds only under the flag", [=](Kit& k) { use_leave(k, false); });
  lc("init_ref_leave_read", "int x; leave(&x); return x: ref_to_uninit alone promises no value", [=](Kit& k) { k.fail("uninitialized", use_leave(k, true)->loc); });

  // ---- question 87: an initialized object stays initialized after a ref_to_uninit call ----
  // void maybe(int *p [ref_to_uninit], int c) requires(owns(*p)) ensures(owns(*p)) { if (c != 0) *p = 5; }
  // int keep(int c) { int x [= 1]; maybe(&x, c); return x; }
  const auto make_maybe = [=](Kit& k) -> Function* {
    Function* f = k.fn("maybe", k.vd, {{"p", PT(k)}, {"c", k.i32}});
    mark(f, 0, InitMarker::RefToUninit);
    k.req(f, owns(k, f, 0, 0));
    k.ens(f, owns(k, f, 0, 0));
    f->body = k.blk({k.iff(k.ne(k.pv(f, 1), N(k, "0")), k.store(k.pn(f->param_var(0)), N(k, "5")))});
    k.done(f);
    return f;
  };
  const auto keep = [=](Kit& k, bool initialized) -> const Expr* {
    Function* maybe = make_maybe(k);
    Function* f = k.fn("keep", k.i32, {{"c", k.i32}});
    auto [x, dx] = initialized ? cell_init(k, f, "x", "1") : cell_bare(k, f, "x");
    const Expr* rd = k.v(x);
    f->body = k.blkc({dx, k.es(k.call(maybe, {k.addr(x), k.pv(f, 0)})), k.ret(rd)});
    k.done(f);
    return rd;
  };
  lc("init_ref_keeps", "int x = 1; maybe(&x, c); return x: old(flag) ==> flag, so the read after the call is fine", [=](Kit& k) { keep(k, true); });
  lc("init_ref_keeps_uninit", "int x; maybe(&x, c); return x: the callee may not write it", [=](Kit& k) { k.fail("uninitialized", keep(k, false)->loc); });

  // ---- item 7, questions 82 and 85: the flag row of every other call is `uninitialized` ----
  // void set(int *p) requires(owns(*p)) ensures(owns(*p)) { *p = 1; }   int look(int *p) requires(reads(*p)) ensures(reads(*p) && r == *p)   (trusted)
  const auto make_set = [=](Kit& k) -> Function* {
    Function* d = k.fn("set", k.vd, {{"p", PT(k)}});
    k.req(d, owns(k, d, 0, 0));
    k.ens(d, owns(k, d, 0, 0));
    k.trusted(d);
    return d;
  };
  const auto make_look = [=](Kit& k) -> Function* {
    Function* d = k.fn("look", k.i32, {{"p", PT(k)}});
    k.req(d, reads(k, d, 0, 0));
    k.ens(d, k.land(reads(k, d, 0, 0), k.eq(k.R(d), star(k, d, 0))));
    k.trusted(d);
    return d;
  };
  const auto use_unmarked = [=](Kit& k, bool owns_param, bool initialized) -> const Expr* {
    Function* callee = owns_param ? make_set(k) : make_look(k);
    Function* f = k.fn("use", k.i32);
    auto [x, dx] = initialized ? cell_init(k, f, "x", "1") : cell_bare(k, f, "x");
    const Expr* c = k.call(callee, {k.addr(x)});
    f->body = k.blkc({dx, owns_param ? k.es(c) : k.es(c), k.ret(N(k, "0"))});
    k.done(f);
    return c;
  };
  lc("init_unmarked_owns", "int x = 1; set(&x): an owns parameter is given an object with a value", [=](Kit& k) { use_unmarked(k, true, true); });
  lc("init_unmarked_owns_uninit", "int x; set(&x): the flag row of the call is `uninitialized`, not `permission`", [=](Kit& k) {
    k.fail("uninitialized", use_unmarked(k, true, false)->loc);
  });
  lc("init_unmarked_reads", "int x = 1; look(&x): a reads parameter is given an object with a value", [=](Kit& k) { use_unmarked(k, false, true); });
  lc("init_unmarked_reads_uninit", "int x; look(&x): the same flag row for a reads parameter", [=](Kit& k) {
    k.fail("uninitialized", use_unmarked(k, false, false)->loc);
  });

  // a global's atom (question 85): `void bump() requires(owns(g)) ensures(owns(g)) { g = 1; }` and `void use() requires(owns(g)) ensures(owns(g)) { bump(); }`;
  // the call's flag row is `uninitialized` too (it cannot fail: program_start and the writes keep a global's flag)
  lc("init_global_flag_row", "a call that needs owns(g): its flag row is category `uninitialized` (never failing for a global), one row at the call", [=](Kit& k) {
    Global* g = k.prog.add_global("g", k.i32, k.at(), nullptr);
    g->init = N(k, "3");
    g->value = "3";
    auto ga = [&]() { return k.b.global_addr(g, k.at()); };
    Function* bump = k.fn("bump", k.vd);
    k.req(bump, k.b.global_atom(AtomKind::Owns, ga(), 0, k.at(), false));
    k.ens(bump, k.b.global_atom(AtomKind::Owns, ga(), 0, k.at(), false));
    bump->body = k.blk({k.store(ga(), N(k, "1"))});
    k.done(bump);
    Function* f = k.fn("use", k.vd);
    k.req(f, k.b.global_atom(AtomKind::Owns, ga(), 0, k.at(), false));
    k.ens(f, k.b.global_atom(AtomKind::Owns, ga(), 0, k.at(), false));
    const Expr* c = k.call(bump, {});
    f->body = k.blk({k.es(c)});
    k.done(f);
    k.rows("uninitialized", c->loc, 1);
  });

  // ---- pointer cells (item 4: a marker on T **pp covers the cell *pp, its range being WF) ----
  // void make(int **pp [must_init]) requires(owns(*pp)) ensures(owns(*pp) && *pp == NULL) { [if (c)] *pp = NULL; }
  const auto make_make = [=](Kit& k, bool conditional) -> Function* {
    std::vector<Kit::P> ps = {{"pp", PP(k)}};
    if (conditional) ps.push_back({"c", k.i32});
    Function* f = k.fn("make", k.vd, ps);
    mark(f, 0, InitMarker::MustInit);
    k.req(f, owns(k, f, 0, 0));
    k.ens(f, conditional ? owns(k, f, 0, 0) : k.land(owns(k, f, 0, 0), k.eq(star(k, f, 0), k.nul(PT(k)))));
    const Stmt* w = k.store(k.pn(f->param_var(0)), k.nul(PT(k)));
    f->body = k.blk({conditional ? k.iff(k.ne(k.pv(f, 1), N(k, "0")), w) : w});
    k.done(f);
    return f;
  };
  lc("init_ptrptr", "void make(int **pp [must_init]) { *pp = NULL; }: the pointer cell has WF in place of a range and gets its flag from the write", [=](Kit& k) { make_make(k, false); });
  lc("init_ptrptr_missing_write", "the pointer cell is written only when c != 0: postcondition at the closing brace", [=](Kit& k) { k.fail("postcondition", make_make(k, true)->end_loc); });
  lc("init_ptrptr_caller", "int *q; make(&q); return q == NULL: the pointer cell has a value after the call", [=](Kit& k) {
    Function* make = make_make(k, false);
    Function* f = k.fn("use", k.i32);
    k.ens(f, k.eq_c(k.R(f), k.i32, "1"));
    Var* q = k.cell(f, "q", PT(k));
    f->body = k.blkc({k.decl(q, nullptr), k.es(k.call(make, {k.addr(q)})),
                      k.ret(k.cond(k.eq(k.pn(q), k.nul(PT(k))), N(k, "1"), N(k, "0")))});
    k.done(f);
  });

  // ---- passing a must_init parameter on ----
  // void init_twice(int *p [must_init]) requires(owns(*p)) ensures(owns(*p) && *p == 0) { init(p) or set(p); }
  const auto pass_on = [=](Kit& k, bool to_marked) -> const Expr* {
    Function* callee = to_marked ? make_init(k, 0) : make_set(k);
    Function* f = k.fn("init_twice", k.vd, {{"p", PT(k)}});
    mark(f, 0, InitMarker::MustInit);
    k.req(f, owns(k, f, 0, 0));
    k.ens(f, to_marked ? k.land(owns(k, f, 0, 0), k.eq_c(star(k, f, 0), k.i32, "0")) : owns(k, f, 0, 0));
    const Expr* c = k.call(callee, {k.pn(f->param_var(0))});
    f->body = k.blk({k.es(c)});
    k.done(f);
    return c;
  };
  lc("init_pass_on", "a must_init parameter passed to another must_init callee: no flag row, and the callee's ensures give the flag back", [=](Kit& k) { pass_on(k, true); });
  lc("init_pass_on_unmarked", "passed to an unmarked owns callee: the flag row of that call is `uninitialized`", [=](Kit& k) { k.fail("uninitialized", pass_on(k, false)->loc); });

  // ---- item 5: a loop over the place in a must_init callee ----
  // void fill(int *p [must_init], int n) requires(owns(*p) && 0 <= n <= 100) ensures(owns(*p) [&& *p == n]) { int i = 0; [*p = 0;] while (i < n) inv ... { *p [+]= ...; i += 1; } }
  const auto loop_fill = [=](Kit& k, bool write_before) -> SrcLoc {
    Function* f = k.fn("fill", k.vd, {{"p", PT(k)}, {"n", k.i32}});
    mark(f, 0, InitMarker::MustInit);
    k.req(f, k.land(owns(k, f, 0, 0), k.range(k.F(f, 1), k.i32, "0", "100")));
    k.ens(f, write_before ? k.land(owns(k, f, 0, 0), k.eq(star(k, f, 0), k.F(f, 1))) : owns(k, f, 0, 0));
    Var* i = k.local(f, "i", k.i32);
    const Var* p = f->param_var(0);
    const Expr* inv = write_before ? k.land(k.land(k.ge_c(k.v(i), k.i32, "0"), k.le(k.v(i), k.pv(f, 1))), k.eq(k.deref(k.pn(p)), k.v(i)))
                                   : k.land(k.ge_c(k.v(i), k.i32, "0"), k.le(k.v(i), k.pv(f, 1)));
    const Stmt* upd = write_before ? k.cstore(k.pn(p), BinaryOp::Add, N(k, "1")) : k.store(k.pn(p), k.v(i));
    const Stmt* body = k.blk({upd, k.asg(i, k.add(k.v(i), N(k, "1")))});
    std::vector<const Stmt*> ss = {k.decl(i, N(k, "0"))};
    if (write_before) ss.push_back(k.store(k.pn(p), N(k, "0")));
    ss.push_back(k.whl(k.lt(k.v(i), k.pv(f, 1)), {k.cl(inv)}, {k.cl(k.sub(k.pv(f, 1), k.v(i)))}, body));
    f->body = k.blk(ss);
    k.done(f);
    return f->end_loc;
  };
  lc("init_loop", "a loop over a must_init place with the write before it: the automatic invariants keep the flag, the postcondition holds", [=](Kit& k) { loop_fill(k, true); });
  lc("init_loop_write_in_loop", "the only write is in a loop that may not run: the flag row at the closing brace fails", [=](Kit& k) { k.fail("postcondition", loop_fill(k, false)); });

  // ---- audit round 1 S1: a loop that does not reach a place, and the marked call or return after it ----
  // void leave(int *p [ref_to_uninit]) ... { }
  // int after(int *q, int n) requires(reads(*q) && 0 <= n <= 10) ensures(reads(*q)) { int x; int s; int i = 0; while (i < n) inv ... { s = *q; i += 1; } leave(&x); return [x | i]; }
  // The loop reads *q, so it touches memory, but writes no int cell: x is a place it does not reach (audit round 2, question 89: a loop that touches no memory has no
  // places, and the case would pass with or without the row `held && flag ==> range`).
  const auto after_loop = [=](Kit& k, bool read) -> const Expr* {
    Function* leave = make_leave(k);
    Function* f = k.fn("after", k.i32, {{"q", PT(k)}, {"n", k.i32}});
    k.req(f, k.land(reads(k, f, 0, 0), k.range(k.F(f, 1), k.i32, "0", "10")));
    k.ens(f, reads(k, f, 0, 0));
    auto [x, dx] = cell_bare(k, f, "x");
    Var* i = k.local(f, "i", k.i32);
    Var* s = k.local(f, "s", k.i32);
    const Stmt* body = k.blk({k.asg(s, k.deref(k.pn(f->param_var(0)))), k.asg(i, k.add(k.v(i), N(k, "1")))});
    const Stmt* loop = k.whl(k.lt(k.v(i), k.pv(f, 1)), {k.cl(k.land(k.ge_c(k.v(i), k.i32, "0"), k.le(k.v(i), k.pv(f, 1))))}, {k.cl(k.sub(k.pv(f, 1), k.v(i)))}, body);
    const Expr* rd = k.v(x);
    f->body = k.blkc({dx, k.decl(s, N(k, "0")), k.decl(i, N(k, "0")), loop, k.es(k.call(leave, {k.addr(x)})), k.ret(read ? rd : k.v(i))});
    k.done(f);
    return rd;
  };
  lc("init_loop_unreached_call", "int x; while (i < n) { s = *q; i += 1; } leave(&x): the loop touches memory but does not reach x, and the marked call's range row (flag ==> range) still holds after it", [=](Kit& k) { after_loop(k, false); });
  lc("init_loop_unreached_read", "the same, then return x: the loop and leave give x no value", [=](Kit& k) { k.fail("uninitialized", after_loop(k, true)->loc); });
  // void spin(int *p [ref_to_uninit], int *q, int n) requires(owns(*p) && reads(*q) && 0 <= n <= 10) ensures(owns(*p) && reads(*q)) { int s; int i = 0; while (i < n) { s = *q; i += 1; } }
  lc("init_loop_unreached_callee", "a ref_to_uninit callee whose loop reads *q and never writes *p: the return rows flag ==> range and old(flag) ==> flag hold after it", [=](Kit& k) {
    Function* f = k.fn("spin", k.vd, {{"p", PT(k)}, {"q", PT(k)}, {"n", k.i32}});
    mark(f, 0, InitMarker::RefToUninit);
    k.req(f, k.land(k.land(owns(k, f, 0, 0), reads(k, f, 1, 1)), k.range(k.F(f, 2), k.i32, "0", "10")));
    k.ens(f, k.land(owns(k, f, 0, 0), reads(k, f, 1, 1)));
    Var* i = k.local(f, "i", k.i32);
    Var* s = k.local(f, "s", k.i32);
    const Stmt* body = k.blk({k.asg(s, k.deref(k.pn(f->param_var(1)))), k.asg(i, k.add(k.v(i), N(k, "1")))});
    f->body = k.blk({k.decl(s, N(k, "0")), k.decl(i, N(k, "0")),
                     k.whl(k.lt(k.v(i), k.pv(f, 2)), {k.cl(k.land(k.ge_c(k.v(i), k.i32, "0"), k.le(k.v(i), k.pv(f, 2))))}, {k.cl(k.sub(k.pv(f, 2), k.v(i)))}, body)});
    k.done(f);
  });
  // item 10 (audit N7): void fill(int *p [must_init]) { int i = 0; while (i < 3) { *p = i; i += 1; } }: the loop always runs, but only ds ==> flag is kept
  lc("init_loop_always_runs", "a completeness cost, pinned: the write is only in a loop that always runs, and the loop keeps only ds ==> flag: postcondition at the closing brace", [=](Kit& k) {
    Function* f = k.fn("fill", k.vd, {{"p", PT(k)}});
    mark(f, 0, InitMarker::MustInit);
    k.req(f, owns(k, f, 0, 0));
    k.ens(f, owns(k, f, 0, 0));
    Var* i = k.local(f, "i", k.i32);
    const Stmt* body = k.blk({k.store(k.pn(f->param_var(0)), k.v(i)), k.asg(i, k.add(k.v(i), N(k, "1")))});
    f->body = k.blk({k.decl(i, N(k, "0")),
                     k.whl(k.lt(k.v(i), N(k, "3")), {k.cl(k.land(k.ge_c(k.v(i), k.i32, "0"), k.le_c(k.v(i), k.i32, "3")))}, {k.cl(k.sub(N(k, "3"), k.v(i)))}, body)});
    k.done(f);
    k.fail("postcondition", f->end_loc);
  });

  // ---- audit round 1 S3: the flag row of a global's atom fails where the global is held through a ref_to_uninit parameter ----
  // static int g = 0; void h() requires(owns(g)) ensures(owns(g)) { g = 1; }
  // void f(int *p [ref_to_uninit]) requires(owns(*p) && p == &g) ensures(owns(*p)) { [*p = 1;] h(); }
  const auto via_param = [=](Kit& k, bool write_first) -> const Expr* {
    Global* g = k.prog.add_global("g", k.i32, k.at(), nullptr);
    g->init = N(k, "0");
    g->value = "0";
    auto ga = [&]() { return k.b.global_addr(g, k.at()); };
    Function* h = k.fn("h", k.vd);
    k.req(h, k.b.global_atom(AtomKind::Owns, ga(), 0, k.at(), false));
    k.ens(h, k.b.global_atom(AtomKind::Owns, ga(), 0, k.at(), false));
    h->body = k.blk({k.store(ga(), N(k, "1"))});
    k.done(h);
    Function* f = k.fn("f", k.vd, {{"p", PT(k)}});
    mark(f, 0, InitMarker::RefToUninit);
    k.req(f, k.land(owns(k, f, 0, 0), k.eq(FP(k, f, 0), ga())));
    k.ens(f, owns(k, f, 0, 0));
    const Expr* c = k.call(h, {});
    f->body = k.blk({write_first ? k.store(k.pn(f->param_var(0)), N(k, "1")) : k.es(N(k, "0")), k.es(c)});
    k.done(f);
    return c;
  };
  lc("init_global_row_via_param", "f holds g's cell through p == &g with an unknown flag; after *p = 1 the call of h, which needs owns(g), is fine", [=](Kit& k) { via_param(k, true); });
  lc("init_global_row_via_param_uninit", "without the write the flag row of h's atom on g, category `uninitialized`, fails at the call", [=](Kit& k) {
    k.fail("uninitialized", via_param(k, false)->loc);
  });

  // ---- the backstop of check_program (items 4 and 6): a marker the encoder would have rejected ----
  // `shape`: 0 a non-pointer, 1 no owns in the requires, 2 must_init without owns in the ensures, 3 reads(*p) beside the marker, 4 must_init on a
  // noreturn function, 5 *p in a requires, 6 *p in a decreases, 7 old(*p) in an ensures, 8 *p in the ensures of a ref_to_uninit-only parameter.
  // The accepted twins are lowering cases above: init_must_init has *p == 0 in its must_init ensures, and every marked callee owns *p in both lists.
  const auto marked = [=](Kit& k, int shape) {
    const bool ref_only = shape == 1 || shape == 3 || shape == 5 || shape == 6 || shape == 8;
    const InitMarker m = ref_only ? InitMarker::RefToUninit : InitMarker::MustInit;
    Function* f = k.fn("f", k.vd, {{"p", shape == 0 ? k.i32 : PT(k)}});
    mark(f, 0, m);
    if (shape == 4) f->noreturn = true;
    if (shape == 0) {
      k.req(f, k.yes());
    } else if (shape == 1) {
      k.req(f, k.yes());
    } else if (shape == 3) {
      k.req(f, k.land(owns(k, f, 0, 0), reads(k, f, 0, 1)));
    } else {
      k.req(f, owns(k, f, 0, 0));
    }
    if (shape == 5) k.req(f, k.gt(star(k, f, 0), N(k, "0")));
    if (shape == 6) k.dec(f, star(k, f, 0));
    if (shape == 2) {
      k.ens(f, k.yes());
    } else if (shape == 7) {
      k.ens(f, k.land(owns(k, f, 0, 0), k.eq(star(k, f, 0), old_star(k, f, 0))));
    } else if (shape == 8) {
      k.ens(f, k.land(owns(k, f, 0, 0), k.eq_c(star(k, f, 0), k.i32, "0")));
    } else if (shape != 4) {
      k.ens(f, owns(k, f, 0, 0));
    }
    k.trusted(f);
  };
  rc("init_backstop_non_pointer", "is not a pointer", [=](Kit& k) { marked(k, 0); });
  rc("init_backstop_no_owns_requires", "without `owns(*p)` in its `requires`", [=](Kit& k) { marked(k, 1); });
  rc("init_backstop_must_init_no_owns_ensures", "is `must_init` without `owns(*p)` in its `ensures`", [=](Kit& k) { marked(k, 2); });
  rc("init_backstop_reads_beside", "beside `reads(*p)`", [=](Kit& k) { marked(k, 3); });
  rc("init_backstop_must_init_noreturn", "`must_init` on a `noreturn` function", [=](Kit& k) { marked(k, 4); });
  rc("init_backstop_read_requires", "a contract reads `*p`", [=](Kit& k) { marked(k, 5); });
  rc("init_backstop_read_decreases", "a contract reads `*p`", [=](Kit& k) { marked(k, 6); });
  rc("init_backstop_read_old", "an `ensures` reads `*p` where its flag is not promised", [=](Kit& k) { marked(k, 7); });
  rc("init_backstop_read_ref_ensures", "an `ensures` reads `*p` where its flag is not promised", [=](Kit& k) { marked(k, 8); });
}

}  // namespace
