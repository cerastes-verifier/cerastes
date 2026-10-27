// Part of cerastes, under the Apache License v2.0 with LLVM Exceptions.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#include "emit.h"

#include "posmap.h"
#include "prelude.h"

namespace cerastes::viper {

namespace {

using low::BinOp;
using low::E;
using low::LExpr;

const char* bin_text(BinOp op) {
  switch (op) {
    case BinOp::Add: return "+";
    case BinOp::Sub: return "-";
    case BinOp::Mul: return "*";
    case BinOp::Lt: return "<";
    case BinOp::Le: return "<=";
    case BinOp::Gt: return ">";
    case BinOp::Ge: return ">=";
    case BinOp::Eq: return "==";
    case BinOp::Ne: return "!=";
    case BinOp::And: return "&&";
    case BinOp::Or: return "||";
    case BinOp::Implies: return "==>";
  }
  return "?";
}

// Fully parenthesized; a negative literal is parenthesized too.
std::string expr(const E& e, bool top = false) {
  if (!e) return "true";
  switch (e->kind) {
    case LExpr::Kind::IntLit: return (!e->text.empty() && e->text[0] == '-') ? "(" + e->text + ")" : e->text;
    case LExpr::Kind::BoolLit: return e->bool_value ? "true" : "false";
    case LExpr::Kind::Var: return e->text;
    case LExpr::Kind::Un: {
      std::string s = std::string(e->un == low::UnOp::Neg ? "-" : "!") + expr(e->a);
      return top ? s : "(" + s + ")";
    }
    case LExpr::Kind::Bin: {
      std::string s = expr(e->a) + " " + bin_text(e->bin) + " " + expr(e->b);
      return top ? s : "(" + s + ")";
    }
    case LExpr::Kind::Ite: {
      std::string s = expr(e->a) + " ? " + expr(e->b) + " : " + expr(e->c);
      return top ? s : "(" + s + ")";
    }
    case LExpr::Kind::ModPow2: {  // decision 0026 item 7: `a % 2^k`, the divisor a positive power-of-two literal
      std::string s = expr(e->a) + " % " + e->text;
      return top ? s : "(" + s + ")";
    }
    case LExpr::Kind::DivPow2: {  // decision 0026 item 7: `a \ 2^k`, the divisor a positive power-of-two literal
      std::string s = expr(e->a) + " \\ " + e->text;
      return top ? s : "(" + s + ")";
    }
    case LExpr::Kind::Fn: {
      std::string s = e->text + "(";
      for (size_t i = 0; i < e->args.size(); ++i) s += (i != 0 ? ", " : "") + expr(e->args[i], true);
      return s + ")";
    }
  }
  return "?";
}

class Printer {
 public:
  explicit Printer(const EmitOptions& o) { raw(o.header); }

  void raw(const std::string& text) {
    for (char c : text) {
      out_ += c;
      if (c == '\n') ++line_;
    }
  }

  // One line of Viper, with the checks that a failure on it can mean.
  void line(int indent, const std::string& text, const std::vector<low::Check>& checks = {}) {
    out_ += std::string(static_cast<size_t>(indent) * 2, ' ') + text + "\n";
    for (const low::Check& c : checks) {
      MapEntry m;
      m.vpr_line = line_;
      m.viper_id = c.viper_id;
      m.category = c.meta.category;
      m.file = c.meta.loc.file;
      m.line = c.meta.loc.line;
      m.column = c.meta.loc.column;
      m.message = c.meta.message;
      if (c.meta.has_note) {
        m.has_note = true;
        m.note_file = c.meta.note_loc.file;
        m.note_line = c.meta.note_loc.line;
        m.note_text = c.meta.note_text;
      }
      map_.push_back(std::move(m));
    }
    ++line_;
  }

  std::string& vpr() { return out_; }
  const std::vector<MapEntry>& map() const { return map_; }

 private:
  std::string out_;
  int line_ = 1;
  std::vector<MapEntry> map_;
};

std::string decl(const low::LVar& v) { return v.name + ": " + (v.is_bool ? "Bool" : "Int"); }

void stmts(Printer& p, const std::vector<low::LStmt>& body, int indent);

void stmt(Printer& p, const low::LStmt& s, int indent) {
  using K = low::LStmt::Kind;
  switch (s.kind) {
    case K::Assign: p.line(indent, s.name + " := " + expr(s.expr, true)); break;
    case K::Assert: p.line(indent, "assert " + expr(s.expr, true), {s.check}); break;
    case K::Call: {
      std::string args;
      for (size_t i = 0; i < s.args.size(); ++i) args += (i != 0 ? ", " : "") + expr(s.args[i], true);
      std::vector<low::Check> checks = {s.check};
      if (s.has_termination) checks.push_back(s.termination);
      p.line(indent, (s.target.empty() ? "" : s.target + " := ") + s.name + "(" + args + ")", checks);
      break;
    }
    case K::If:
      p.line(indent, "if (" + expr(s.expr, true) + ") {");
      stmts(p, s.then_body, indent + 1);
      if (!s.else_body.empty()) {
        p.line(indent, "} else {");
        stmts(p, s.else_body, indent + 1);
      }
      p.line(indent, "}");
      break;
    case K::While:
      p.line(indent, "while (" + expr(s.expr, true) + ")");
      for (const low::LInvariant& inv : s.invariants) p.line(indent + 1, "invariant " + expr(inv.expr, true), inv.checks);
      {
        std::string measure;
        for (size_t i = 0; i < s.decreases.size(); ++i) measure += (i != 0 ? ", " : "") + expr(s.decreases[i], true);
        p.line(indent + 1, "decreases " + measure, {s.decreases_check});  // one line: Silicon reports it at the measure
      }
      p.line(indent, "{");
      stmts(p, s.then_body, indent + 1);
      p.line(indent, "}");
      break;
    case K::Goto: p.line(indent, "goto " + s.name); break;
    case K::Label: p.line(indent, "label " + s.name); break;
    case K::Comment: p.line(indent, "// " + s.name); break;
    case K::Block: stmts(p, s.stmts, indent); break;
  }
}

void stmts(Printer& p, const std::vector<low::LStmt>& body, int indent) {
  for (const low::LStmt& s : body) stmt(p, s, indent);
}

void method(Printer& p, const low::LMethod& m) {
  p.line(0, "");
  if (!m.comment.empty()) p.line(0, "// " + m.comment);
  if (!m.has_body) p.line(0, "// cerastes:trusted-contract " + m.trusted_reason);
  std::string head = "method " + m.name + "(";
  for (size_t i = 0; i < m.formals.size(); ++i) head += (i != 0 ? ", " : "") + decl(m.formals[i]);
  head += ")";
  if (!m.results.empty()) {
    head += " returns (";
    for (size_t i = 0; i < m.results.size(); ++i) head += (i != 0 ? ", " : "") + decl(m.results[i]);
    head += ")";
  }
  p.line(0, head);
  for (const low::LClause& c : m.requires_) p.line(1, "requires " + expr(c.expr, true), c.checks);
  for (const low::LClause& c : m.ensures_) p.line(1, "ensures " + expr(c.expr, true), c.checks);
  // Every method has a `decreases` clause (decision 0013): Silicon checks termination only where one is present.
  if (m.decreases_assumed) {
    p.line(1, "decreases _");  // a trusted contract is assumed to terminate (the trusted-contract tag above says so)
  } else {
    std::string measure;
    for (size_t i = 0; i < m.decreases_.size(); ++i) measure += (i != 0 ? ", " : " ") + expr(m.decreases_[i], true);
    std::vector<low::Check> checks;
    if (!m.decreases_.empty()) checks.push_back(m.decreases_check);
    p.line(1, "decreases" + measure, checks);
  }
  if (!m.has_body) return;
  p.line(0, "{");
  for (const low::LVar& v : m.locals) p.line(1, "var " + decl(v));
  stmts(p, m.body, 1);
  p.line(0, "}");
}

}  // namespace

Emitted emit(const low::LProgram& program, const EmitOptions& options) {
  Printer p(options);
  p.line(0, "// cerastes: Viper emitted from VIR (internal/encoding.md); do not edit");
  const std::string prelude = prelude_text(program.prelude_used);
  p.raw(prelude);
  for (const low::LMethod& m : program.methods) method(p, m);
  Emitted e;
  e.vpr = std::move(p.vpr());
  e.map = render_map(p.map(), &e.problems);
  return e;
}

}  // namespace cerastes::viper
