// (included by selftest.cpp) `cerastes::not_invalidating`, Part N (decision 0058 item 2, M4.5d2b2d): the `f__wf` row `prior ==> (g_req ==> g_ens)` for a marked
// parameter (a pair of guards that differ and are not one expression: a cell atom, then a block site), its absence for an unmarked parameter and for one guard
// written twice, the postcondition message of a clause the encoder spliced an equality into, and the backstop of check_program.  The spliced equalities
// themselves (the loaded pointers) are tests of the encoder: tests/memory/not_invalidating_*.c.
namespace {

void define_not_invalidating()
{
    auto PT = [](Kit& k)
    {
        return k.ptr(k.i32);
    };
    auto N = [](Kit& k, const char* v)
    {
        return k.num(k.i32, v);
    };
    auto FP = [](Kit& k, Function* f, int i)
    {
        return k.b.text(k.F(f, i), f->params[static_cast<size_t>(i)].name);
    };
    auto owns = [=](Kit& k, Function* f, int i, int amount)
    {
        return k.b.atom(AtomKind::Owns, FP(k, f, i), amount, k.at());
    };
    auto block = [=](Kit& k, Function* f, int i)
    {
        return k.b.atom(AtomKind::OwnsBlock, FP(k, f, i), -1, k.at());
    };
    // guards over the formal c: 0 `c != 0`, 1 `c < 0 || c > 0` (the same set, another expression), 2 `c > 0`, 3 none
    auto guard = [=](Kit& k, Function* f, int shape, const Expr* then)
    {
        const Expr* c {k.F(f, 0)};
        switch (shape)
        {
            case 0:
                return k.cond(k.ne(c, N(k, "0")), then, k.yes());
            case 1:
                return k.cond(k.lor(k.lt(c, N(k, "0")), k.gt(c, N(k, "0"))), then, k.yes());
            case 2:
                return k.cond(k.gt(c, N(k, "0")), then, k.yes());
            default:
                return then;
        }
    };
    // void f(int c, int *p [not_invalidating]) requires(G1(owns(*p)))  ensures(G2(owns(*p)))        (trusted: only f__wf is checked)
    // the same with owns_block(p) for `blocks`.  Where the ensures atom is: the failure is the row at that atom.
    const auto pair = [=](Kit& k, bool blocks, bool marked, int req_guard, int ens_guard) -> SrcLoc
    {
        Function* f {k.fn("f", k.vd, {{"c", k.i32}, {"p", PT(k)}})};
        f->params[1U].not_invalidating = marked;
        const Expr* rq {blocks ? block(k, f, 1) : owns(k, f, 1, 0)};
        const Expr* en {blocks ? block(k, f, 1) : owns(k, f, 1, 0)};
        k.req(f, guard(k, f, req_guard, rq));
        k.ens(f, guard(k, f, ens_guard, en));
        k.trusted(f);
        k.done(f);
        return en->loc;
    };
    lc("ni_row_cells_equivalent",
       "requires(c != 0 ? owns(*p) : 1) ensures(c < 0 || c > 0 ? owns(*p) : 1) with p not_invalidating: the guards are one set in two expressions, the new row holds",
       [=](Kit& k)
       {
           pair(k, false, true, 0, 1);
       });
    lc("ni_row_cells_guard_narrower",
       "the ensures guard is c > 0 and the requires guard c != 0: the contract takes *p for c < 0 and does not return it (the new row, permission at the ensures atom)",
       [=](Kit& k)
       {
           k.fail("permission", pair(k, false, true, 0, 2), 1U, "", "not_invalidating");
       });
    lc("ni_row_cells_requires_unguarded",
       "requires(owns(*p)) ensures(c != 0 ? owns(*p) : 1): the contract takes *p whatever c is (the new row, permission at the ensures atom)",
       [=](Kit& k)
       {
           k.fail("permission", pair(k, false, true, 3, 0), 1U, "", "not_invalidating");
       });
    lc("ni_row_cells_unmarked",
       "the same contract with p unmarked: no new row, and the partner row of today holds (a returned atom was taken)",
       [=](Kit& k)
       {
           pair(k, false, false, 0, 2);
       });
    lc("ni_row_cells_same_guard",
       "the same guard on both sides, with p marked: the row asks nothing (one expression)",
       [=](Kit& k)
       {
           pair(k, false, true, 0, 0);
       });
    lc("ni_row_blocks_equivalent",
       "requires(c != 0 ? owns_block(p) : 1) ensures(c < 0 || c > 0 ? owns_block(p) : 1) with p not_invalidating: the block sites' guards are one set, the new row holds",
       [=](Kit& k)
       {
           pair(k, true, true, 0, 1);
       });
    lc("ni_row_blocks_guard_narrower",
       "the same with the ensures guard c > 0: the contract takes the block for c < 0 and does not return it (the new row, permission at the ensures site)",
       [=](Kit& k)
       {
           k.fail("permission", pair(k, true, true, 0, 2), 1U, "", "not_invalidating");
       });
    lc("ni_row_blocks_unmarked",
       "the same block contract with p unmarked: no new row",
       [=](Kit& k)
       {
           pair(k, true, false, 0, 2);
       });

    // void f(int n, int *p [not_invalidating]) requires(owns(*p)) ensures(owns(*p)) ensures(n == 0 | n == n) { }      with `added` set on the last clause
    // (what the encoder splices in: `p->q == old(p->q)`): the postcondition of a clause that holds, and the message of one that does not
    const auto added = [=](Kit& k, bool holds) -> SrcLoc
    {
        Function* f {k.fn("f", k.vd, {{"n", k.i32}, {"p", PT(k)}})};
        f->params[1U].not_invalidating = true;
        k.req(f, owns(k, f, 1, 0));
        k.ens(f, owns(k, f, 1, 0));
        k.ens(f, holds ? k.eq(k.F(f, 0), k.F(f, 0)) : k.eq(k.F(f, 0), N(k, "0")));
        f->ensures_.back().added = "p->q == old(p->q)";
        f->body = k.blk({});
        k.done(f);
        return f->end_loc;
    };
    lc("ni_added_holds", "a clause with an added equality that holds: no postcondition error",
       [=](Kit& k)
       {
           added(k, true);
       });
    lc("ni_added_message",
       "the postcondition of a clause with an added equality fails: the message names the equality and the marked parameter",
       [=](Kit& k)
       {
           k.fail("postcondition", added(k, false), 1U, "",
                  "(with `p->q == old(p->q)`, which `cerastes::not_invalidating` on `p` adds)");
       });
}

// the backstop of check_program: a marker the encoder would have rejected
void define_not_invalidating_rejections()
{
    auto PT = [](Kit& k)
    {
        return k.ptr(k.i32);
    };
    auto FP = [](Kit& k, Function* f, int i)
    {
        return k.b.text(k.F(f, i), f->params[static_cast<size_t>(i)].name);
    };
    auto owns = [=](Kit& k, Function* f, int i, int amount)
    {
        return k.b.atom(AtomKind::Owns, FP(k, f, i), amount, k.at());
    };
    // shape 0: a non-pointer, 1: beside ref_to_uninit, 2: a requires atom with no ensures partner, 3: an ensures atom with no requires partner
    const auto marked = [=](Kit& k, int shape)
    {
        Function* f {k.fn("f", k.vd, {{"p", shape == 0 ? k.i32 : PT(k)}})};
        f->params[0U].not_invalidating = true;
        if (shape == 1)
        {
            f->params[0U].init = InitMarker::RefToUninit;
        }
        if (shape == 0)
        {
            k.req(f, k.yes());
        }
        else if (shape == 3)
        {
            k.req(f, k.yes());
            k.ens(f, owns(k, f, 0, 0));
        }
        else
        {
            k.req(f, owns(k, f, 0, 0));
            k.ens(f, shape == 1 ? owns(k, f, 0, 0) : k.yes());
        }
        k.trusted(f);
    };
    rc("ni_backstop_non_pointer", "is marked `not_invalidating` but is not a pointer",
       [=](Kit& k)
       {
           marked(k, 0);
       });
    rc("ni_backstop_beside_init", "is marked `not_invalidating` beside an initialization marker",
       [=](Kit& k)
       {
           marked(k, 1);
       });
    rc("ni_backstop_takes_without_returning", "an object through it without returning it",
       [=](Kit& k)
       {
           marked(k, 2);
       });
    rc("ni_backstop_gives_without_taking", "an object through it without taking it",
       [=](Kit& k)
       {
           marked(k, 3);
       });
}

} // namespace
