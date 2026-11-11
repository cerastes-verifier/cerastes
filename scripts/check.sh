#!/usr/bin/env bash
# Part of cerastes, under the Apache License v2.0 with LLVM Exceptions.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#
# Build, lint and test everything.  Must be green at the end of every session.
# Requires a prior scripts/bootstrap.sh.
#
# CERASTES_CHECK_BASE unset or empty (the default, and what every session ends with): the full
# check.  CI's pull requests and pushes set it to the commit they are compared with (decision 0016
# item 14): the PR tier, in which both test runs below make Silicon runs only for the tests changed
# since that commit, the doc examples and a fixed sample; the nightly CI run is the full check.
#
# CERASTES_LONG unset or 0 (the default): the long-running tests under tests/long are skipped, counted in the
# test summary, and the last line says so.  CERASTES_LONG=1 runs them (decision 0057); any other value, the
# empty string included, is an error, and so is 1 together with the PR tier, which never runs them.

set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"

step() { printf '\n==== %s\n' "$*"; }

pr_base="${CERASTES_CHECK_BASE:-}"
if [[ -n "$pr_base" ]]; then
  printf 'check.sh: the PR tier, against %s (decision 0016 item 14)\n' "$pr_base"
fi

long_tier="${CERASTES_LONG-0}"
case "$long_tier" in
  0 | 1) ;;
  *)
    echo "check.sh: CERASTES_LONG=$long_tier is not 0 or 1 (unset it, or set 1 to run tests/long: decision 0057)" >&2
    exit 1
    ;;
esac
if [[ -n "$pr_base" && "$long_tier" == 1 ]]; then
  echo "check.sh: CERASTES_LONG=1 cannot be combined with the PR tier, which never runs tests/long (decision 0057)" >&2
  exit 1
fi
long_flag=""
if [[ "$long_tier" == 1 ]]; then
  long_flag="--long"
fi

step "edg tree"
python3 scripts/edg_tree.py check

step "vlibc tree"
# decision 0046: third_party/vlibc is clean, at its gitlink, a commit of vlibc's main; an override only off main
python3 scripts/vlibc_tree.py check

step "build"
scripts/bootstrap.sh --build-only

step "lint"
for script in scripts/*.sh; do
  bash -n "$script"
done
command -v shellcheck >/dev/null || { echo "check.sh: shellcheck missing; run scripts/bootstrap.sh" >&2; exit 1; }
shellcheck scripts/*.sh
# the tool code keeps the owner's Boost style (decision 0051), checked with the pinned clang-format
scripts/format.sh --check
python3 -m py_compile scripts/*.py
python3 scripts/run_tests.py --check-headers
python3 scripts/check_doc_examples.py
# the contracts vlibc's headers trust are the committed list (decision 0046 item 8): a pin move that changes one
# fails here until tests/vlibc/trusted_contracts.txt is changed and audited
vlibc_dir="$(python3 scripts/vlibc_tree.py resolve)"
python3 scripts/vlibc_contracts.py check --include-dir "${vlibc_dir#* }/include/vlibc" --list tests/vlibc/trusted_contracts.txt
# the mutants of the must-verify tests generate, and the equivalence list is current
# (the full mutation run, with the verifier, is an acceptance step)
python3 scripts/mutate.py --validate

step "edg configuration"
scripts/check_edg_config.sh
scripts/check_edg_options.sh
# vlibc's headers compiled as ordinary (non-system) headers with the driver's flags, no diagnostic (decision 0046 item 3)
scripts/check_vlibc_headers.sh

step "harness self-test"
# The self-tests that need the real front end must fail, not skip, here.
export CERASTES_REQUIRE_E2E=1
python3 scripts/test_run_tests.py
python3 scripts/test_walker_oracle.py
python3 scripts/test_lint_viper.py
python3 scripts/test_mutate.py
python3 scripts/test_vlibc_contracts.py
python3 scripts/test_object_identity.py

step "tests"
python3 scripts/run_tests.py --pr-base="$pr_base" ${long_flag:+"$long_flag"}

step "viper lint"
# the Viper that the tests above left in build/vpr (invariant 4)
python3 scripts/lint_viper.py

step "vir self-test"
# VIR's own cases (typing rejections and lowering cases), through Silicon and the lint;
# a directory of its own, emptied first: build/vpr belongs to the harness
# The case count is pinned twice: the tool must state at least VIR_CASES_MIN cases (a smaller
# set must not pass), and the harness must pass exactly the number it stated (a case lost on
# the way, or a file that is not run, must not pass either).
VIR_CASES_MIN=1765
rm -rf build/vir-selftest
mkdir -p build/vir-selftest
vir_out="$(build/cerastes-vir-selftest build/vir-selftest)"
printf '%s\n' "$vir_out"
vir_cases="$(printf '%s\n' "$vir_out" | sed -n 's/^cerastes-vir-selftest: \([0-9][0-9]*\) lowering cases written to .*, 0 problems$/\1/p')"
if [[ ! "$vir_cases" =~ ^[0-9]+$ ]] || (( vir_cases < VIR_CASES_MIN )); then
  echo "check.sh: the VIR self-test stated ${vir_cases:-no number of} cases; at least $VIR_CASES_MIN are expected" >&2
  exit 1
fi
python3 scripts/run_tests.py --pr-base="$pr_base" --expect-passed "$vir_cases" build/vir-selftest
python3 scripts/lint_viper.py build/vir-selftest/*.vpr

step "walker oracle"
# every walker golden agrees with an independent print of the IL (--il_display)
python3 scripts/walker_oracle.py

step "docs"
scripts/build_docs.sh
python3 scripts/check_doc_examples.py --html build/doc

python3 scripts/edg_tree.py describe --verify-build
python3 scripts/vlibc_tree.py describe
if [[ -n "$pr_base" ]]; then
  printf '\ncheck.sh: green in the PR tier (not the full check)\n'
else
  printf '\ncheck.sh: all green\n'
fi
if [[ "$long_tier" == 1 ]]; then
  printf 'check.sh: long tier: ran (CERASTES_LONG=1)\n'
else
  printf 'check.sh: long tier: skipped (set CERASTES_LONG=1 to run tests/long)\n'
fi
