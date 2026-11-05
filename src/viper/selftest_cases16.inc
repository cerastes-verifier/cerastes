// (included by selftest.cpp) Places reached through a heap read (decision 0045, M4.5c2b): `owns(**pp)`, `live(*pp)`, `reads(**pp)`, `old(**pp)` and
// `decreases(**pp)` over a T** parameter (items 2-8), a member chain `p->q->x` and a by-value parameter's pointer member `*s.q` (items 3, 9), the loops
// that hold them (item 10: the entry snapshots, a written atom on `**pp`) and the backstop rejections of item 16.  The categories and the VIR node that
// carries each position come from the record (the call, the `return` or closing brace, the contract clause, the invariant clause, the body's closing
// brace); the row COUNTS of the expectations were taken from the lowering, which the case file first built (the record does not fix how many rows a
// call or a return has).  Each rule has a case that verifies and a twin that fails for one reason (invariant 2).
namespace {

void define_heap_read_places()
{
    auto PT = [](Kit& k)
    {
        return k.ptr(k.i32);
    };
    auto PPT = [](Kit& k)
    {
        return k.ptr(k.ptr(k.i32));
    };
    auto N = [](Kit& k, const char* v)
    {
        return k.num(k.i32, v);
    };
    auto FP = [](Kit& k, Function* f, int i)
    {
        return k.b.text(k.F(f, i), f->params[static_cast<size_t>(i)].name);
    };
    auto owns = [](Kit& k, const Expr* place, int amount)
    {
        return k.b.atom(AtomKind::Owns, place, amount, k.at());
    };
    auto reads = [](Kit& k, const Expr* place, int amount)
    {
        return k.b.atom(AtomKind::Reads, place, amount, k.at());
    };
    auto live = [](Kit& k, const Expr* place, int amount)
    {
        return k.b.atom(AtomKind::Live, place, amount, k.at());
    };
    // `*pp` in a contract (the pointer a contract loads), `**pp` (the int behind it)
    auto inner = [=](Kit& k, Function* f, int i)
    {
        return k.deref(FP(k, f, i));
    };
    auto inner2 = [=](Kit& k, Function* f, int i)
    {
        return k.deref(k.deref(FP(k, f, i)));
    };
    auto reqc = [](Kit& k, Function* f, const Expr* e)
    {
        Clause c {k.cl(e)};
        f->requires_.push_back(c);
        return c.loc;
    };
    auto cell_init = [=](Kit& k, Function* f, const char* name, const char* v)
    {
        Var* x {k.cell(f, name, k.i32)};
        return std::make_pair(x, k.decl(x, N(k, v)));
    };

    // ---- owns(*pp) && owns(**pp): the inner atom reads *pp through the atom before it (items 3, 4, 6) ----
    // void set(int **pp) requires(owns(*pp) && owns(**pp)) ensures(owns(*pp) && *pp == old(*pp) && owns(**pp) && **pp == 5) { **pp = 5; }
    const auto make_set = [=](Kit& k, const char* val) -> Function*
    {
        Function* f {k.fn("set", k.vd, {{"pp", PPT(k)}})};
        k.req(f, k.land(owns(k, FP(k, f, 0), 0), owns(k, inner(k, f, 0), 1)));
        k.ens(f, k.land(k.land(k.land(owns(k, FP(k, f, 0), 0), k.eq(inner(k, f, 0), k.old(inner(k, f, 0)))),
                               owns(k, inner(k, f, 0), 1)),
                        k.eq_c(inner2(k, f, 0), k.i32, val)));
        f->body = k.blk({k.store(k.deref(k.pn(f->param_var(0))), N(k, "5"))});
        k.done(f);
        return f;
    };
    lc("hr_set", "set(pp) with owns(*pp) && owns(**pp) and **pp == 5: the callee verifies against its contract",
       [=](Kit& k)
       {
           make_set(k, "5");
       });
    lc("hr_set_wrong", "the same promising 6: the value ensures fails at the closing brace",
       [=](Kit& k)
       {
           k.fail("postcondition", make_set(k, "6")->end_loc);
       });
    lc("hr_set_unframed",
       "owns(**pp) written before owns(*pp): the contract reads *pp without owning it, reported at the read of *pp",
       [=](Kit& k)
       {
           Function* f {k.fn("set", k.vd, {{"pp", PPT(k)}})};
           const Expr* load {inner(k, f, 0)}; // the contract's read of *pp: the node the framing check is reported at
           reqc(k, f, k.land(owns(k, load, 0), owns(k, FP(k, f, 0), 1)));
           f->body = k.blk({});
           k.done(f);
           k.fail("permission", load->loc);
       });
    // int use(void) ensures(r == 5) { int x = 0; int *p = &x; set(&p); check(x == [5|6]); return x; }
    const auto use_set = [=](Kit& k, bool wrong) -> SrcLoc
    {
        Function* set {make_set(k, "5")};
        Function* f {k.fn("use", k.i32)};
        auto [x, dx] = cell_init(k, f, "x", "0");
        Var* p {k.cell(f, "p", PT(k))};
        const Stmt* c {k.chk(k.eq_c(k.v(x), k.i32, wrong ? "6" : "5"))};
        f->body = k.blkc({dx, k.decl(p, k.addr(x)), k.es(k.call(set, {k.addr(p)})), c, k.ret(k.v(x))});
        k.done(f);
        return c->loc;
    };
    lc("hr_set_caller", "int x = 0; int *p = &x; set(&p); check(x == 5): the callee's ensures come back through p",
       [=](Kit& k)
       {
           use_set(k, false);
       });
    lc("hr_set_caller_check", "check(x == 6) does not hold",
       [=](Kit& k)
       {
           k.fail("check", use_set(k, true));
       });

    // ---- live(*pp): the loaded pointer is usable (items 3, 4, 6) ----
    // void keep(int **pp) requires(owns(*pp) && live(*pp)) ensures(owns(*pp) && *pp == old(*pp) && live(*pp)) { [int local = 1; *pp = &local;] }
    const auto make_keep = [=](Kit& k, bool escape) -> Function*
    {
        Function* f {k.fn("keep", k.vd, {{"pp", PPT(k)}})};
        k.req(f, k.land(owns(k, FP(k, f, 0), 0), live(k, inner(k, f, 0), 1)));
        k.ens(f, k.land(k.land(owns(k, FP(k, f, 0), 0), k.eq(inner(k, f, 0), k.old(inner(k, f, 0)))),
                        live(k, inner(k, f, 0), 1)));
        if (escape)
        {
            Var* local {k.cell(f, "local", k.i32)};
            f->body = k.blkc({k.decl(local, N(k, "1")), k.store(k.pn(f->param_var(0)), k.addr(local))});
        }
        else
        {
            f->body = k.blk({});
        }
        k.done(f);
        return f;
    };
    lc("hr_live", "live(*pp) in requires and ensures with *pp == old(*pp): nothing to give back",
       [=](Kit& k)
       {
           make_keep(k, false);
       });
    lc("hr_live_escape", "*pp = &local: the local dies at the scope exit, so the return does not give back live(*pp)",
       [=](Kit& k)
       {
           Function* f {make_keep(k, true)};
           // the liveness row of live(*pp); the rows of owns(*pp) and the root rows hold
           k.fail_only("permission", f->end_loc, 9U, 8U);
       });

    // ---- a move: *pp = q, the ensures names the object on return and its amount follows the object (items 3, 7) ----
    // void move(int **pp, int *q) requires(owns(*pp) && owns(**pp) && owns(*q))
    //   ensures(owns(*pp) && *pp == q && owns(**pp) && **pp == [old(*q)|old(**pp)]) { *pp = q; }
    const auto make_move = [=](Kit& k, bool keeps_value) -> Function*
    {
        Function* f {k.fn("move", k.vd, {{"pp", PPT(k)}, {"q", PT(k)}})};
        k.req(f, k.land(k.land(owns(k, FP(k, f, 0), 0), owns(k, inner(k, f, 0), 1)), owns(k, FP(k, f, 1), 2)));
        const Expr* value {keeps_value ? k.old(inner2(k, f, 0)) : k.old(k.deref(FP(k, f, 1)))};
        k.ens(f, k.land(k.land(k.land(owns(k, FP(k, f, 0), 0), k.eq(inner(k, f, 0), FP(k, f, 1))),
                               owns(k, inner(k, f, 0), 1)),
                        k.eq(inner2(k, f, 0), value)));
        f->body = k.blk({k.store(k.pn(f->param_var(0)), k.pn(f->param_var(1)))});
        k.done(f);
        return f;
    };
    lc("hr_move", "*pp = q: owns(**pp) after the move names q's object at the amount it was given",
       [=](Kit& k)
       {
           make_move(k, false);
       });
    lc("hr_move_value", "**pp == old(**pp) after a move: q's object need not hold the old value (postcondition)",
       [=](Kit& k)
       {
           k.fail("postcondition", make_move(k, true)->end_loc);
       });

    // ---- a guard that reads the heap (item 11): *pp == NULL || owns(**pp) ----
    // void h(int **pp) requires(owns(*pp) && (*pp == NULL || owns(**pp))) ensures(owns(*pp) && *pp == old(*pp) && (*pp == NULL || owns(**pp))) { if (*pp != NULL) **pp = 1; }
    lc("hr_guard",
       "a heap-reading guard: the atom on **pp holds where *pp is not null, the body writes under the same test",
       [=](Kit& k)
       {
           Function* f {k.fn("h", k.vd, {{"pp", PPT(k)}})};
           auto guarded = [&](Kit& kk)
           {
               return kk.lor(kk.eq(inner(kk, f, 0), kk.nul(PT(kk))), owns(kk, inner(kk, f, 0), 1));
           };
           k.req(f, k.land(owns(k, FP(k, f, 0), 0), guarded(k)));
           k.ens(f, k.land(k.land(owns(k, FP(k, f, 0), 0), k.eq(inner(k, f, 0), k.old(inner(k, f, 0)))), guarded(k)));
           const Var* pp {f->param_var(0)};
           f->body = k.blk({k.iff(k.ne(k.deref(k.pn(pp)), k.nul(PT(k))), k.store(k.deref(k.pn(pp)), N(k, "1")))});
           k.done(f);
       });
    lc("hr_guard_unframed",
       "the guard *pp == NULL is written before owns(*pp): nothing frames its read of *pp (reported at the clause)",
       [=](Kit& k)
       {
           Function* f {k.fn("h", k.vd, {{"pp", PPT(k)}})};
           const Expr* load {inner(k, f, 0)}; // the guard's read of *pp
           reqc(k, f, k.land(k.lor(k.eq(load, k.nul(PT(k))), owns(k, inner(k, f, 0), 0)), owns(k, FP(k, f, 0), 1)));
           f->body = k.blk({});
           k.done(f);
           k.fail("permission", load->loc);
       });

    // ---- reads(**pp) and a write under it ----
    // int peek(int **pp) requires(reads(*pp) && reads(**pp) && **pp < 100) ensures(reads(*pp) && reads(**pp) && r == **pp + 1) { return **pp + 1; }
    const auto make_peek = [=](Kit& k, bool write) -> const Stmt*
    {
        Function* f {k.fn("peek", k.i32, {{"pp", PPT(k)}})};
        k.req(f, k.land(k.land(reads(k, FP(k, f, 0), 0), reads(k, inner(k, f, 0), 1)),
                        k.lt(inner2(k, f, 0), N(k, "100"))));
        k.ens(f, k.land(k.land(reads(k, FP(k, f, 0), 0), reads(k, inner(k, f, 0), 1)),
                        k.eq(k.R(f), k.add(inner2(k, f, 0), N(k, "1")))));
        const Var* pp {f->param_var(0)};
        const Stmt* w {k.store(k.deref(k.pn(pp)), N(k, "0"))};
        const Stmt* r {k.ret(k.add(k.deref(k.deref(k.pn(pp))), N(k, "1")))};
        f->body = k.blk(write ? std::vector<const Stmt*> {w, r} : std::vector<const Stmt*> {r});
        k.done(f);
        return w;
    };
    lc("hr_reads", "reads(*pp) && reads(**pp) on both sides: the callee reads **pp + 1",
       [=](Kit& k)
       {
           make_peek(k, false);
       });
    lc("hr_reads_write", "**pp = 0 under reads(**pp): a write needs the whole permission",
       [=](Kit& k)
       {
           k.fail("permission", make_peek(k, true)->loc);
       });

    // ---- decreases(**pp) on a recursive function (item 8) ----
    // void countdown(int **pp) requires(owns(*pp) && owns(**pp) && **pp >= 0) ensures(owns(*pp) && *pp == old(*pp) && owns(**pp) && **pp == 0) decreases(**pp)
    // { if (**pp > 0) { [**pp -= 1;] countdown(pp); } }
    const auto make_countdown = [=](Kit& k, bool decrease) -> SrcLoc
    {
        Function* f {k.fn("countdown", k.vd, {{"pp", PPT(k)}})};
        k.req(f,
              k.land(k.land(owns(k, FP(k, f, 0), 0), owns(k, inner(k, f, 0), 1)), k.ge_c(inner2(k, f, 0), k.i32, "0")));
        k.ens(f, k.land(k.land(k.land(owns(k, FP(k, f, 0), 0), k.eq(inner(k, f, 0), k.old(inner(k, f, 0)))),
                               owns(k, inner(k, f, 0), 1)),
                        k.eq_c(inner2(k, f, 0), k.i32, "0")));
        k.dec(f, inner2(k, f, 0));
        const Var* pp {f->param_var(0)};
        const Expr* call {k.call(f, {k.pn(pp)})};
        std::vector<const Stmt*> then;
        if (decrease)
        {
            then.push_back(k.store(k.deref(k.pn(pp)), k.sub(k.deref(k.deref(k.pn(pp))), N(k, "1"))));
        }
        then.push_back(k.es(call));
        f->body = k.blk({k.iff(k.gt(k.deref(k.deref(k.pn(pp))), N(k, "0")), k.blk(then))});
        k.done(f);
        return call->loc;
    };
    lc("hr_decreases",
       "the measure **pp is read in the entry heap; the recursive call's measure is the callee's at the arguments in the current heap",
       [=](Kit& k)
       {
           make_countdown(k, true);
       });
    lc("hr_decreases_stuck", "the body forgets **pp -= 1: the recursive call does not decrease the measure",
       [=](Kit& k)
       {
           k.fail("termination", make_countdown(k, false));
       });

    // ---- a loop that holds **pp through its requires places (item 10) ----
    // void bump_n(int **pp, int n) requires(owns(*pp) && owns(**pp) && 0 <= n <= 100 && **pp == 0) ensures(owns(*pp) && *pp == old(*pp) && owns(**pp))
    // { int i = 0; while (i < n) invariant 0 <= i <= n && **pp == [i|0] { **pp += 1; i += 1; } }
    const auto make_loop = [=](Kit& k, bool wrong) -> SrcLoc
    {
        Function* f {k.fn("bump_n", k.vd, {{"pp", PPT(k)}, {"n", k.i32}})};
        k.req(f, k.land(k.land(k.land(owns(k, FP(k, f, 0), 0), owns(k, inner(k, f, 0), 1)),
                               k.range(k.F(f, 1), k.i32, "0", "100")),
                        k.eq_c(inner2(k, f, 0), k.i32, "0")));
        k.ens(f, k.land(k.land(owns(k, FP(k, f, 0), 0), k.eq(inner(k, f, 0), k.old(inner(k, f, 0)))),
                        owns(k, inner(k, f, 0), 1)));
        Var* i {k.local(f, "i", k.i32)};
        const Var* pp {f->param_var(0)};
        const SrcLoc where {k.at()};
        const Expr* inv {k.eq(k.deref(k.deref(k.pn(pp))), wrong ? N(k, "0") : k.v(i))};
        const Stmt* body {k.blk({k.store(k.deref(k.pn(pp)), k.add(k.deref(k.deref(k.pn(pp))), N(k, "1"))),
                                 k.asg(i, k.add(k.v(i), N(k, "1")))})};
        f->body = k.blk({k.decl(i, N(k, "0")), k.whl(k.lt(k.v(i), k.pv(f, 1)),
                                                     {k.cl(k.range(k.v(i), k.i32, "0", "100")),
                                                      k.cl(k.le(k.v(i), k.pv(f, 1))), Clause {inv, where}},
                                                     {k.cl(k.sub(k.pv(f, 1), k.v(i)))}, body)});
        k.done(f);
        return where;
    };
    lc("hr_loop",
       "the loop holds **pp (the object *pp designates on entry is an automatic place) and reads it in the invariant",
       [=](Kit& k)
       {
           make_loop(k, false);
       });
    lc("hr_loop_value", "the invariant claims **pp == 0, the body increments it: invariant-preserved at the clause",
       [=](Kit& k)
       {
           k.fail("invariant-preserved", make_loop(k, true));
       });

    // ---- a written atom on **pp in an invariant (item 12) ----
    // int walk2(void) ensures(r == 1) { int x = 1; int *p = &x; int **pp = &p; int i = 0;
    //   while (i < 3) invariant i <= 3 && pp == &p && *pp == &x && owns(**pp) && **pp == 1 { i += 1; } return **pp; }
    const auto make_walk2 = [=](Kit& k, bool give_away) -> SrcLoc
    {
        Function* dispose {nullptr};
        if (give_away)
        {
            dispose = k.fn("dispose", k.vd, {{"p", PT(k)}});
            k.req(dispose, owns(k, FP(k, dispose, 0), 0));
            k.trusted(dispose);
            k.done(dispose);
        }
        Function* f {k.fn("walk2", k.i32)};
        k.ens(f, k.eq_c(k.R(f), k.i32, "1"));
        auto [x, dx] = cell_init(k, f, "x", "1");
        Var* p {k.cell(f, "p", PT(k))};
        Var* pp {k.local(f, "pp", PPT(k))};
        Var* i {k.local(f, "i", k.i32)};
        const SrcLoc where {k.at()};
        const Expr* atom {k.b.atom(AtomKind::Owns, k.deref(k.pn(pp)), 0, where)};
        const Expr* inv {k.land(k.land(k.land(k.land(k.le_c(k.v(i), k.i32, "3"), k.eq(k.pn(pp), k.addr(p))),
                                              k.eq(k.deref(k.pn(pp)), k.addr(x))),
                                       atom),
                                k.eq_c(k.deref(k.deref(k.pn(pp))), k.i32, "1"))};
        std::vector<const Stmt*> inner_body;
        if (give_away)
        {
            inner_body.push_back(k.es(k.call(dispose, {k.addr(x)})));
        }
        inner_body.push_back(k.asg(i, k.add(k.v(i), N(k, "1"))));
        const Stmt* body {k.blkc(inner_body)};
        const Stmt* ret {k.ret(k.deref(k.deref(k.pn(pp))))};
        f->body = k.blkc({dx, k.decl(p, k.addr(x)), k.decl(pp, k.addr(p)), k.decl(i, N(k, "0")),
                          k.whl(k.lt(k.v(i), N(k, "3")), {k.cl(k.range(k.v(i), k.i32, "0", "3")), Clause {inv, where}},
                                {k.cl(k.sub(N(k, "3"), k.v(i)))}, body),
                          ret});
        k.done(f);
        return give_away ? body->end_loc : where;
    };
    lc("hr_written_pp",
       "owns(**pp) written in an invariant (pp a local pointer to the cell p): the atom's object is the chained read of *pp",
       [=](Kit& k)
       {
           make_walk2(k, false);
       });
    lc("hr_written_pp_gives_away",
       "an iteration gives x away: the back-edge row of the atom fails at the body's closing brace",
       [=](Kit& k)
       {
           k.fail_only("permission", make_walk2(k, true), 12U, 0U);
       });

    // ---- a member chain p->q->x and a by-value parameter's pointer member (items 3, 9) ----
    // struct L { struct L *q; int x; };   void inc(struct L *p) requires(owns(*p) && owns(p->q->x) && p->q->x < 100)
    //   ensures(owns(*p) && p->q == old(p->q) && owns(p->q->x) && p->q->x == old(p->q->x) + [1|2]) { p->q->x += 1; }
    const auto make_chain = [=](Kit& k, const char* delta) -> Function*
    {
        StructType* L {k.prog.add_struct("L", "struct L", k.at())};
        k.prog.add_member(L, "q", k.ptr(Type::structure(L)), k.at());
        k.prog.add_member(L, "x", k.i32, k.at());
        Function* f {k.fn("inc", k.vd, {{"p", k.ptr(Type::structure(L))}})};
        auto q_of = [&](Kit& kk)
        {
            return kk.deref(kk.mem(FP(kk, f, 0), 0));
        }; // p->q
        auto x_cell = [&](Kit& kk)
        {
            return kk.mem(q_of(kk), 1);
        }; // &p->q->x
        auto x_val = [&](Kit& kk)
        {
            return kk.deref(x_cell(kk));
        }; // p->q->x
        k.req(f, k.land(k.land(owns(k, FP(k, f, 0), 0), owns(k, x_cell(k), 1)), k.lt(x_val(k), N(k, "100"))));
        k.ens(f, k.land(k.land(k.land(owns(k, FP(k, f, 0), 0), k.eq(q_of(k), k.old(q_of(k)))), owns(k, x_cell(k), 1)),
                        k.eq(x_val(k), k.add(k.old(x_val(k)), N(k, delta)))));
        const Var* p {f->param_var(0)};
        const Expr* cell {k.mem(k.deref(k.mem(k.pn(p), 0)), 1)};
        f->body = k.blk({k.store(cell, k.add(k.deref(k.mem(k.deref(k.mem(k.pn(p), 0)), 1)), N(k, "1")))});
        k.done(f);
        return f;
    };
    lc("hr_chain", "owns(*p) && owns(p->q->x): the callee increments through the chain",
       [=](Kit& k)
       {
           make_chain(k, "1");
       });
    lc("hr_chain_wrong", "the same promising + 2: postcondition at the closing brace",
       [=](Kit& k)
       {
           k.fail("postcondition", make_chain(k, "2")->end_loc);
       });
    // ---- a pointer global's target: owns(gp) && owns(*gp) (item 3) ----
    // static int h = 7; static int *gp = &h; void bump(void) requires(owns(gp) && owns(*gp) && *gp < 100) ensures(owns(gp) && gp == old(gp) && owns(*gp) && *gp == old(*gp) + 1) { *gp += 1; }
    // int main(void) { bump(); check(h == [8|9]); return 0; }
    const auto make_gbump = [=](Kit& k, bool wrong) -> SrcLoc
    {
        Global* h {k.prog.add_global("h", k.i32, k.at())};
        h->init = k.num(k.i32, "7");
        h->value = "7";
        Global* gp {k.prog.add_global("gp", PT(k), k.at())};
        gp->init_kind = Global::Init::Addr;
        gp->target = h;
        auto ga = [&](Kit& kk, const Global* g)
        {
            return kk.b.global_addr(g, kk.at());
        };
        auto gread = [&](Kit& kk, const Global* g)
        {
            return kk.deref(ga(kk, g));
        };
        Function* bump {k.fn("bump", k.vd)};
        k.req(bump, k.land(k.land(k.b.global_atom(AtomKind::Owns, ga(k, gp), 0, k.at()), owns(k, gread(k, gp), 1)),
                           k.lt(k.deref(gread(k, gp)), N(k, "100"))));
        k.ens(bump, k.land(k.land(k.land(k.b.global_atom(AtomKind::Owns, ga(k, gp), 0, k.at()),
                                         k.eq(gread(k, gp), k.old(gread(k, gp)))),
                                  owns(k, gread(k, gp), 1)),
                           k.eq(k.deref(gread(k, gp)), k.add(k.old(k.deref(gread(k, gp))), N(k, "1")))));
        bump->body = k.blk({k.store(gread(k, gp), k.add(k.deref(gread(k, gp)), N(k, "1")))});
        k.done(bump);
        Function* m {k.fn("main", k.i32)};
        const Stmt* c {k.chk(k.eq_c(k.deref(ga(k, h)), k.i32, wrong ? "9" : "8"))};
        m->body = k.blk({k.es(k.call(bump, {})), c, k.ret(N(k, "0"))});
        k.done(m);
        k.prog.set_entry(m);
        return c->loc;
    };
    lc("hr_global_target",
       "owns(gp) && owns(*gp): the root of *gp is the global's cell; main learns h == 8 through gp == &h",
       [=](Kit& k)
       {
           make_gbump(k, false);
       });
    lc("hr_global_target_check", "check(h == 9) does not hold",
       [=](Kit& k)
       {
           k.fail("check", make_gbump(k, true));
       });

    // ---- a by-value parameter's pointer member: owns(*s.q) (item 9) ----
    // struct P { int *q; };  int get(struct P s) requires(owns(*s.q) && *s.q == 3) ensures(owns(*s.q) && r == [3|4]) { return *s.q; }
    const auto make_leaf = [=](Kit& k, bool wrong) -> const Stmt*
    {
        StructType* P {k.strukt("P", {{"q", PT(k)}})};
        Function* get {k.fn("get", k.i32, {{"s", k.st(P)}})};
        get->mark_address_taken(get->param_var(0));
        auto sq = [&](Kit& kk)
        {
            return kk.b.field(kk.F(get, 0), 0, kk.at());
        }; // s.q, the entry value of the leaf
        k.req(get, k.land(owns(k, sq(k), 0), k.eq_c(k.deref(sq(k)), k.i32, "3")));
        k.ens(get, k.land(owns(k, sq(k), 0), k.eq_c(k.R(get), k.i32, wrong ? "4" : "3")));
        const Stmt* ret {k.ret(k.deref(k.leaf(k.addr(get->param_var(0)), 0)))};
        get->body = k.blkc({ret});
        k.done(get);
        return ret;
    };
    lc("hr_leaf",
       "owns(*s.q) of a by-value parameter's pointer member: the atom's root is the leaf formal, the body reads the copy's member",
       [=](Kit& k)
       {
           make_leaf(k, false);
       });
    lc("hr_leaf_wrong", "the ensures says 4, the callee returns *s.q == 3: postcondition at the return",
       [=](Kit& k)
       {
           k.fail("postcondition", make_leaf(k, true)->loc);
       });
}

void define_heap_read_place_rejections()
{
    auto PT = [](Kit& k)
    {
        return k.ptr(k.i32);
    };
    auto PPT = [](Kit& k)
    {
        return k.ptr(k.ptr(k.i32));
    };
    auto owns = [](Kit& k, const Expr* place, int amount)
    {
        return k.b.atom(AtomKind::Owns, place, amount, k.at());
    };
    // owns(*r.q) of a struct result's pointer member: an atom with no requires partner (question 106: M4.5d2)
    rc("hr_rej_result_member_atom", "without a `requires` atom",
       [=](Kit& k)
       {
           StructType* S {k.strukt("S", {{"q", PT(k)}})};
           Function* f {k.fn("f", Type::structure(S), {{"p", PT(k)}})};
           k.ens(f, owns(k, k.b.field(k.R(f), 0, k.at()), 0));
           f->body = k.blk({});
           k.done(f);
       });
    // owns(**r) through the result of type int ** (M4.5d2)
    rc("hr_rej_result_double_deref", "without a `requires` atom",
       [=](Kit& k)
       {
           Function* f {k.fn("f", PPT(k))};
           k.ens(f, owns(k, k.deref(k.R(f)), 0));
           f->body = k.blk({k.ret(k.nul(PPT(k)))});
           k.done(f);
       });
}

} // namespace
