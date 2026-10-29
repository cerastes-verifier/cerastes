// (included by selftest.cpp) Pointer parameters, results and contracts (decision 0037, M4.5b1b): the permission atoms `owns(*p)`, `reads(*p)` and
// `live(p)`, `old(*p)`, the ghost amounts and the case splits, the explicit checks at calls and returns, the framing of contract reads (item 7),
// `&param`, pointer results, `pointer-comparison` and the rejections of items 2, 5 and 15.
// PRE-REGISTERED from the record (item numbers below): each expectation names the category and the VIR node whose position the record gives (the
// call, the `return` or closing brace, the contract clause, the comparison), so the cases test the position map as well.  Each rule has a
// case that verifies and a twin that fails for one reason (invariant 2).
namespace {

void define_pointer_contracts() {
  auto PT = [](Kit& k) { return k.ptr(k.i32); };
  auto N = [](Kit& k, const char* v) { return k.num(k.i32, v); };
  // `p`'s formal with its spelling for messages, and its local copy.
  auto FP = [](Kit& k, Function* f, int i) { return k.b.text(k.F(f, i), f->params[static_cast<size_t>(i)].name); };
  auto owns = [=](Kit& k, Function* f, int i, int amount) { return k.b.atom(AtomKind::Owns, FP(k, f, i), amount, k.at()); };
  auto reads = [=](Kit& k, Function* f, int i, int amount) { return k.b.atom(AtomKind::Reads, FP(k, f, i), amount, k.at()); };
  auto live = [=](Kit& k, Function* f, int i, int amount) { return k.b.atom(AtomKind::Live, FP(k, f, i), amount, k.at()); };
  auto star = [=](Kit& k, Function* f, int i) { return k.deref(FP(k, f, i)); };  // `*p` in a contract
  auto old_star = [=](Kit& k, Function* f, int i) { return k.old(star(k, f, i)); };
  // Mark a requires/ensures clause and return its position (the clause's `loc`).
  auto reqc = [](Kit& k, Function* f, const Expr* e) {
    Clause c = k.cl(e);
    f->requires_.push_back(c);
    return c.loc;
  };
  auto ensc = [](Kit& k, Function* f, const Expr* e) {
    Clause c = k.cl(e);
    f->ensures_.push_back(c);
    return c.loc;
  };
  // A cell `x` of a caller, written with `v`, and the call of `callee` on `&x` (void callees).
  auto cell_init = [=](Kit& k, Function* f, const char* name, const char* v) {
    Var* x = k.cell(f, name, k.i32);
    return std::make_pair(x, k.decl(x, N(k, v)));
  };

  // ---- owns(*p) and old(*p) across a call (items 5, 6, 8, 10) ----
  // void inc(int *p) requires(owns(*p) && *p < 100) ensures(owns(*p) && *p == old(*p) + delta) { *p = *p + 1; }
  const auto make_inc = [=](Kit& k, const char* delta) -> Function* {
    Function* f = k.fn("inc", k.vd, {{"p", PT(k)}});
    k.req(f, k.land(owns(k, f, 0, 0), k.lt(star(k, f, 0), N(k, "100"))));
    k.ens(f, k.land(owns(k, f, 0, 0), k.eq(star(k, f, 0), k.add(old_star(k, f, 0), N(k, delta)))));
    const Var* p = f->param_var(0);
    f->body = k.blk({k.store(k.pn(p), k.add(k.deref(k.pn(p)), N(k, "1")))});
    k.done(f);
    return f;
  };
  lc("ptr_owns_inc", "inc(p) with owns(*p) and old(*p): the callee verifies against its contract", [=](Kit& k) { make_inc(k, "1"); });
  lc("ptr_owns_inc_wrong", "the same promising +2: the value ensures fails at the closing brace", [=](Kit& k) {
    Function* f = make_inc(k, "2");
    k.fail("postcondition", f->end_loc);
  });
  // void use() { int x = 5; inc(&x); check(x == 6); }
  const auto use_inc = [=](Kit& k, bool wrong) -> SrcLoc {
    Function* inc = make_inc(k, "1");
    Function* f = k.fn("use", k.vd);
    auto [x, dx] = cell_init(k, f, "x", "5");
    const Stmt* c = k.chk(k.eq_c(k.v(x), k.i32, wrong ? "7" : "6"));
    f->body = k.blkc({dx, k.es(k.call(inc, {k.addr(x)})), c});
    k.done(f);
    return c->loc;
  };
  lc("ptr_owns_inc_caller", "int x = 5; inc(&x); check(x == 6): the callee's ensures come back, the scope exit finds the whole cell", [=](Kit& k) { use_inc(k, false); });
  lc("ptr_owns_inc_caller_wrong", "check(x == 7) does not hold", [=](Kit& k) { k.fail("check", use_inc(k, true)); });
  // a caller that only reads (reads(*p)) passes the pointer to inc: the call needs the whole cell (item 8)
  lc("ptr_owns_inc_from_reader", "void reader(int *p) requires(reads(*p) && *p < 100) { inc(p); }: the call needs to own *p", [=](Kit& k) {
    Function* inc = make_inc(k, "1");
    Function* f = k.fn("reader", k.vd, {{"p", PT(k)}});
    k.req(f, k.land(reads(k, f, 0, 0), k.lt(star(k, f, 0), N(k, "100"))));
    k.ens(f, reads(k, f, 0, 0));
    const Expr* c = k.call(inc, {k.pn(f->param_var(0))});
    f->body = k.blk({k.es(c)});
    k.done(f);
    k.fail_only("permission", c->loc, 11, 3);  // of the 11 rows of the call, the fourth: perm(p.v_int) == write
  });

  // ---- swap: two owns atoms, the separation of the two places (items 8, 9) ----
  const auto make_swap = [=](Kit& k, bool wrong) -> Function* {
    Function* f = k.fn("swap", k.vd, {{"a", PT(k)}, {"b", PT(k)}});
    k.req(f, k.land(owns(k, f, 0, 0), owns(k, f, 1, 1)));
    k.ens(f, k.land(k.land(owns(k, f, 0, 0), owns(k, f, 1, 1)),
                    k.land(k.eq(star(k, f, 0), old_star(k, f, 1)), k.eq(star(k, f, 1), old_star(k, f, 0)))));
    Var* t = k.local(f, "t", k.i32);
    const Var* a = f->param_var(0);
    const Var* b = f->param_var(1);
    f->body = k.blk({k.decl(t, k.deref(k.pn(a))), k.store(k.pn(a), k.deref(k.pn(b))), k.store(k.pn(b), wrong ? k.deref(k.pn(a)) : k.v(t))});
    k.done(f);
    return f;
  };
  lc("ptr_swap", "swap(a, b) with owns(*a) && owns(*b): two atoms, two amounts", [=](Kit& k) { make_swap(k, false); });
  lc("ptr_swap_wrong", "the last store is *a, so *b == old(*a) fails at the closing brace", [=](Kit& k) {
    Function* f = make_swap(k, true);
    k.fail("postcondition", f->end_loc);
  });
  const auto swap_caller = [=](Kit& k, bool alias) -> const Expr* {
    Function* swap = make_swap(k, false);
    Function* f = k.fn("use", k.vd);
    auto [x, dx] = cell_init(k, f, "x", "1");
    auto [y, dy] = cell_init(k, f, "y", "2");
    const Expr* c = k.call(swap, {k.addr(x), k.addr(alias ? x : y)});
    const Stmt* ck = alias ? nullptr : k.chk(k.land(k.eq_c(k.v(x), k.i32, "2"), k.eq_c(k.v(y), k.i32, "1")));
    std::vector<const Stmt*> body = {dx, dy, k.es(c)};
    if (ck != nullptr) body.push_back(ck);
    f->body = k.blkc(body);
    k.done(f);
    return c;
  };
  lc("ptr_swap_caller", "int x = 1, y = 2; swap(&x, &y); check(x == 2 && y == 1)", [=](Kit& k) { swap_caller(k, false); });
  lc("ptr_swap_caller_alias", "swap(&x, &x): the two full permissions of one cell: the separation check fails at the call", [=](Kit& k) {
    const Expr* c = swap_caller(k, true);
    k.fail("permission", c->loc, 1, "", "twice");
  });

  // ---- reads(*p): fractions, two reads on one place, a function that only reads (items 6, 8, 10) ----
  // int peek(const int *p) requires(reads(*p)) ensures(reads(*p) && r == *p) { return *p; }
  const auto make_peek = [=](Kit& k, int mode) -> Function* {  // mode 0: reads, 1: writes *p, 2: returns less
    Function* f = k.fn(mode == 0 ? "peek" : "bad", k.i32, {{"p", PT(k)}});
    k.req(f, reads(k, f, 0, 0));
    k.ens(f, k.land(reads(k, f, 0, 0), k.eq(k.R(f), star(k, f, 0))));
    const Var* p = f->param_var(0);
    f->body = k.blk({mode == 1 ? k.store(k.pn(p), N(k, "1")) : k.es(N(k, "0")), k.ret(k.deref(k.pn(p)))});
    k.done(f);
    return f;
  };
  lc("ptr_reads_peek", "peek(p) with reads(*p) returns *p", [=](Kit& k) { make_peek(k, 0); });
  lc("ptr_reads_writes", "a function with reads(*p) that writes *p: the write's permission check fails", [=](Kit& k) {
    Function* f = make_peek(k, 1);
    k.fail("permission", f->body->stmts[0]->loc);
  });
  // int use() { int x = 7; int a = peek(&x); int b = peek(&x); x = a + b; return x; } ensures r == 14: the whole cell is back between the calls
  lc("ptr_reads_twice_then_write", "peek(&x) twice, then x = a + b: the fractions are given back, the write finds the whole cell", [=](Kit& k) {
    Function* peek = make_peek(k, 0);
    Function* f = k.fn("use", k.i32);
    k.ens(f, k.eq_c(k.R(f), k.i32, "14"));
    auto [x, dx] = cell_init(k, f, "x", "7");
    Var* a = k.local(f, "a", k.i32);
    Var* b = k.local(f, "b", k.i32);
    f->body = k.blkc({dx, k.decl(a, k.call(peek, {k.addr(x)})), k.decl(b, k.call(peek, {k.addr(x)})), k.asg(x, k.add(k.v(a), k.v(b))), k.ret(k.v(x))});
    k.done(f);
  });
  // int sum(const int *p, const int *q) requires(reads(*p) && reads(*q) && bounds) ensures(reads(*p) && reads(*q) && r == *p + *q); sum(&x, &x)
  const auto make_sum = [=](Kit& k) -> Function* {
    Function* f = k.fn("sum", k.i32, {{"p", PT(k)}, {"q", PT(k)}});
    k.req(f, k.land(k.land(reads(k, f, 0, 0), reads(k, f, 1, 1)),
                    k.land(k.range(star(k, f, 0), k.i32, "0", "1000"), k.range(star(k, f, 1), k.i32, "0", "1000"))));
    k.ens(f, k.land(k.land(reads(k, f, 0, 0), reads(k, f, 1, 1)), k.eq(k.R(f), k.add(old_star(k, f, 0), old_star(k, f, 1)))));
    f->body = k.blk({k.ret(k.add(k.deref(k.pn(f->param_var(0))), k.deref(k.pn(f->param_var(1)))))});
    k.done(f);
    return f;
  };
  lc("ptr_reads_two_places", "sum(p, q) with two reads atoms: the sums of the amounts are checked when p == q", [=](Kit& k) { make_sum(k); });
  lc("ptr_reads_same_place_twice", "sum(&x, &x): one cell in two reads atoms, per-atom amounts, then the write after", [=](Kit& k) {
    Function* sum = make_sum(k);
    Function* f = k.fn("use", k.i32);
    k.ens(f, k.eq_c(k.R(f), k.i32, "14"));
    auto [x, dx] = cell_init(k, f, "x", "7");
    f->body = k.blkc({dx, k.ret(k.call(sum, {k.addr(x), k.addr(x)}))});
    k.done(f);
  });
  // a function with reads(*p) that hands its fraction to a consumer (a callee with a reads atom and no ensures): it returns less than it was lent
  lc("ptr_reads_returns_less", "void hold(const int *p) requires(reads(*p)) ensures(reads(*p)) { consume(p); }: the return check fails", [=](Kit& k) {
    Function* consume = k.fn("consume", k.vd, {{"p", PT(k)}});
    k.req(consume, reads(k, consume, 0, 0));
    k.trusted(consume);
    Function* f = k.fn("hold", k.vd, {{"p", PT(k)}});
    k.req(f, reads(k, f, 0, 0));
    k.ens(f, reads(k, f, 0, 0));
    f->body = k.blk({k.es(k.call(consume, {k.pn(f->param_var(0))}))});
    k.done(f);
    k.fail_only("permission", f->end_loc, 6, 1);  // of the 6 rows of the return check, the cell's value fraction: perm(p.v_int) >= amt
  });

  // ---- p == NULL || owns(*p): a guarded atom (items 5, 6) ----
  const auto make_get = [=](Kit& k, bool guarded) -> Function* {
    Function* f = k.fn("get", k.i32, {{"p", PT(k)}});
    const auto nul = [&]() { return k.eq(FP(k, f, 0), k.nul(PT(k))); };
    k.req(f, k.lor(nul(), owns(k, f, 0, 0)));
    k.ens(f, k.land(k.lor(nul(), owns(k, f, 0, 0)),
                    k.cond(nul(), k.eq_c(k.R(f), k.i32, "0"), k.land(k.eq(k.R(f), star(k, f, 0)), k.eq(star(k, f, 0), old_star(k, f, 0))))));
    const Var* p = f->param_var(0);
    if (guarded)
      f->body = k.blk({k.iff(k.eq(k.pn(p), k.nul(PT(k))), k.blk({k.ret(N(k, "0"))})), k.ret(k.deref(k.pn(p)))});
    else
      f->body = k.blk({k.ret(k.deref(k.pn(p)))});
    k.done(f);
    return f;
  };
  lc("ptr_null_or_owns", "int get(int *p) requires(p == NULL || owns(*p)): the body tests p first", [=](Kit& k) { make_get(k, true); });
  lc("ptr_null_or_owns_unguarded", "the body dereferences p without the test: null-dereference at the read", [=](Kit& k) {
    Function* f = make_get(k, false);
    k.fail("null-dereference", f->body->stmts[0]->expr->loc);  // the Return's operand `*p`
  });
  // callers with a guarded atom: &x and NULL
  lc("ptr_null_or_owns_callers", "get(&x) and get(NULL): the amount of a false guard is write / (N + 1), the call with a cell gives the cell", [=](Kit& k) {
    Function* get = make_get(k, true);
    Function* f = k.fn("use", k.i32);
    k.ens(f, k.eq_c(k.R(f), k.i32, "5"));
    auto [x, dx] = cell_init(k, f, "x", "5");
    Var* a = k.local(f, "a", k.i32);
    Var* b = k.local(f, "b", k.i32);
    f->body = k.blkc({dx, k.decl(a, k.call(get, {k.addr(x)})), k.decl(b, k.call(get, {k.nul(PT(k))})), k.ret(k.add(k.v(a), k.v(b)))});
    k.done(f);
  });

  // ---- a callee that keeps the cell (item 13) ----
  const auto make_dispose = [=](Kit& k) -> Function* {
    Function* f = k.fn("dispose", k.vd, {{"p", PT(k)}});
    k.req(f, owns(k, f, 0, 0));
    f->body = k.blk({});
    k.done(f);
    return f;
  };
  lc("ptr_consume_param", "void pass(int *p) requires(owns(*p)) { dispose(p); }: the caller owned it, the cell is the callee's now", [=](Kit& k) {
    Function* dispose = make_dispose(k);
    Function* f = k.fn("pass", k.vd, {{"p", PT(k)}});
    k.req(f, owns(k, f, 0, 0));
    f->body = k.blk({k.es(k.call(dispose, {k.pn(f->param_var(0))}))});
    k.done(f);
  });
  lc("ptr_consume_local_scope_exit", "int x = 1; dispose(&x): the callee kept the cell, so the scope exit of x fails (lifetime at the closing brace)", [=](Kit& k) {
    Function* dispose = make_dispose(k);
    Function* f = k.fn("leak", k.vd);
    auto [x, dx] = cell_init(k, f, "x", "1");
    f->body = k.blkc({dx, k.es(k.call(dispose, {k.addr(x)}))});
    k.done(f);
    k.fail("lifetime", f->body->end_loc);
  });
  lc("ptr_consume_local_read", "int x = 1; dispose(&x); return x: the read of the cell fails permission", [=](Kit& k) {
    Function* dispose = make_dispose(k);
    Function* f = k.fn("use", k.i32);
    auto [x, dx] = cell_init(k, f, "x", "1");
    const Expr* r = k.v(x);
    f->body = k.blkc({dx, k.es(k.call(dispose, {k.addr(x)})), k.ret(r)});
    k.done(f);
    k.fail("permission", r->loc);
  });

  // ---- pointer-comparison (item 12) ----
  const auto make_same = [=](Kit& k, bool use_live) -> const Expr* {
    Function* f = k.fn("same", k.i32, {{"p", PT(k)}, {"q", PT(k)}});
    const auto both = [&]() {
      return use_live ? k.land(live(k, f, 0, 0), live(k, f, 1, 1)) : k.land(owns(k, f, 0, 0), owns(k, f, 1, 1));
    };
    k.req(f, both());
    k.ens(f, both());
    const Expr* cmp = k.eq(k.pn(f->param_var(0)), k.pn(f->param_var(1)));
    f->body = k.blk({k.ret(k.bi(k.i32, cmp))});
    k.done(f);
    return cmp;
  };
  lc("ptr_compare_owned", "int same(p, q) requires(owns(*p) && owns(*q)) { return p == q; }: both exclude one past the end", [=](Kit& k) { make_same(k, false); });
  lc("ptr_compare_live", "the same with live(p) && live(q): a pointer one past its object may equal a pointer to another: pointer-comparison", [=](Kit& k) {
    k.fail("pointer-comparison", make_same(k, true)->loc);
  });
  lc("ptr_compare_null_is_free", "int is_null(p) requires(p == NULL || live(p)) { return p == NULL; }: a comparison with NULL is never checked", [=](Kit& k) {
    Function* f = k.fn("is_null", k.i32, {{"p", PT(k)}});
    k.req(f, k.lor(k.eq(FP(k, f, 0), k.nul(PT(k))), live(k, f, 0, 0)));
    k.ens(f, k.lor(k.eq(FP(k, f, 0), k.nul(PT(k))), live(k, f, 0, 0)));
    f->body = k.blk({k.ret(k.bi(k.i32, k.eq(k.pn(f->param_var(0)), k.nul(PT(k)))))});
    k.done(f);
    k.rows("pointer-comparison", f->body->stmts[0]->loc, 0);
  });
  lc("ptr_compare_in_contract", "a contract's p == q is model identity: it needs no atoms and is not checked as a comparison of objects", [=](Kit& k) {
    Function* f = k.fn("f", k.vd, {{"p", PT(k)}, {"q", PT(k)}});
    k.req(f, k.land(live(k, f, 0, 0), live(k, f, 1, 1)));
    k.ens(f, k.land(k.land(live(k, f, 0, 0), live(k, f, 1, 1)), k.eq(FP(k, f, 0), FP(k, f, 0))));
    f->body = k.blk({});
    k.done(f);
  });

  // ---- a pointer result (item 3) ----
  const auto make_pick = [=](Kit& k, bool wrong) -> Function* {
    Function* f = k.fn("pick", PT(k), {{"c", k.i32}, {"p", PT(k)}, {"q", PT(k)}});
    k.req(f, k.land(owns(k, f, 1, 0), owns(k, f, 2, 1)));
    const Expr* pq = wrong ? k.eq(k.R(f), FP(k, f, 1)) : k.lor(k.eq(k.R(f), FP(k, f, 1)), k.eq(k.R(f), FP(k, f, 2)));
    k.ens(f, k.land(pq, k.land(owns(k, f, 1, 0), owns(k, f, 2, 1))));
    f->body = k.blk({k.ret(k.cond(k.ne(k.pv(f, 0), N(k, "0")), k.pn(f->param_var(1)), k.pn(f->param_var(2))))});
    k.done(f);
    return f;
  };
  lc("ptr_result_pick", "int *pick(c, p, q) requires(owns(*p) && owns(*q)) ensures(r == p || r == q && owns...): a result related to its parameters", [=](Kit& k) { make_pick(k, false); });
  lc("ptr_result_pick_wrong", "ensures r == p only: the result may be q, the value ensures fails at the return", [=](Kit& k) {
    Function* f = make_pick(k, true);
    k.fail("postcondition", f->body->stmts[0]->loc);
  });
  lc("ptr_result_pick_caller", "int *t = pick(1, &x, &y); *t = 7; return *t: the write through the result is legal on both cells (a case split)", [=](Kit& k) {
    Function* pick = make_pick(k, false);
    Function* f = k.fn("use", k.i32);
    k.ens(f, k.eq_c(k.R(f), k.i32, "7"));
    auto [x, dx] = cell_init(k, f, "x", "1");
    auto [y, dy] = cell_init(k, f, "y", "2");
    Var* t = k.local(f, "t", PT(k));
    f->body = k.blkc({dx, dy, k.decl(t, k.call(pick, {N(k, "1"), k.addr(x), k.addr(y)})), k.store(k.pn(t), N(k, "7")), k.ret(k.deref(k.pn(t)))});
    k.done(f);
  });
  // int *dangle() { int x = 1; int *p = &x; return p; }  and the caller: int *t = dangle(); return t != NULL;
  lc("ptr_result_dangling", "dangle() returns a pointer to its own local: passes at the return (a use while x lives), fails at the caller's first use", [=](Kit& k) {
    Function* d = k.fn("dangle", PT(k));
    auto [x, dx] = cell_init(k, d, "x", "1");
    Var* p = k.local(d, "p", PT(k));
    d->body = k.blkc({dx, k.decl(p, k.addr(x)), k.ret(k.pn(p))});
    k.done(d);
    Function* f = k.fn("use", k.i32);
    Var* t = k.local(f, "t", PT(k));
    const Expr* use = k.call(d, {});
    f->body = k.blk({k.decl(t, use), k.ret(k.bi(k.i32, k.ne(k.pn(t), k.nul(PT(k)))))});
    k.done(f);
    k.fail("lifetime", use->loc);
  });
  // int *id(int *p) requires(live(p)) ensures(r == p && live(p)) { return p; } and the twin without live(p)
  const auto make_id = [=](Kit& k, bool with_live) -> const Stmt* {
    Function* f = k.fn("id", PT(k), {{"p", PT(k)}});
    if (with_live) k.req(f, live(k, f, 0, 0));
    k.ens(f, with_live ? k.land(k.eq(k.R(f), FP(k, f, 0)), live(k, f, 0, 0)) : k.eq(k.R(f), FP(k, f, 0)));
    const Stmt* r = k.ret(k.pn(f->param_var(0)));
    f->body = k.blk({r});
    k.done(f);
    return r;
  };
  lc("ptr_result_id", "returning a parameter is a use: with live(p) in the requires it passes", [=](Kit& k) { make_id(k, true); });
  lc("ptr_result_id_nolive", "without live(p) the use at the return fails lifetime", [=](Kit& k) { k.fail("lifetime", make_id(k, false)->expr->loc); });

  // ---- &param (item 4) ----
  const auto addr_param = [=](Kit& k, bool wrong) -> SrcLoc {
    Function* f = k.fn("f", k.i32, {{"a", k.i32}});
    k.ens(f, k.eq_c(k.R(f), k.i32, "1"));
    Var* q = k.local(f, "q", PT(k));
    f->mark_address_taken(f->param_var(0));
    const Stmt* c = k.chk(k.eq_c(k.pv(f, 0), k.i32, wrong ? "2" : "1"));
    f->body = k.blkc({k.decl(q, k.addr(f->param_var(0))), k.store(k.pn(q), N(k, "1")), c, k.ret(k.pv(f, 0))});
    k.done(f);
    return c->loc;
  };
  lc("ptr_address_of_param", "int *q = &a; *q = 1; check(a == 1); return a: the parameter is copied into a block at entry", [=](Kit& k) { addr_param(k, false); });
  lc("ptr_address_of_param_wrong", "check(a == 2) after the write through q", [=](Kit& k) { k.fail("check", addr_param(k, true)); });
  lc("ptr_address_of_param_contract_reads_entry", "the contract keeps reading the entry value of a: r == a for int g(int a) { int *q = &a; return *q; }", [=](Kit& k) {
    Function* f = k.fn("g", k.i32, {{"a", k.i32}});
    k.ens(f, k.eq(k.R(f), k.F(f, 0)));
    Var* q = k.local(f, "q", PT(k));
    f->mark_address_taken(f->param_var(0));
    f->body = k.blkc({k.decl(q, k.addr(f->param_var(0))), k.ret(k.deref(k.pn(q)))});
    k.done(f);
  });
  lc("ptr_address_of_param_consumed", "dispose(&a): the callee keeps the parameter's block, so the function's scope exit fails lifetime", [=](Kit& k) {
    Function* dispose = make_dispose(k);
    Function* f = k.fn("give_away", k.vd, {{"a", k.i32}});
    f->mark_address_taken(f->param_var(0));
    f->body = k.blkc({k.es(k.call(dispose, {k.addr(f->param_var(0))}))});
    k.done(f);
    k.fail("lifetime", f->end_loc);  // a parameter's block ends with the function: its closing brace
  });

  // ---- the framing of contract reads (item 7) ----
  // int f(int *p) requires(p == NULL || owns(*p)) requires(p == NULL || *p > 0) { return 0; }, and the unframed forms
  enum class Frame { Guarded, Unframed, GuardMismatch };
  const auto framed = [=](Kit& k, Frame shape) -> SrcLoc {
    Function* f = k.fn("f", k.i32, {{"c", k.i32}, {"p", PT(k)}});
    k.req(f, k.range(k.F(f, 0), k.i32, "-100", "100"));
    SrcLoc where;
    const auto isnull = [&]() { return k.eq(FP(k, f, 1), k.nul(PT(k))); };
    switch (shape) {
      case Frame::Guarded:
        reqc(k, f, k.lor(isnull(), owns(k, f, 1, 0)));
        reqc(k, f, k.lor(isnull(), k.gt(star(k, f, 1), N(k, "0"))));
        break;
      case Frame::Unframed: {
        const Expr* d = star(k, f, 1);
        reqc(k, f, k.gt(d, N(k, "0")));
        where = d->loc;
        break;
      }
      case Frame::GuardMismatch: {  // the atom is on c > 0, the read on c >= 0
        reqc(k, f, k.cond(k.gt(k.F(f, 0), N(k, "0")), owns(k, f, 1, 0), k.yes()));
        const Expr* d = star(k, f, 1);
        reqc(k, f, k.cond(k.ge(k.F(f, 0), N(k, "0")), k.gt(d, N(k, "0")), k.yes()));
        where = d->loc;
        break;
      }
    }
    f->body = k.blk({k.ret(N(k, "0"))});
    k.done(f);
    return where;
  };
  lc("ptr_framed_guarded", "p == NULL || owns(*p) frames p == NULL || *p > 0: both on the same guard", [=](Kit& k) { framed(k, Frame::Guarded); });
  lc("ptr_framed_unframed", "requires(*p > 0) with no atom: the contract reads *p without owning it (permission at the clause)", [=](Kit& k) {
    k.fail("permission", framed(k, Frame::Unframed));
  });
  lc("ptr_framed_guard_mismatch", "atom on c > 0, read on c >= 0: the framing fails on the path c == 0", [=](Kit& k) {
    k.fail("permission", framed(k, Frame::GuardMismatch));
  });
  // audit B1: ensures(p != NULL ? *p == 1 : r == 0), the body returns 5 for NULL; audit B2: ensures(*q == 5) framed only by owns(*p)
  lc("ptr_framed_b1", "ensures(p != NULL ? *p == 1 : r == 0) with p == NULL || owns(*p): returning 5 for NULL breaks the postcondition", [=](Kit& k) {
    Function* f = k.fn("b1", k.i32, {{"p", PT(k)}});
    const auto isnull = [&]() { return k.eq(FP(k, f, 0), k.nul(PT(k))); };
    k.req(f, k.lor(isnull(), owns(k, f, 0, 0)));
    k.ens(f, k.lor(isnull(), owns(k, f, 0, 0)));
    k.ens(f, k.cond(k.ne(FP(k, f, 0), k.nul(PT(k))), k.eq_c(star(k, f, 0), k.i32, "1"), k.eq_c(k.R(f), k.i32, "0")));
    const Var* p = f->param_var(0);
    const Stmt* bad = k.ret(N(k, "5"));
    f->body = k.blk({k.iff(k.eq(k.pn(p), k.nul(PT(k))), k.blk({bad})), k.store(k.pn(p), N(k, "1")), k.ret(N(k, "0"))});
    k.done(f);
    k.fail("postcondition", bad->loc);
  });
  lc("ptr_framed_b2", "void b2(p, q) requires(owns(*p)) ensures(owns(*p) && *q == 5): *q is framed only by owns(*p)", [=](Kit& k) {
    Function* f = k.fn("b2", k.vd, {{"p", PT(k)}, {"q", PT(k)}});
    k.req(f, owns(k, f, 0, 0));
    k.ens(f, owns(k, f, 0, 0));
    const Expr* d = star(k, f, 1);
    ensc(k, f, k.eq_c(d, k.i32, "5"));
    const SrcLoc where = d->loc;
    k.trusted(f);
    k.fail("permission", where);
  });

  // ---- restrict (item 2): accepted with owns, backstop in check_program ----
  lc("ptr_restrict_owns", "int g(int *restrict p) requires(p == NULL || owns(*p)) ensures(p == NULL || owns(*p)): accepted", [=](Kit& k) {
    Function* f = k.fn("g", k.i32, {{"p", PT(k)}});
    f->params[0].is_restrict = true;
    const auto isnull = [&]() { return k.eq(FP(k, f, 0), k.nul(PT(k))); };
    k.req(f, k.lor(isnull(), owns(k, f, 0, 0)));
    k.ens(f, k.lor(isnull(), owns(k, f, 0, 0)));
    const Var* p = f->param_var(0);
    f->body = k.blk({k.iff(k.eq(k.pn(p), k.nul(PT(k))), k.blk({k.ret(N(k, "0"))})), k.ret(k.deref(k.pn(p)))});
    k.done(f);
  });
}

void define_pointer_contract_rejections() {
  auto PT = [](Kit& k) { return k.ptr(k.i32); };
  auto FP = [](Kit& k, Function* f, int i) { return k.b.text(k.F(f, i), f->params[static_cast<size_t>(i)].name); };

  // ---- the four rejections of b1a that b1b lifts, as lowering cases (the flips of the plan) ----
  lc("ptr_param_without_contract", "void f(int *p) {}: a pointer parameter is accepted (it only gets WF(p))", [=](Kit& k) {
    Function* f = k.fn("f", k.vd, {{"p", PT(k)}});
    f->body = k.blk({});
    k.done(f);
  });
  lc("ptr_result_null", "int *f(void) { return NULL; }: a pointer result is accepted (it gets ensures WF(result))", [=](Kit& k) {
    Function* f = k.fn("f", PT(k));
    f->body = k.blk({k.ret(k.nul(PT(k)))});
    k.done(f);
  });
  lc("ptr_comparison_in_contract", "ensures(p == p) over a pointer parameter: a pointer in a contract is accepted, == is model identity", [=](Kit& k) {
    Function* f = k.fn("f", k.vd, {{"p", PT(k)}});
    k.ens(f, k.eq(FP(k, f, 0), FP(k, f, 0)));
    f->body = k.blk({});
    k.done(f);
  });
  lc("ptr_address_of_param_unused", "void f(int a) { int *q = &a; }: the address of a parameter is accepted", [=](Kit& k) {
    Function* f = k.fn("f", k.vd, {{"a", k.i32}});
    Var* q = k.local(f, "q", PT(k));
    f->mark_address_taken(f->param_var(0));
    f->body = k.blkc({k.decl(q, k.addr(f->param_var(0)))});
    k.done(f);
  });

  // ---- check_program's backstops for item 5 (the encoder reports them first) ----
  rc("ptr_rej_atom_under_not", "a permission atom in a position", [=](Kit& k) {
    Function* f = k.fn("f", k.vd, {{"p", PT(k)}});
    k.req(f, k.lnot(k.b.atom(AtomKind::Owns, FP(k, f, 0), 0, k.at())));
    f->body = k.blk({});
    k.done(f);
  });
  rc("ptr_rej_atom_left_of_or", "a permission atom in a position", [=](Kit& k) {
    Function* f = k.fn("f", k.vd, {{"p", PT(k)}});
    k.req(f, k.lor(k.b.atom(AtomKind::Owns, FP(k, f, 0), 0, k.at()), k.eq(FP(k, f, 0), k.nul(PT(k)))));
    f->body = k.blk({});
    k.done(f);
  });
  rc("ptr_rej_atom_of_non_parameter", "a permission atom of something other", [=](Kit& k) {
    Function* f = k.fn("f", k.vd, {{"p", PT(k)}});
    k.req(f, k.b.atom(AtomKind::Owns, k.deref(FP(k, f, 0)), 0, k.at()));
    f->body = k.blk({});
    k.done(f);
  });
  rc("ptr_rej_restrict_without_owns", "restrict", [=](Kit& k) {
    Function* f = k.fn("f", k.vd, {{"p", PT(k)}});
    f->params[0].is_restrict = true;
    k.req(f, k.b.atom(AtomKind::Reads, FP(k, f, 0), 0, k.at()));
    k.ens(f, k.b.atom(AtomKind::Reads, FP(k, f, 0), 0, k.at()));
    f->body = k.blk({});
    k.done(f);
  });
  rc("ptr_rej_restrict_other_parameter_without_atom", "restrict", [=](Kit& k) {
    Function* f = k.fn("f", k.vd, {{"p", PT(k)}, {"q", PT(k)}});
    f->params[0].is_restrict = true;
    k.req(f, k.b.atom(AtomKind::Owns, FP(k, f, 0), 0, k.at()));
    k.ens(f, k.b.atom(AtomKind::Owns, FP(k, f, 0), 0, k.at()));
    f->body = k.blk({});
    k.done(f);
  });
  rc("ptr_rej_ensures_atom_without_partner", "an `ensures` atom without", [=](Kit& k) {
    Function* f = k.fn("f", k.vd, {{"p", PT(k)}});
    k.ens(f, k.b.atom(AtomKind::Owns, FP(k, f, 0), 0, k.at()));
    f->body = k.blk({});
    k.done(f);
  });
  rc("ptr_rej_atom_in_body", "a permission atom outside a contract", [=](Kit& k) {
    Function* f = k.fn("f", k.vd, {{"p", PT(k)}});
    k.req(f, k.b.atom(AtomKind::Live, FP(k, f, 0), 0, k.at()));
    f->body = k.blk({k.chk(k.b.atom(AtomKind::Live, k.pn(f->param_var(0)), 0, k.at()))});
    k.done(f);
  });
}

}  // namespace
