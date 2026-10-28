# 0033 — `loc_inv`: a slot determines its block and its offset (the `Block` domain's axioms, M4.5b1a)

- **Status:** accepted for M4.5b1a (2026-10-28). Record 0028 item 2 (owner, 2026-10-27, after S24 and S26) chose the
  `Block` domain with four axioms and said that each gets its own record and lint tag when M4.5b1 implements it.
- **Milestone:** M4.5b1a (2026-10-28); emitted from M4.5b1a by every program that has a pointer type or an address-taken
  local (record 0032 item 5).
- **Part of:** record 0028 item 2's domain; the other axioms are 0034, 0035, 0036.

## Decision

```
axiom loc_inv { forall b: Block, i: Int :: { loc(b, i) } block_of(loc(b, i)) == b && offset_of(loc(b, i)) == i }
```

It makes `loc` injective. **Needed** (S24 ablation): with it removed, 10 of S24's 19 files change verdict (swap, escape,
`malloc_free`, struct, `ptr_diff`, `array_sum` and its twin, `double_free_fail`, …). It gives `WF` of every pointer
built by `loc` (an allocation's result, later `p + i` and `&p->y`), the block and offset of a derived pointer, and the
injectivity Silicon requires of a quantified permission's receiver (`qp.not.injective`, from M4.5e1). In M4.5b1a it
proves that two cells are different objects (`&x != &y`) and that a pointer to a cell is at offset 0 of a one-slot block
(the one-past-the-end row of record 0032 item 7).

Triggered only by `loc(b, i)`: every term the encoder builds.

**The model** (0028 item 2): `Ref` is the free term algebra `null` | `loc(Block, Int)`, `Block` is the allocated blocks
| `member(Ref, Int)`; `block_of`, `offset_of`, `parent_of`, `index_of` are the projections; `lvloc(b)` is `loc(b, -2)`
for an allocated block and `lvloc(block_of(r))` for `member(r, k)`, by recursion on the term; `len` is arbitrary. Every
one of the four axioms holds in it for every argument, with no guard, and every trusted method (`alloc_T`, record 0032
item 4; later `program_start` and vlibc's allocations) states only facts true of a fresh allocated block in it. **Runs**
(`il-inventory.md`, S24, S26 final): `e2_consistency_terms_fail.vpr` instantiates the terms of every axiom and then
asserts `false`, failing with exactly that one error for each of the 16 subsets of the four axioms;
`e2_consistency_calls_fail.vpr` does the same after every trusted method; `vacuity2.py` found all 139 points (method
starts, ends, after every trusted call) of the regenerated fixtures reachable.

**Lint** (record 0032 item 11): the line before the axiom is `// cerastes:axiom-ok 0033`, and the domain is preceded by
`// cerastes:domain-ok 0028`; `lint_viper.py` refuses an axiom without its tag or with a tag naming a record that does
not exist, and any other domain or axiom.

## Consequences

- One of exactly four axioms in cerastes's Viper (with 0033–0036); any other is refused by the lint.
- Changing it changes the fixtures in `tests/viper-smoke/memory/`, which change only with a record.
