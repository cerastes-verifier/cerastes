// (included by selftest.cpp) Pointers to pointers, owning parameters, `*result` and written atoms (decision 0040, M4.5b1c3): pointer cells (a
// pointer variable whose address is taken: item 2), a `T**` parameter's contract, `*result` as a value (item 4), `owns`/`reads`/`live` written in
// a loop invariant (item 5: the entry and back-edge checks, the separation of two atoms, a guard that flips, the user's tie) and the clause that
// `[[cerastes::owning]]` desugars to (item 3: `p == NULL || owns(*p)` first in the `requires` and the `ensures`).
// PRE-REGISTERED from the record (item numbers below), before lowering ran these cases: each expectation names the category and the VIR node whose
// position the record gives (the store, the call, the atom for an entry check, the body's closing brace for a back-edge check, the return), so the
// cases test the position map as well.  Each rule has a case that verifies and a twin that fails for one reason (invariant 2).  The three
// `mem_rej_*` cases of b1a and b1c1 that item 2 and item 5 lift (`mem_rej_pointer_to_pointer`, `mem_rej_address_of_pointer_variable`,
// `mem_rej_atom_in_loop_invariant`) are lowering cases here (pre-approved flips).
namespace {

void define_pointer_cells() {
  auto PT = [](Kit& k) { return k.ptr(k.i32); };
  auto PP = [](Kit& k) { return k.ptr(k.ptr(k.i32)); };
  auto N = [](Kit& k, const char* v) { return k.num(k.i32, v); };
  auto FP = [](Kit& k, Function* f, int i) { return k.b.text(k.F(f, i), f->params[static_cast<size_t>(i)].name); };
  auto owns = [=](Kit& k, Function* f, int i, int amount) { return k.b.atom(AtomKind::Owns, FP(k, f, i), amount, k.at()); };
  auto live = [=](Kit& k, Function* f, int i, int amount) { return k.b.atom(AtomKind::Live, FP(k, f, i), amount, k.at()); };
  auto star = [=](Kit& k, Function* f, int i) { return k.deref(FP(k, f, i)); };  // `*p` in a contract
  auto old_star = [=](Kit& k, Function* f, int i) { return k.old(star(k, f, i)); };
  auto cell_init = [=](Kit& k, Function* f, const char* name, const char* v) {
    Var* x = k.cell(f, name, k.i32);
    return std::make_pair(x, k.decl(x, N(k, v)));
  };

  // ---- item 2: a pointer cell ----
  // int store(void) ensures(r == 5) { int x = 0; int *p = &x; int **pp = &p; [*pp = NULL;] **pp = 5; check(x == 5); return x; }
  const auto store = [=](Kit& k, bool null_first) -> SrcLoc {
    Function* f = k.fn("store", k.i32);
    if (!null_first) k.ens(f, k.eq_c(k.R(f), k.i32, "5"));
    auto [x, dx] = cell_init(k, f, "x", "0");
    Var* p = k.cell(f, "p", PT(k));
    Var* pp = k.local(f, "pp", PP(k));
    std::vector<const Stmt*> s = {dx, k.decl(p, k.addr(x)), k.decl(pp, k.addr(p))};
    if (null_first) s.push_back(k.store(k.pn(pp), k.nul(PT(k))));
    const Stmt* w = k.store(k.deref(k.pn(pp)), N(k, "5"));
    s.push_back(w);
    s.push_back(k.chk(k.eq_c(k.v(x), k.i32, "5")));
    s.push_back(k.ret(k.v(x)));
    f->body = k.blkc(s);
    k.done(f);
    return w->loc;
  };
  lc("mem_ptrcell_store", "int *p = &x; int **pp = &p; **pp = 5: the cell p holds &x, the store reaches x", [=](Kit& k) { store(k, false); });
  lc("mem_ptrcell_store_null", "*pp = NULL stores null into the cell p, so **pp = 5 applies * to null", [=](Kit& k) { k.fail("null-dereference", store(k, true)); });

  // int retarget(void) ensures(r == 7) { int x = 1, y = 7; int *p = &x; int **pp = &p; *pp = &y; check(p == [&y|&x]); return *p; }
  const auto retarget = [=](Kit& k, bool wrong) -> SrcLoc {
    Function* f = k.fn("retarget", k.i32);
    k.ens(f, k.eq_c(k.R(f), k.i32, "7"));
    auto [x, dx] = cell_init(k, f, "x", "1");
    auto [y, dy] = cell_init(k, f, "y", "7");
    Var* p = k.cell(f, "p", PT(k));
    Var* pp = k.local(f, "pp", PP(k));
    const Stmt* ck = k.chk(k.eq(k.pn(p), k.addr(wrong ? x : y)));
    f->body = k.blkc({dx, dy, k.decl(p, k.addr(x)), k.decl(pp, k.addr(p)), k.store(k.pn(pp), k.addr(y)), ck, k.ret(k.deref(k.pn(p)))});
    k.done(f);
    return ck->loc;
  };
  lc("mem_ptrcell_retarget", "*pp = &y retargets the cell p: check(p == &y) and *p reads y", [=](Kit& k) { retarget(k, false); });
  lc("mem_ptrcell_retarget_check", "after *pp = &y the old value p == &x no longer holds", [=](Kit& k) { k.fail("check", retarget(k, true)); });

  // int depth3(void) ensures(r == 9) { int x = 1; int *p = &x; int **pp = &p; int ***ppp = &pp; [*ppp = NULL;] ***ppp = 9; return x; }
  const auto depth3 = [=](Kit& k, bool null_middle) -> SrcLoc {
    Function* f = k.fn("depth3", k.i32);
    if (!null_middle) k.ens(f, k.eq_c(k.R(f), k.i32, "9"));
    auto [x, dx] = cell_init(k, f, "x", "1");
    Var* p = k.cell(f, "p", PT(k));
    Var* pp = k.cell(f, "pp", PP(k));
    Var* ppp = k.local(f, "ppp", k.ptr(PP(k)));
    std::vector<const Stmt*> s = {dx, k.decl(p, k.addr(x)), k.decl(pp, k.addr(p)), k.decl(ppp, k.addr(pp))};
    if (null_middle) s.push_back(k.store(k.pn(ppp), k.nul(PP(k))));
    const Expr* inner = k.deref(k.pn(ppp));
    const Expr* outer = k.deref(inner);
    const Stmt* w = k.store(outer, N(k, "9"));
    s.push_back(w);
    s.push_back(k.ret(k.v(x)));
    f->body = k.blkc(s);
    k.done(f);
    return outer->loc;
  };
  lc("mem_ptrcell_depth3", "int ***: a chain of three levels, ***ppp = 9 writes x", [=](Kit& k) { depth3(k, false); });
  lc("mem_ptrcell_depth3_null", "*ppp = NULL makes pp null: the load of *ppp is null, so **ppp applies * to null (reported at the dereference of the loaded pointer)", [=](Kit& k) { k.fail("null-dereference", depth3(k, true)); });

  // the flips of b1a's rejections: an int ** local compared with null, and a pointer local whose address is taken
  lc("mem_ptrcell_null_compare", "int **pp = 0; return pp == 0 (was mem_rej_pointer_to_pointer)", [=](Kit& k) {
    Function* f = k.fn("f", k.i32);
    k.ens(f, k.eq_c(k.R(f), k.i32, "1"));
    Var* pp = k.local(f, "pp", PP(k));
    f->body = k.blk({k.decl(pp, k.nul(PP(k))), k.ret(k.bi(k.i32, k.eq(k.pn(pp), k.nul(PP(k)))))});
    k.done(f);
  });
  lc("mem_ptrcell_address_unused", "a pointer local whose address is taken is a cell, released at the end (was mem_rej_address_of_pointer_variable)", [=](Kit& k) {
    Function* f = k.fn("f", k.vd);
    Var* p = k.cell(f, "p", PT(k));
    f->body = k.blkc({k.decl(p, k.nul(PT(k)))});
    k.done(f);
  });

  // void set(int **pp, int *q) requires(owns(*pp) && live(q)) ensures(owns(*pp) && [*pp == q | *pp == old(*pp)] && live(q)) { *pp = q; }
  const auto make_set = [=](Kit& k, bool wrong) -> Function* {
    Function* f = k.fn("set", k.vd, {{"pp", PP(k)}, {"q", PT(k)}});
    k.req(f, k.land(owns(k, f, 0, 0), live(k, f, 1, 1)));
    k.ens(f, k.land(k.land(owns(k, f, 0, 0), k.eq(star(k, f, 0), wrong ? old_star(k, f, 0) : k.F(f, 1))), live(k, f, 1, 1)));
    f->body = k.blk({k.store(k.pn(f->param_var(0)), k.pn(f->param_var(1)))});
    k.done(f);
    return f;
  };
  lc("mem_ptrcell_param_set", "set(pp, q): owns of a pointer cell (WF in place of a range), the callee stores q, *pp == q on return", [=](Kit& k) { make_set(k, false); });
  lc("mem_ptrcell_param_set_post", "the contract says *pp keeps its old value, the body stores q", [=](Kit& k) {
    Function* f = make_set(k, true);
    k.fail("postcondition", f->end_loc);
  });
  const auto set_caller = [=](Kit& k, bool unowned) -> const Expr* {
    Function* set = make_set(k, false);
    Function* f = unowned ? k.fn("use", k.vd, {{"pp", PP(k)}}) : k.fn("use", k.i32);
    if (!unowned) k.ens(f, k.eq_c(k.R(f), k.i32, "4"));
    if (unowned) k.req(f, k.b.atom(AtomKind::Live, k.b.text(k.F(f, 0), "pp"), 0, k.at()));
    if (unowned) {
      auto [y, dy] = cell_init(k, f, "y", "4");
      const Expr* c = k.call(set, {k.pn(f->param_var(0)), k.addr(y)});
      f->body = k.blkc({dy, k.es(c)});
      k.done(f);
      return c;
    }
    auto [x, dx] = cell_init(k, f, "x", "1");
    auto [y, dy] = cell_init(k, f, "y", "4");
    Var* p = k.cell(f, "p", PT(k));
    const Expr* c = k.call(set, {k.addr(p), k.addr(y)});
    f->body = k.blkc({dx, dy, k.decl(p, k.addr(x)), k.es(c), k.ret(k.deref(k.pn(p)))});
    k.done(f);
    return c;
  };
  lc("mem_ptrcell_param_set_caller", "int *p = &x; set(&p, &y); return *p == y's value", [=](Kit& k) { set_caller(k, false); });
  lc("mem_ptrcell_param_set_unowned", "the caller holds only live(pp): set's requires owns(*pp) fails at the call", [=](Kit& k) {
    const Expr* c = set_caller(k, true);
    k.fail_only("permission", c->loc, 17, 2);  // the bound of pp's block (live alone does not give it) and the write permission fail
    k.fail_only("permission", c->loc, 17, 3);
  });

  // ---- item 4: *result as a value ----
  // int *pick(int *p) requires(owns(*p) [&& *p < INT_MAX]) ensures(r == p && owns(*p) && *r == old(*p) [+ 1]) { return p; }   (mode 2: *r read with only live(p))
  const auto make_pick = [=](Kit& k, int mode, SrcLoc* read_loc = nullptr) -> Function* {  // 0: right, 1: *r == old(*p) + 1, 2: unframed (live only)
    Function* f = k.fn("pick", PT(k), {{"p", PT(k)}});
    if (mode == 2) {
      k.req(f, live(k, f, 0, 0));
      const Expr* rd = k.deref(k.R(f));
      if (read_loc != nullptr) *read_loc = rd->loc;
      k.ens(f, k.land(live(k, f, 0, 0), k.eq_c(rd, k.i32, "1")));
    } else {
      k.req(f, mode == 1 ? k.land(owns(k, f, 0, 0), k.lt(star(k, f, 0), N(k, "2147483647"))) : owns(k, f, 0, 0));
      const Expr* want = mode == 1 ? k.add(old_star(k, f, 0), N(k, "1")) : old_star(k, f, 0);
      k.ens(f, k.land(k.land(k.eq(k.R(f), FP(k, f, 0)), owns(k, f, 0, 0)), k.eq(k.deref(k.R(f)), want)));
    }
    f->body = k.blk({k.ret(k.pn(f->param_var(0)))});
    k.done(f);
    return f;
  };
  lc("mem_result_value", "*r in an ensures, framed by owns(*p) and r == p", [=](Kit& k) { make_pick(k, 0); });
  lc("mem_result_value_wrong", "*r is old(*p), not old(*p) + 1: the postcondition fails at the return", [=](Kit& k) {
    Function* f = make_pick(k, 1);
    k.fail("postcondition", f->body->stmts[0]->loc);
  });
  lc("mem_result_value_unframed", "*r read with only live(p): no earlier atom frames it, permission at the clause (and the chain's dummy value fails the postcondition)", [=](Kit& k) {
    SrcLoc rl;
    Function* f = make_pick(k, 2, &rl);
    k.fail("permission", rl);
    k.fail("postcondition", f->body->stmts[0]->loc);
  });
  lc("mem_result_value_caller", "int x = 7; int *q = pick(&x); return *q: the caller learns *r through the call", [=](Kit& k) {
    Function* pick = make_pick(k, 0);
    Function* f = k.fn("use", k.i32);
    k.ens(f, k.eq_c(k.R(f), k.i32, "7"));
    auto [x, dx] = cell_init(k, f, "x", "7");
    Var* q = k.local(f, "q", PT(k));
    f->body = k.blkc({dx, k.decl(q, k.call(pick, {k.addr(x)})), k.ret(k.deref(k.pn(q)))});
    k.done(f);
  });
}

void define_written_atoms() {
  auto PT = [](Kit& k) { return k.ptr(k.i32); };
  auto N = [](Kit& k, const char* v) { return k.num(k.i32, v); };
  auto FP = [](Kit& k, Function* f, int i) { return k.b.text(k.F(f, i), f->params[static_cast<size_t>(i)].name); };
  auto owns = [=](Kit& k, Function* f, int i, int amount) { return k.b.atom(AtomKind::Owns, FP(k, f, i), amount, k.at()); };
  auto reads = [=](Kit& k, Function* f, int i, int amount) { return k.b.atom(AtomKind::Reads, FP(k, f, i), amount, k.at()); };
  auto live = [=](Kit& k, Function* f, int i, int amount) { return k.b.atom(AtomKind::Live, FP(k, f, i), amount, k.at()); };
  auto star = [=](Kit& k, Function* f, int i) { return k.deref(FP(k, f, i)); };
  auto rd = [=](Kit& k, const Var* p) { return k.deref(k.pn(p)); };
  // An atom written in an invariant, on the variable `v`; its position is `at`.
  auto watom = [=](Kit& k, AtomKind kind, const Var* v, int amount, SrcLoc at) { return k.b.atom(kind, k.pn(v), amount, at); };
  auto make_dispose = [=](Kit& k) -> Function* {
    Function* d = k.fn("dispose", k.vd, {{"p", PT(k)}});
    k.req(d, owns(k, d, 0, 0));
    k.trusted(d);
    return d;
  };
  const auto cell_init = [=](Kit& k, Function* f, const char* name, const char* v) {
    Var* x = k.cell(f, name, k.i32);
    return std::make_pair(x, k.decl(x, N(k, v)));
  };

  // ---- item 5: a parameter the function assigns, with owns(*p) written in the invariant ----
  // void touch(int *p, int n) requires(owns(*p) && 0 <= n <= 100) ensures(owns(*p)) { q = p; p = q; i = 0; while (i < n) inv i <= n && owns(*p) { *p = i; i += 1; } }
  // mode 0: verifies; 1: the function holds only live(p): the loop enters without owning *p; 2: dispose(p) in the body: the iteration ends without it
  const auto touch = [=](Kit& k, int mode) -> SrcLoc {
    SrcLoc where;
    Function* dispose = mode == 2 ? make_dispose(k) : nullptr;
    Function* f = k.fn("touch", k.vd, {{"p", PT(k)}, {"n", k.i32}});
    k.req(f, k.land(mode == 1 ? live(k, f, 0, 0) : owns(k, f, 0, 0), k.range(k.F(f, 1), k.i32, "0", "100")));
    k.ens(f, mode == 1 ? live(k, f, 0, 0) : owns(k, f, 0, 0));
    Var* q = k.local(f, "q", PT(k));
    Var* i = k.local(f, "i", k.i32);
    const Var* p = f->param_var(0);
    const SrcLoc al = k.at();
    const Expr* atom = watom(k, AtomKind::Owns, p, 0, al);
    std::vector<const Stmt*> inner;
    inner.push_back(mode == 2 ? k.es(k.call(dispose, {k.pn(p)})) : k.store(k.pn(p), k.v(i)));
    inner.push_back(k.asg(i, k.add(k.v(i), N(k, "1"))));
    const Stmt* body = k.blkc(inner);
    if (mode == 1) where = al;
    if (mode == 2) where = body->end_loc;
    f->body = k.blk({k.decl(q, k.pn(p)), k.asg(p, k.pn(q)), k.decl(i, N(k, "0")),
                     k.whl(k.lt(k.v(i), k.pv(f, 1)), {k.cl(k.range(k.v(i), k.i32, "0", "100")), k.cl(k.le(k.v(i), k.pv(f, 1))), k.cl(atom)},
                           {k.cl(k.sub(k.pv(f, 1), k.v(i)))}, body)});
    k.done(f);
    return where;
  };
  lc("mem_atom_param", "touch(p, n): owns(*p) written in the invariant of a loop over a parameter the function assigns", [=](Kit& k) { touch(k, 0); });
  lc("mem_atom_param_entry", "the function holds only live(p): the loop enters without owning *p, reported at the atom", [=](Kit& k) { {
    const SrcLoc al = touch(k, 1);
    k.fail_only("permission", al, 10, 2);  // the bound of the block and the write permission
    k.fail_only("permission", al, 10, 3);
  }; });
  lc("mem_atom_param_consume", "dispose(p) in the body: an iteration ends without owning *p, reported at the body's closing brace", [=](Kit& k) { k.fail_only("permission", touch(k, 2), 10, 3); });

  // ---- a pointer local walking two cells, with the user's tie in the invariant ----
  // int walk(int *p, int n) requires(owns(*p) && *p == 1 && 0 <= n <= 100) ensures(owns(*p) && r == [1|2])
  // { int y = 1; int *q = p; i = 0; while (i < n) inv i <= n && (q == p || q == &y) && owns(*q) && *q == 1 { q = (q == p) ? &y : p; i += 1; } return *q; }
  const auto walk = [=](Kit& k, bool wrong) -> SrcLoc {
    Function* f = k.fn("walk", k.i32, {{"p", PT(k)}, {"n", k.i32}});
    k.req(f, k.land(k.land(owns(k, f, 0, 0), k.eq_c(star(k, f, 0), k.i32, "1")), k.range(k.F(f, 1), k.i32, "0", "100")));
    k.ens(f, k.land(owns(k, f, 0, 0), k.eq_c(k.R(f), k.i32, wrong ? "2" : "1")));
    auto [y, dy] = cell_init(k, f, "y", "1");
    Var* q = k.local(f, "q", PT(k));
    Var* i = k.local(f, "i", k.i32);
    const Var* p = f->param_var(0);
    const Expr* atom = watom(k, AtomKind::Owns, q, 0, k.at());
    const Expr* inv = k.land(k.land(k.land(k.le(k.v(i), k.pv(f, 1)), k.lor(k.eq(k.pn(q), k.pn(p)), k.eq(k.pn(q), k.addr(y)))), atom), k.eq_c(rd(k, q), k.i32, "1"));
    const Stmt* step = k.asg(q, k.cond(k.eq(k.pn(q), k.pn(p)), k.addr(y), k.pn(p)));
    const Stmt* r = k.ret(rd(k, q));
    f->body = k.blkc({dy, k.decl(q, k.pn(p)), k.decl(i, N(k, "0")),
                      k.whl(k.lt(k.v(i), k.pv(f, 1)), {k.cl(k.range(k.v(i), k.i32, "0", "100")), k.cl(inv)}, {k.cl(k.sub(k.pv(f, 1), k.v(i)))},
                            k.blk({step, k.asg(i, k.add(k.v(i), N(k, "1")))})),
                      r});
    k.done(f);
    return r->loc;
  };
  lc("mem_atom_walk", "a pointer local walking p and &y, owns(*q) written with q == p || q == &y, the atom frames *q", [=](Kit& k) { walk(k, false); });
  lc("mem_atom_walk_wrong", "both cells hold 1, so the walk returns 1, not 2", [=](Kit& k) { k.fail("postcondition", walk(k, true)); });

  // ---- S28 P8: the loop assigns p; the invariant says where it ends (p == q) ----
  // int tie(int *p, int n) requires(owns(*p) && range) ensures(owns(*p)) { q = p; i = 0; while (i < n) inv i <= n && [p == q &&] live(q) && owns(*p) { *p = i; p = q; i += 1; } return i; }
  const auto tie = [=](Kit& k, bool with_tie, SrcLoc* back_edge) -> SrcLoc {
    Function* f = k.fn("tie", k.i32, {{"p", PT(k)}, {"n", k.i32}});
    k.req(f, k.land(owns(k, f, 0, 0), k.range(k.F(f, 1), k.i32, "0", "100")));
    k.ens(f, owns(k, f, 0, 0));
    Var* q = k.local(f, "q", PT(k));
    Var* i = k.local(f, "i", k.i32);
    const Var* p = f->param_var(0);
    const Expr* la = watom(k, AtomKind::Live, q, 0, k.at());
    const Expr* oa = watom(k, AtomKind::Owns, p, 1, k.at());
    const Expr* inv = k.land(k.land(k.le(k.v(i), k.pv(f, 1)), la), oa);
    if (with_tie) inv = k.land(k.land(k.le(k.v(i), k.pv(f, 1)), k.eq(k.pn(p), k.pn(q))), k.land(la, oa));
    const Stmt* body = k.blkc({k.store(k.pn(p), k.v(i)), k.asg(p, k.pn(q)), k.asg(i, k.add(k.v(i), N(k, "1")))});
    const Stmt* r = k.ret(k.v(i));
    f->body = k.blk({k.decl(q, k.pn(p)), k.decl(i, N(k, "0")),
                     k.whl(k.lt(k.v(i), k.pv(f, 1)), {k.cl(k.range(k.v(i), k.i32, "0", "100")), k.cl(inv)}, {k.cl(k.sub(k.pv(f, 1), k.v(i)))}, body), r});
    k.done(f);
    if (back_edge != nullptr) *back_edge = body->end_loc;
    return r->loc;
  };
  lc("mem_atom_tie", "the loop assigns p = q, the invariant says p == q: the permission comes back for the ensures", [=](Kit& k) { tie(k, true, nullptr); });
  lc("mem_atom_tie_gap", "without p == q the back edge cannot show the place p = q is held, and the ensures cannot get the entry place back after the loop", [=](Kit& k) {
    SrcLoc be;
    const SrcLoc ret = tie(k, false, &be);
    k.fail_only("permission", be, 14, 7);
    k.fail_only("permission", ret, 6, 1);
  });

  // ---- reads(*p) and live(p) written in an invariant ----
  // int total(int *p, int n) requires(reads(*p) && 0 <= *p <= 10 && range) ensures(reads(*p) && 0 <= r <= 10 * n)
  // { q = p; p = q; s = 0; i = 0; while (i < n) inv i <= n && 0 <= s <= 10 * i && reads(*p) && 0 <= *p <= 10 { s += *p; [*p = 0;] i += 1; } return s; }
  const auto total = [=](Kit& k, bool write) -> SrcLoc {
    SrcLoc where;
    Function* f = k.fn("total", k.i32, {{"p", PT(k)}, {"n", k.i32}});
    k.req(f, k.land(k.land(reads(k, f, 0, 0), k.range(star(k, f, 0), k.i32, "0", "10")), k.range(k.F(f, 1), k.i32, "0", "100")));
    k.ens(f, k.land(reads(k, f, 0, 0), k.range(k.R(f), k.i32, "0", "1000")));
    Var* q = k.local(f, "q", PT(k));
    Var* s = k.local(f, "s", k.i32);
    Var* i = k.local(f, "i", k.i32);
    const Var* p = f->param_var(0);
    const Expr* atom = watom(k, AtomKind::Reads, p, 0, k.at());
    const Expr* inv = k.land(k.land(k.land(k.le(k.v(i), k.pv(f, 1)), k.range(k.v(s), k.i32, "0", "1000")), k.le(k.v(s), k.mul(N(k, "10"), k.v(i)))),
                             k.land(atom, k.range(rd(k, p), k.i32, "0", "10")));
    std::vector<const Stmt*> inner = {k.asg(s, k.add(k.v(s), rd(k, p)))};
    if (write) {
      const Stmt* w = k.store(k.pn(p), N(k, "0"));
      where = w->loc;
      inner.push_back(w);
    }
    inner.push_back(k.asg(i, k.add(k.v(i), N(k, "1"))));
    f->body = k.blk({k.decl(q, k.pn(p)), k.asg(p, k.pn(q)), k.decl(s, N(k, "0")), k.decl(i, N(k, "0")),
                     k.whl(k.lt(k.v(i), k.pv(f, 1)), {k.cl(k.range(k.v(i), k.i32, "0", "100")), k.cl(inv)}, {k.cl(k.sub(k.pv(f, 1), k.v(i)))}, k.blk(inner)),
                     k.ret(k.v(s))});
    k.done(f);
    return where;
  };
  lc("mem_atom_reads", "reads(*p) written in the invariant of a loop over a reassigned parameter: the atom frames the invariant's reads of *p", [=](Kit& k) { total(k, false); });
  lc("mem_atom_reads_write", "a write through p under reads(*p) has no write permission", [=](Kit& k) { k.fail("permission", total(k, true)); });

  // int live_loop(int *p, int n) requires([live(p) &&] range) ensures([live(p) &&] r == n) { q = p; p = q; i = 0; while (i < n) inv i <= n && live(p) { i += 1; } return i; }
  const auto live_loop = [=](Kit& k, bool held) -> SrcLoc {
    Function* f = k.fn("live_loop", k.i32, {{"p", PT(k)}, {"n", k.i32}});
    const Expr* r = k.range(k.F(f, 1), k.i32, "0", "100");
    k.req(f, held ? k.land(live(k, f, 0, 0), r) : r);
    const Expr* e = k.eq(k.R(f), k.F(f, 1));
    k.ens(f, held ? k.land(live(k, f, 0, 0), e) : e);
    Var* q = k.local(f, "q", PT(k));
    Var* i = k.local(f, "i", k.i32);
    const Var* p = f->param_var(0);
    const SrcLoc al = k.at();
    const Expr* atom = watom(k, AtomKind::Live, p, 0, al);
    std::vector<const Stmt*> pre;
    if (held) {  // q = p; p = q (the copy is a use of p, which needs live(p))
      pre.push_back(k.decl(q, k.pn(p)));
      pre.push_back(k.asg(p, k.pn(q)));
    }
    pre.push_back(k.decl(i, N(k, "0")));
    pre.push_back(
                     k.whl(k.lt(k.v(i), k.pv(f, 1)), {k.cl(k.range(k.v(i), k.i32, "0", "100")), k.cl(k.le(k.v(i), k.pv(f, 1))), k.cl(atom)},
                           {k.cl(k.sub(k.pv(f, 1), k.v(i)))}, k.blk({k.asg(i, k.add(k.v(i), N(k, "1")))})));
    pre.push_back(k.ret(k.v(i)));
    f->body = k.blk(pre);
    k.done(f);
    return al;
  };
  lc("mem_atom_live", "live(p) written in the invariant of a loop over a reassigned parameter: the liveness comes back for the ensures", [=](Kit& k) { live_loop(k, true); });
  lc("mem_atom_live_entry", "nothing says p points to a live object: the loop enters without the liveness its invariant needs", [=](Kit& k) {
    const SrcLoc al = live_loop(k, false);
    k.fail_only("permission", al, 4, 0);  // the null check and the liveness permission
    k.fail_only("permission", al, 4, 2);
  });

  // ---- two written atoms: owns(*q) && owns(*s) ----
  // void two(int *p, int *r, int n) requires(owns(*p) && owns(*r) && range) { a = p; b = r; p = a; r = b; q = p; s = [r | n == 5 ? p : r]; i = 0;
  //   while (i < n) inv i <= n && owns(*q) && owns(*s) { *q = i; *s = i; i += 1; } }
  const auto two = [=](Kit& k, bool may_alias) -> SrcLoc {
    Function* f = k.fn("two", k.vd, {{"p", PT(k)}, {"r", PT(k)}, {"n", k.i32}});
    k.req(f, k.land(k.land(owns(k, f, 0, 0), owns(k, f, 1, 1)), k.range(k.F(f, 2), k.i32, "0", "10")));
    Var* a = k.local(f, "a", PT(k));
    Var* b = k.local(f, "b", PT(k));
    Var* q = k.local(f, "q", PT(k));
    Var* s = k.local(f, "s", PT(k));
    Var* i = k.local(f, "i", k.i32);
    const Var* p = f->param_var(0);
    const Var* r = f->param_var(1);
    const Expr* a1 = watom(k, AtomKind::Owns, q, 0, k.at());
    const SrcLoc l2 = k.at();
    const Expr* a2 = watom(k, AtomKind::Owns, s, 1, l2);
    const Expr* sv = may_alias ? k.cond(k.eq(k.pv(f, 2), N(k, "5")), k.pn(p), k.pn(r)) : k.pn(r);
    f->body = k.blk({k.decl(a, k.pn(p)), k.decl(b, k.pn(r)), k.asg(p, k.pn(a)), k.asg(r, k.pn(b)), k.decl(q, k.pn(p)), k.decl(s, sv), k.decl(i, N(k, "0")),
                     k.whl(k.lt(k.v(i), k.pv(f, 2)), {k.cl(k.range(k.v(i), k.i32, "0", "10")), k.cl(k.le(k.v(i), k.pv(f, 2))), k.cl(a1), k.cl(a2)},
                           {k.cl(k.sub(k.pv(f, 2), k.v(i)))},
                           k.blk({k.store(k.pn(q), k.v(i)), k.store(k.pn(s), k.v(i)), k.asg(i, k.add(k.v(i), N(k, "1")))}))});
    k.done(f);
    return l2;
  };
  lc("mem_atom_two", "two owns atoms on two distinct owned places", [=](Kit& k) { two(k, false); });
  lc("mem_atom_two_sep", "s may be q (n == 5): two owns atoms on one place, the separation fails at the later atom", [=](Kit& k) { k.fail_only("permission", two(k, true), 11, 10); });

  // ---- an atom whose guard is false at entry and true after q = p ----
  // int flip(int *p, int n) requires(owns(*p) && range) ensures(owns(*p) && (r == 0 || r == n))
  // { a = p; p = a; q = NULL; i = 0; while (i < n) inv i <= n && live(p) && (q == NULL || owns(*q)) { q = p; i += 1; } return q == NULL ? 0 : n; }
  lc("mem_atom_guard_flip", "the guard q == NULL || owns(*q) is false at entry, so the loop holds nothing of p's place; q = p makes it true and the back edge cannot show it", [=](Kit& k) {
    Function* f = k.fn("flip", k.i32, {{"p", PT(k)}, {"n", k.i32}});
    k.req(f, k.land(owns(k, f, 0, 0), k.range(k.F(f, 1), k.i32, "0", "10")));
    k.ens(f, k.land(owns(k, f, 0, 0), k.lor(k.eq_c(k.R(f), k.i32, "0"), k.eq(k.R(f), k.F(f, 1)))));
    Var* a = k.local(f, "a", PT(k));
    Var* q = k.local(f, "q", PT(k));
    Var* i = k.local(f, "i", k.i32);
    const Var* p = f->param_var(0);
    const Expr* la = watom(k, AtomKind::Live, p, 0, k.at());
    const Expr* oa = watom(k, AtomKind::Owns, q, 1, k.at());
    const Expr* inv = k.land(k.land(k.le(k.v(i), k.pv(f, 1)), la), k.lor(k.eq(k.pn(q), k.nul(PT(k))), oa));
    const Stmt* body = k.blkc({k.asg(q, k.pn(p)), k.asg(i, k.add(k.v(i), N(k, "1")))});
    f->body = k.blk({k.decl(a, k.pn(p)), k.asg(p, k.pn(a)), k.decl(q, k.nul(PT(k))), k.decl(i, N(k, "0")),
                     k.whl(k.lt(k.v(i), k.pv(f, 1)), {k.cl(k.range(k.v(i), k.i32, "0", "10")), k.cl(inv)}, {k.cl(k.sub(k.pv(f, 1), k.v(i)))}, body),
                     k.ret(k.cond(k.eq(k.pn(q), k.nul(PT(k))), N(k, "0"), k.pv(f, 1)))});
    k.done(f);
    k.fail_only("permission", body->end_loc, 14, 7);
  });

  // ---- an atom on an automatic place (p is never assigned): it takes nothing twice ----
  // void fill(int *p, int n) requires([owns|reads](*p) && range) ensures([owns|reads](*p) [&& *p == n]) { i = 0; while (i < n) inv i <= n && owns(*p) { *p = i; i += 1; } }
  const auto fill = [=](Kit& k, int mode) -> SrcLoc {  // 0 verifies; 1: ensures *p == n; 2: the function holds only reads(*p)
    Function* f = k.fn("fill", k.vd, {{"p", PT(k)}, {"n", k.i32}});
    const Expr* r = k.range(k.F(f, 1), k.i32, "0", "100");
    k.req(f, k.land(mode == 2 ? reads(k, f, 0, 0) : owns(k, f, 0, 0), r));
    k.ens(f, mode == 1 ? k.land(owns(k, f, 0, 0), k.eq(star(k, f, 0), k.F(f, 1))) : (mode == 2 ? reads(k, f, 0, 0) : owns(k, f, 0, 0)));
    Var* i = k.local(f, "i", k.i32);
    const Var* p = f->param_var(0);
    const SrcLoc al = k.at();
    const Expr* atom = watom(k, AtomKind::Owns, p, 0, al);
    f->body = k.blk({k.decl(i, N(k, "0")),
                     k.whl(k.lt(k.v(i), k.pv(f, 1)), {k.cl(k.range(k.v(i), k.i32, "0", "100")), k.cl(k.le(k.v(i), k.pv(f, 1))), k.cl(atom)},
                           {k.cl(k.sub(k.pv(f, 1), k.v(i)))}, k.blk({mode == 2 ? k.es(N(k, "0")) : k.store(k.pn(p), k.v(i)), k.asg(i, k.add(k.v(i), N(k, "1")))}))});
    k.done(f);
    return mode == 1 ? f->end_loc : al;
  };
  lc("mem_atom_automatic", "owns(*p) written on an automatic place (p is never assigned): the atom takes nothing twice (was mem_rej_atom_in_loop_invariant)", [=](Kit& k) { fill(k, 0); });
  lc("mem_atom_automatic_post", "the atom does not say what the loop wrote, so *p == n is not known after it", [=](Kit& k) { k.fail("postcondition", fill(k, 1)); });
  lc("mem_atom_owns_on_reads", "owns(*p) written on an automatic place the function holds only as reads(*p): fails at entry", [=](Kit& k) { k.fail_only("permission", fill(k, 2), 10, 3); });

  // ---- item 3: the clause owning desugars to: p == NULL || owns(*p) in the requires and the ensures ----
  // void set_one(int *p) requires(p == NULL || owns(*p)) ensures(p == NULL || owns(*p)) { [if (p != NULL)] *p = 1; }
  // mode 0 verifies; 1: no test (null-dereference); 2: keep(p) consumes the cell (permission at the end)
  const auto owning = [=](Kit& k, int mode) -> SrcLoc {
    Function* keep = nullptr;
    if (mode == 2) {
      keep = k.fn("keep", k.vd, {{"p", PT(k)}});
      k.req(keep, owns(k, keep, 0, 0));
      k.trusted(keep);
    }
    Function* f = k.fn("set_one", k.vd, {{"p", PT(k)}});
    k.req(f, k.lor(k.eq(k.F(f, 0), k.nul(PT(k))), owns(k, f, 0, 0)));
    k.ens(f, k.lor(k.eq(k.F(f, 0), k.nul(PT(k))), owns(k, f, 0, 0)));
    const Var* p = f->param_var(0);
    const Stmt* act = mode == 2 ? k.es(k.call(keep, {k.pn(p)})) : k.store(k.pn(p), N(k, "1"));
    f->body = k.blk({mode == 1 ? act : k.iff(k.ne(k.pn(p), k.nul(PT(k))), k.blk({act}))});
    k.done(f);
    return mode == 1 ? act->loc : f->end_loc;
  };
  lc("mem_owning_clause", "p == NULL || owns(*p) in both clauses: the body tests p before it writes", [=](Kit& k) { owning(k, 0); });
  lc("mem_owning_clause_null", "an owning parameter may be null: *p = 1 without a test applies * to null", [=](Kit& k) { k.fail("null-dereference", owning(k, 1)); });
  lc("mem_owning_clause_kept", "keep(p) consumes the cell: the ensures p == NULL || owns(*p) fails at the end", [=](Kit& k) { k.fail_only("permission", owning(k, 2), 6, 1); });
  lc("mem_owning_clause_caller", "set_one(&x); set_one(NULL); x = 3", [=](Kit& k) {
    Function* g = k.fn("set_one", k.vd, {{"p", PT(k)}});
    k.req(g, k.lor(k.eq(k.F(g, 0), k.nul(PT(k))), owns(k, g, 0, 0)));
    k.ens(g, k.lor(k.eq(k.F(g, 0), k.nul(PT(k))), owns(k, g, 0, 0)));
    k.trusted(g);
    Function* f = k.fn("use", k.i32);
    k.ens(f, k.eq_c(k.R(f), k.i32, "3"));
    auto [x, dx] = cell_init(k, f, "x", "0");
    f->body = k.blkc({dx, k.es(k.call(g, {k.addr(x)})), k.es(k.call(g, {k.nul(PT(k))})), k.asg(x, N(k, "3")), k.ret(k.v(x))});
    k.done(f);
  });
}

}  // namespace
