# 0002 — The test output filter does not normalize `**possible non-file-scope ptr** (0x…)`

- **Labels:** `bug`, `infrastructure`
- **Found:** M2 (2026-10-14), recording the fork series' `--il_display` tests
- **Status:** fixed. Branch `0002` in the fork clone is commit `6129554087` on upstream
  `main` `034385ba0a`, the fork series commit FILT (`86b5465978`) cherry-picked
  (`-x`)
- **Upstream issue:** Filed and a PR submitted

## Summary

`il_display` prints a pointer to an entity outside the file-scope memory
region with its raw address. One example is the name of a function's
parameter, in the `param-type` entry of the function's type:

```
name:                    **possible non-file-scope ptr** (0x7c74a620a0): "x"
```

The address changes from run to run. The test output filter
(`dev_tools/pylibs/edgtest/filter/__init__.py`, `_REGEX_REPLACEMENTS`)
normalizes the other addresses (`@…`, `@0x…`, `.text+0x…`) but not this one.
So the recorded output of any test that displays such an IL never matches
twice.

## Reproducer

```c++
int f(int x)
{
  return x;
}
```

```sh
cpfe --clang_version=210000 --c++17 --il_display f.cpp | grep -m1 "possible non-file-scope ptr"
```

Two runs of the same command:

```
name:                    **possible non-file-scope ptr** (0x766ea620a0): "x"
name:                    **possible non-file-scope ptr** (0x7626a620a0): "x"
```

Passed through every replacement in `GLOBAL_REGEX_REPLACEMENTS`:

**Actual:** the line is unchanged, so the address stays in the filtered
output.

```
name:                    **possible non-file-scope ptr** (0x766ea620a0): "x"
```

**Expected:** the address is normalized like the others.

```
name:                    **possible non-file-scope ptr** (0xXX): "x"
```

## Confirmation

The replacements were applied with `re.sub` in table order to a line taken
from upstream's `il_display`. With the filter of `034385ba0a` the address
stays. With the filter of branch `0002` it becomes `(0xXX)`. The fork series'
`--il_display` tests in `tests/tests/changes/` are recorded with this filter
and match from run to run.

## Fix

Branch `0002`, commit `6129554087` "Normalize the addresses printed for
non-file-scope pointers in test output". It adds one entry to
`_REGEX_REPLACEMENTS`:

```python
  # IL display of a name that is not in the file scope memory region.
  r'\*\*possible non-file-scope ptr\*\* \(0x[0-9a-f]+\)':
      '**possible non-file-scope ptr** (0xXX)',
```

No test of its own: the filter is exercised by every recorded test that uses
`--il_display`.

## Impact on cerastes

The fork series records IL-display tests for the attributes it attaches, and
they could not be recorded stably without this.
