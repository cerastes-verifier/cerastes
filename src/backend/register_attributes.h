// Part of cerastes, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// Registers the `cerastes` attribute namespace with the EDG front end
// (decisions 0006 and 0007).  Call register_cerastes_annotations() before
// edg_main().
#pragma once

namespace cerastes {

// Registers the closed namespace `cerastes` and its eight attributes:
// requires, ensures, invariant, check, fold, unfold, predicate, pure.  The
// table is static, so this may be called more than once.
void register_cerastes_annotations();

// Verify mode only (decision 0017 item 2): puts a recorder in front of EDG's handlers of the diagnostic pragmas
// (pk_diag_suppress/remark/warning/error/once/default and pk_diagnostic), by replacing their entries of EDG's
// `function_pointers` table; each handler then runs unchanged.  Call before edg_main(); the encoder rejects every recorded
// pragma in a verified file (diagnostic_pragma_in_verify).
void install_diag_pragma_recorder();

}  // namespace cerastes
