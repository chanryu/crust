#!/usr/bin/env python3
"""
Generate one Rust-like sum type in C++ (std::variant wrapper + named cases)
from a single-enum JSON specification.

Shorthand JSON supported:
- cases:
 - "Quit"
 - {"Move": [["x","int"], ["y","int"]]}
- fields:
 - ["x","int"]
 - ["x","int","42"]  # default (raw C++)

Features:
- constexpr constructors for case structs and wrapper type
- hides <EnumName>Tags inside namespace detail (within the target namespace if provided)
- JSON Schema included and validated *after normalization*
 (uses jsonschema if installed; otherwise strict manual validation)

This generator emits ONLY the sum type. Keep your generic match wrapper in a shared header.
"""

from __future__ import annotations

import json
import sys
from pathlib import Path
from typing import Any, Dict, List, Optional, Set, Tuple


# Normalized schema (post-shorthand expansion)
SCHEMA_NORMALIZED: Dict[str, Any] = {
   "$schema": "https://json-schema.org/draft/2020-12/schema",
   "$id": "https://example.local/sumtype.normalized.schema.json",
   "type": "object",
   "additionalProperties": False,
   "properties": {
       "namespace": {"type": "string"},
       "includes": {"type": "array", "items": {"type": "string"}},
       "name": {"type": "string"},
       "cases": {
           "type": "array",
           "minItems": 1,
           "items": {
               "type": "object",
               "additionalProperties": False,
               "properties": {
                   "name": {"type": "string"},
                   "fields": {
                       "type": "array",
                       "items": {
                           "type": "object",
                           "additionalProperties": False,
                           "properties": {
                               "name": {"type": "string"},
                               "type": {"type": "string"},
                               "default": {"type": "string"},
                           },
                           "required": ["name", "type"],
                       },
                   },
               },
               "required": ["name"],
           },
       },
   },
   "required": ["name", "cases"],
}


def eprint(*args: Any) -> None:
    print(*args, file=sys.stderr)


def cpp_ident(name: str) -> str:
   if not isinstance(name, str):
       raise ValueError(f"Identifier must be a string, got {type(name).__name__}")
   if not name or not (name[0].isalpha() or name[0] == "_"):
       raise ValueError(f"Invalid C++ identifier: {name!r}")
   for ch in name[1:]:
       if not (ch.isalnum() or ch == "_"):
           raise ValueError(f"Invalid C++ identifier: {name!r}")
   return name


# ----------------------------
# Shorthand normalization
# ----------------------------

def normalize_field(field: Any, ctx: str) -> Dict[str, Any]:
   """
   Return normalized field object:
     {"name": <str>, "type": <str>, "default": <optional str>}
   """
   if isinstance(field, list):
       if len(field) not in (2, 3):
           raise ValueError(f"{ctx}: field array must be [name,type] or [name,type,default]")
       out: Dict[str, Any] = {"name": field[0], "type": field[1]}
       if len(field) == 3:
           out["default"] = field[2]
       return out

   raise ValueError(f"{ctx}: invalid field form (expected array)")


def normalize_case(case: Any, idx: int) -> Dict[str, Any]:
   """
   Return normalized case object:
     {"name": <str>, "fields": [<normalized fields>]}  # fields may be omitted/empty
   """
   ctx = f"cases[{idx}]"

   # Case as string: "Quit"
   if isinstance(case, str):
       return {"name": case}

   if isinstance(case, dict):
       # Single-key shorthand: {"Move": [...]}
       if len(case) == 1:
           (k, v), = case.items()
           out = {"name": k}
           if v is None:
               return out
           if not isinstance(v, list):
               raise ValueError(f"{ctx}: shorthand case value must be a list of fields")
           if v:
               out["fields"] = [normalize_field(f, f"{ctx}['{k}']") for f in v]
           return out

       raise ValueError(
           f"{ctx}: case object must be single-key shorthand {{'Case':[...]}}"
       )

   raise ValueError(f"{ctx}: invalid case form (expected string or object)")


def normalize_spec(raw: Any) -> Dict[str, Any]:
   if not isinstance(raw, dict):
       raise ValueError("Top-level JSON must be an object")

   out: Dict[str, Any] = {}
   if "namespace" in raw:
       out["namespace"] = raw["namespace"]
   if "includes" in raw:
       out["includes"] = raw["includes"]

   if "name" not in raw:
       raise ValueError("Missing required key 'name'")
   out["name"] = raw["name"]

   if "cases" not in raw or not isinstance(raw["cases"], list) or not raw["cases"]:
       raise ValueError("'cases' must be a non-empty list")
   out["cases"] = [normalize_case(c, i) for i, c in enumerate(raw["cases"])]

   return out


# ----------------------------
# Validation (normalized)
# ----------------------------

def validate_with_jsonschema(spec_norm: Dict[str, Any]) -> Tuple[bool, Optional[str]]:
   try:
       import jsonschema  # type: ignore
       from jsonschema.validators import Draft202012Validator  # type: ignore

       v = Draft202012Validator(SCHEMA_NORMALIZED)
       errors = sorted(v.iter_errors(spec_norm), key=lambda e: list(e.path))
       if not errors:
           return True, None
       e = errors[0]
       path = ".".join(str(p) for p in e.path) if e.path else "(root)"
       return False, f"Schema validation failed at {path}: {e.message}"
   except ImportError:
       return False, "jsonschema not installed"
   except Exception as ex:
       return False, f"jsonschema validation error: {ex}"


def manual_validate_normalized(spec: Dict[str, Any]) -> None:
   ns = spec.get("namespace")
   if ns is not None:
       if not isinstance(ns, str):
           raise ValueError("'namespace' must be a string")
       if ns.strip():
           for part in ns.split("::"):
               cpp_ident(part)

   incs = spec.get("includes", [])
   if incs is not None and not isinstance(incs, list):
       raise ValueError("'includes' must be a list of strings if present")
   if isinstance(incs, list):
       for inc in incs:
           if not isinstance(inc, str):
               raise ValueError("All 'includes' entries must be strings")

   cpp_ident(spec["name"])

   cases = spec.get("cases")
   if not isinstance(cases, list) or not cases:
       raise ValueError("'cases' must be a non-empty list")

   seen_case_names: Set[str] = set()
   for case in cases:
       cname = cpp_ident(case["name"])
       if cname in seen_case_names:
           raise ValueError(f"Duplicate case name: {cname}")
       seen_case_names.add(cname)

       fields = case.get("fields", [])
       if fields is None:
           fields = []
       if not isinstance(fields, list):
           raise ValueError(f"Case '{cname}': 'fields' must be a list")
       seen_fields: Set[str] = set()
       for f in fields:
           fname = cpp_ident(f["name"])
           if fname in seen_fields:
               raise ValueError(f"Case '{cname}': duplicate field name '{fname}'")
           seen_fields.add(fname)
           if not isinstance(f["type"], str) or not f["type"].strip():
               raise ValueError(f"Case '{cname}': field 'type' must be a non-empty string")
           if "default" in f and not isinstance(f["default"], str):
               raise ValueError(f"Case '{cname}': field 'default' must be a string (raw C++)")


# ----------------------------
# C++ generation
# ----------------------------

def collect_includes(spec: Dict[str, Any]) -> List[str]:
   user_incs = spec.get("includes", [])
   out: List[str] = []
   seen: Set[str] = set()

   def norm(inc: str) -> str:
       inc = inc.strip()
       if inc.startswith("#include"):
           return inc
       if inc.startswith("<") or inc.startswith('"'):
           return f"#include {inc}"
       return f"#include <{inc}>"

   def add(inc: str) -> None:
       incn = norm(inc)
       if incn not in seen:
           seen.add(incn)
           out.append(incn)

   if isinstance(user_incs, list):
       for inc in user_incs:
           if isinstance(inc, str) and inc.strip():
               add(inc)

   # Required baseline
   add("<variant>")
   add("<utility>")

   return out


def field_decl(field: Dict[str, Any]) -> str:
   name = cpp_ident(field["name"])
   typ = str(field["type"]).strip()
   default = field.get("default")
   if default is None:
       return f"{typ} {name};"
   return f"{typ} {name} = {default};"


def ctor_param_list(fields: List[Dict[str, Any]]) -> str:
   # Pass by value; move into members.
   return ", ".join(f"{str(f['type']).strip()} {cpp_ident(f['name'])}" for f in fields)


def ctor_init_list(fields: List[Dict[str, Any]]) -> str:
   return ", ".join(f"{cpp_ident(f['name'])}(std::move({cpp_ident(f['name'])}))" for f in fields)


def tags_name(enum_name: str) -> str:
   return f"{enum_name}Tags"


def tags_qualified(enum_name: str) -> str:
   return f"detail::{tags_name(enum_name)}"


def emit_case(case: Dict[str, Any], enum_name: str, indent: str) -> str:
   cname = cpp_ident(case["name"])
   fields = case.get("fields", [])
   if fields is None:
       fields = []

   lines: List[str] = [f"{indent}struct {cname} {{"]
   lines.append(f"{indent}    using Variant = {enum_name};")

   if fields:
       for f in fields:
           lines.append(f"{indent}    {field_decl(f)}")
       lines.append("")
   lines.append(f"{indent}    constexpr {cname}() = default;")
   if fields:
       lines.append(f"{indent}    constexpr {cname}({ctor_param_list(fields)}) : {ctor_init_list(fields)} {{}}")
   lines.append(f"{indent}}};")
   return "\n".join(lines)


def emit_tags(enum_name: str, cases: List[Dict[str, Any]]) -> str:
   """
   Emit <EnumName>Tags in namespace detail to discourage direct use.
   """
   tags = tags_name(enum_name)
   lines: List[str] = []
   lines.append("namespace detail {")
   lines.append(f"struct {tags} {{")
   for i, c in enumerate(cases):
       lines.append(emit_case(c, enum_name, "    "))
       if i != len(cases) - 1:
           lines.append("")
   lines.append("};")
   lines.append("} // namespace detail")
   return "\n".join(lines)


def emit_variant(enum_name: str, cases: List[Dict[str, Any]]) -> str:
   tagsq = tags_qualified(enum_name)
   alts = ", ".join(f"{tagsq}::{cpp_ident(c['name'])}" for c in cases)

   lines: List[str] = [
       f"struct {enum_name} : std::variant<{alts}> {{",
       f"    using Base = std::variant<{alts}>;",
       f"    using Base::Base;",
       "",
       f"    constexpr {enum_name}() = default;",
       f"    constexpr {enum_name}(const {enum_name}&) = default;",
       f"    constexpr {enum_name}({enum_name}&&) = default;",
       f"    constexpr {enum_name}& operator=(const {enum_name}&) = default;",
       f"    constexpr {enum_name}& operator=({enum_name}&&) = default;",
       "",
       f"    // Re-export cases so you can write {enum_name}::Case",
   ]

   for c in cases:
       cname = cpp_ident(c["name"])
       lines.append(f"    using {cname} = {tagsq}::{cname};")

   lines.append("")
   lines.append(f"    // Convenience constructors: {enum_name} v = {enum_name}::Case{{...}};")
   for c in cases:
       cname = cpp_ident(c["name"])
       lines.append(f"    constexpr {enum_name}({cname} v) : Base(std::move(v)) {{}}")

   lines.append("};")
   return "\n".join(lines)


def generate_cpp(spec_norm: Dict[str, Any]) -> str:
   ns = spec_norm.get("namespace")
   ns = ns.strip() if isinstance(ns, str) else ""

   enum_name = cpp_ident(spec_norm["name"])
   cases = spec_norm["cases"]

   includes = collect_includes(spec_norm)

   out: List[str] = []
   out.append("// Auto-generated. Do not edit by hand.")
   out.append("")
   out.append("#pragma once")
   out.append("")
   out.extend(sorted(includes))
   out.append("")

   if ns:
       out.append(f"namespace {ns} {{")
       out.append("")

   out.append(f"struct {enum_name};")
   out.append("")

   out.append(emit_tags(enum_name, cases))
   out.append("")
   out.append(f"// ===== {enum_name} =====")
   out.append(emit_variant(enum_name, cases))
   out.append("")

   if ns:
       out.append(f"}} // namespace {ns}")

   return "\n".join(out).rstrip() + "\n"


# ----------------------------
# CLI
# ----------------------------

def main(argv: List[str]) -> int:
   if len(argv) >= 2 and argv[1] == "--schema":
       json.dump(SCHEMA_NORMALIZED, sys.stdout, indent=2)
       sys.stdout.write("\n")
       return 0

   if len(argv) < 2:
       eprint("Usage: gen_sumtype.py <spec.json> [output.hpp]")
       eprint("       gen_sumtype.py --schema")
       return 2

   spec_path = Path(argv[1])
   out_path: Optional[Path] = Path(argv[2]) if len(argv) > 2 else None

   try:
       raw = json.loads(spec_path.read_text(encoding="utf-8"))
       spec_norm = normalize_spec(raw)

       ok, msg = validate_with_jsonschema(spec_norm)
       if ok:
           pass
       else:
           if msg == "jsonschema not installed":
               manual_validate_normalized(spec_norm)
           else:
               raise ValueError(msg)

       cpp = generate_cpp(spec_norm)
   except Exception as ex:
       eprint(f"Error: {ex}")
       return 1

   if out_path:
       out_path.write_text(cpp, encoding="utf-8")
   else:
       sys.stdout.write(cpp)

   return 0


if __name__ == "__main__":
   raise SystemExit(main(sys.argv))