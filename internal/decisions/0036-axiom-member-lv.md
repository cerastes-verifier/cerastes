# 0036 — `member_lv`: a member lives exactly as long as its enclosing object (the `Block` domain's axioms, M4.5b1a)

- **Status:** accepted for M4.5b1a (2026-10-28). Record 0028 item 2 (owner, 2026-10-27, after S24 and S26) chose the
  `Block` domain with four axioms and said that each gets its own record and lint tag when M4.5b1 implements it.
- **Milestone:** M4.5b1a (2026-10-28); emitted from M4.5b1a by every program that has a pointer type or an address-taken
  local (record 0032 item 5).
- **Part of:** record 0028 item 2's domain; the other axioms are 0033, 0034, 0035.

## Decision

```
axiom member_lv { forall r: Ref, k: Int :: { lvloc(member(r, k)) } lvloc(member(r, k)) == lvloc(block_of(r)) }
```

Liveness is one ghost cell per top-level block (0028 item 3a, owner after S26): a sub-object's liveness cell is its
enclosing object's, so a struct pointer's `owns` carries one fraction, not one per member, and a member pointer is
dangling exactly when its object is. Never triggered in M4.5b1a (no `member` term before M4.5c).

**The trigger.** 0028 gives `{ lvloc(member(r, k)) }`, and the 43 fixtures regenerated in S26 final use it; the 12
`s25_*` fixtures still carried an earlier `{ member(r, k) }`. With this record the 12 use 0028's trigger too (no other
change, every outcome unchanged), so the fixtures and the encoder have one prelude.

Triggered only by `lvloc(member(r, k))`: an `lvloc` of a member block.

**The model** (0028 item 2): `Ref` is the free term algebra `null` | `loc(Block, Int)`, `Block` is the allocated blocks
| `member(Ref, Int)`; `block_of`, `offset_of`, `parent_of`, `index_of` are the projections; `lvloc(b)` is `loc(b, -2)`
for an allocated block and `lvloc(block_of(r))` for `member(r, k)`, by recursion on the term; `len` is arbitrary. Every
one of the four axioms holds in it for every argument, with no guard, and every trusted method (`alloc_T`, record 0032
item 4; later `program_start` and vlibc's allocations) states only facts true of a fresh allocated block in it. **Runs**
(`il-inventory.md`, S24, S26 final): `e2_consistency_terms_fail.vpr` instantiates the terms of every axiom and then
asserts `false`, failing with exactly that one error for each of the 16 subsets of the four axioms;
`e2_consistency_calls_fail.vpr` does the same after every trusted method; `vacuity2.py` found all 139 points (method
starts, ends, after every trusted call) of the regenerated fixtures reachable.

**Lint** (record 0032 item 11): the line before the axiom is `// cerastes:axiom-ok 0036`, and the domain is preceded by
`// cerastes:domain-ok 0028`; `lint_viper.py` refuses an axiom without its tag or with a tag naming a record that does
not exist, and any other domain or axiom.

## Consequences

- One of exactly four axioms in cerastes's Viper (with 0033–0036); any other is refused by the lint.
- Changing it changes the fixtures in `tests/viper-smoke/memory/`, which change only with a record.
