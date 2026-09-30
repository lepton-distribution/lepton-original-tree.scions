#!/usr/bin/env python3
"""transform_iar.py — transformations rejouables des IAR-ismes (ETAPE-3 tâche 1 : amorce ;
ETAPE-4 tâche 2 : transformation de masse).

Une règle par IAR-isme, activable seule (--rule). Chaque occurrence est classée « automatique »
(transformée) ou « résiduelle » (laissée, avec sa raison, dans le rapport --report) ; seules les
résiduelles se traitent à la main. Écrit uniquement dans le clone (jamais dans le trunk, jamais
sur un lien) et seulement avec --apply ; sinon, simulation.

Règles :
  garde-iar-arm     branches de préprocesseur réservées au compilateur IAR ARM (__ICCARM__,
                    __IAR_SYSTEMS_ICC__, __tauon_compiler__ == __compiler_iar_arm__) : évaluées en
                    logique à trois valeurs (GCC seul : prédicats IAR faux, autres atomes
                    inconnus) ; branche fausse retirée, branche toujours vraie déballée, condition
                    mixte simplifiée. Résiduels : condition non décidable (ex. « || » avec un
                    atome inconnu restant IAR), IAR M16C (code gelé, étape 6), branche retirée
                    contenant un _Pragma/#pragma (placement mémoire à reporter, étape 5).
  intrinsics-cmsis  intrinsics IAR → CMSIS-Core (Cortex-M) ; résiduels : symboles de l'éditeur de
                    liens (__sfe, __section_begin/end), accès CPSR et coprocesseur (ARM7/9).

Vérification associée (--cpp-snapshot / --cpp-compare) : sortie « gcc -E -P » de chaque source
de compile_commands.json avant et après ; une règle de gardes correcte la laisse identique.

Exemples :
  transform_iar.py --rule garde-iar-arm --files-from chaine.txt              # simulation
  transform_iar.py --rule garde-iar-arm --files-from chaine.txt --apply \\
                   --report doc/migration/residuel-etape3.md
"""
import argparse
import json
import os
import re
import subprocess
import sys

# code tiers vendored dans l'arbre : jamais transformé (CLAUDE.md)
TIERS = ("sys/root/src/kernel/core/ucore/", "sys/root/src/kernel/net/lwip/api/",
         "sys/root/src/kernel/net/lwip/core/", "sys/root/src/kernel/net/lwip/include/",
         "sys/root/src/kernel/net/lwip/netif/", "sys/root/src/kernel/net/uip",
         "sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/",
         "sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/driverlib/")

# ---------------------------------------------------------------------------------------------
# expressions de préprocesseur : logique à trois valeurs
# ---------------------------------------------------------------------------------------------
IAR_ARM_MACROS = ("__ICCARM__", "__IAR_SYSTEMS_ICC__")
IAR_OTHER = re.compile(r"__compiler_iar_m16c__|\b__IAR_SYSTEMS_ICC\b(?!__)")
T, F, U = "T", "F", "U"

TOKEN = re.compile(r"\s*(defined|[A-Za-z_]\w*|0[xX][0-9a-fA-F]+[uUlL]*|\d+[uUlL]*|&&|\|\||==|!=|"
                   r"<=|>=|<<|>>|[()!<>+\-*/%&|^~?:,])")


def tokenize(expr):
    expr = re.sub(r"/\*.*?\*/", " ", expr)
    expr = re.sub(r"//.*", "", expr).strip()
    toks, pos = [], 0
    while pos < len(expr):
        m = TOKEN.match(expr, pos)
        if not m:
            if expr[pos:].strip() == "":
                break
            raise ValueError("jeton inattendu : %r" % expr[pos:])
        toks.append(m.group(1))
        pos = m.end()
    return toks


class Node:
    """op : 'or', 'and', 'not', 'rel' (a, rel, b), 'defined' (name), 'atom' (texte brut)."""
    def __init__(self, op, *args):
        self.op, self.args = op, args


class Parser:
    def __init__(self, toks):
        self.toks, self.i = toks, 0

    def peek(self):
        return self.toks[self.i] if self.i < len(self.toks) else None

    def take(self, t=None):
        tok = self.peek()
        if t is not None and tok != t:
            raise ValueError("attendu %r, lu %r" % (t, tok))
        self.i += 1
        return tok

    def parse(self):
        n = self.p_or()
        if self.peek() is not None:
            raise ValueError("reste : %r" % self.toks[self.i:])
        return n

    def p_or(self):
        n = self.p_and()
        while self.peek() == "||":
            self.take()
            n = Node("or", n, self.p_and())
        return n

    def p_and(self):
        n = self.p_not()
        while self.peek() == "&&":
            self.take()
            n = Node("and", n, self.p_not())
        return n

    def p_not(self):
        if self.peek() == "!":
            self.take()
            return Node("not", self.p_not())
        return self.p_rel()

    def p_rel(self):
        a = self.p_primary()
        if self.peek() in ("==", "!=", "<", ">", "<=", ">="):
            op = self.take()
            b = self.p_primary()
            return Node("rel", a, op, b)
        return a

    def p_primary(self):
        tok = self.peek()
        if tok == "(":
            self.take()
            n = self.p_or()
            self.take(")")
            return Node("paren", n)
        if tok == "defined":
            self.take()
            if self.peek() == "(":
                self.take()
                name = self.take()
                self.take(")")
            else:
                name = self.take()
            return Node("defined", name)
        if tok is None or tok in ("&&", "||", ")"):
            raise ValueError("expression incomplète")
        # atome : identifiant, nombre, ou suite arithmétique non analysée jusqu'à un opérateur
        parts = [self.take()]
        while self.peek() is not None and self.peek() not in ("&&", "||", ")", "==", "!=", "<",
                                                              ">", "<=", ">="):
            if self.peek() == "(":
                depth = 0
                while True:
                    t = self.take()
                    parts.append(t)
                    depth += (t == "(") - (t == ")")
                    if depth == 0:
                        break
            else:
                parts.append(self.take())
        return Node("atom", " ".join(parts))


def strip_paren(n):
    while n.op == "paren":
        n = n.args[0]
    return n


def value_of(n):
    """Valeur entière connue d'un opérande sous GCC, ou None."""
    n = strip_paren(n)
    if n.op != "atom":
        return None
    t = n.args[0]
    if t in IAR_ARM_MACROS:
        return 0                                   # non défini sous GCC
    m = re.fullmatch(r"(0[xX][0-9a-fA-F]+|\d+)[uUlL]*", t)
    return int(m.group(1), 0) if m else None


def evaluate(n):
    """Rend (valeur T/F/U, nœud simplifié, touché) ; touché : un prédicat IAR ARM a été décidé."""
    op = n.op
    if op == "paren":
        v, s, t = evaluate(n.args[0])
        return v, (s if s.op in ("atom", "defined", "paren", "not") else Node("paren", s)), t
    if op == "defined":
        if n.args[0] in IAR_ARM_MACROS:
            return F, n, True
        return U, n, False
    if op == "atom":
        if n.args[0] in IAR_ARM_MACROS:
            return F, n, True
        return U, n, False
    if op == "not":
        v, s, t = evaluate(n.args[0])
        return {T: F, F: T, U: U}[v], Node("not", s), t
    if op == "rel":
        a, rel, b = n.args
        names = {strip_paren(a).args[0] if strip_paren(a).op == "atom" else None,
                 strip_paren(b).args[0] if strip_paren(b).op == "atom" else None}
        if "__compiler_iar_arm__" in names and "__tauon_compiler__" in names and rel in ("==", "!="):
            return (F if rel == "==" else T), n, True
        va, vb = value_of(a), value_of(b)
        iar = any(strip_paren(x).op == "atom" and strip_paren(x).args[0] in IAR_ARM_MACROS
                  for x in (a, b))
        if iar and va is not None and vb is not None:
            r = {"==": va == vb, "!=": va != vb, "<": va < vb, ">": va > vb,
                 "<=": va <= vb, ">=": va >= vb}[rel]
            return (T if r else F), n, True
        return U, n, False
    if op in ("and", "or"):
        va, sa, ta = evaluate(n.args[0])
        vb, sb, tb = evaluate(n.args[1])
        touched = ta or tb
        absorb, neutral = (F, T) if op == "and" else (T, F)
        if absorb in (va, vb):
            return absorb, n, touched
        if va == neutral:
            return vb, sb, touched
        if vb == neutral:
            return va, sa, touched
        if va == neutral and vb == neutral:
            return neutral, n, touched
        return U, Node(op, sa, sb), touched
    raise ValueError(op)


def render(n):
    op = n.op
    if op == "paren":
        return "(" + render(n.args[0]) + ")"
    if op == "defined":
        return "defined(%s)" % n.args[0]
    if op == "atom":
        return n.args[0]
    if op == "not":
        return "!" + render(n.args[0])
    if op == "rel":
        return "%s %s %s" % (render(n.args[0]), n.args[1], render(n.args[2]))
    sep = " && " if op == "and" else " || "
    return render(n.args[0]) + sep + render(n.args[1])


# ---------------------------------------------------------------------------------------------
# structure #if … #endif
# ---------------------------------------------------------------------------------------------
DIRECTIVE = re.compile(r"^(\s*)#\s*(if|ifdef|ifndef|elif|else|endif)\b(.*)$", re.S)


def logical_lines(lines):
    """Regroupe les lignes continuées (\\) : liste de (index de début, index de fin exclus)."""
    out, i = [], 0
    while i < len(lines):
        j = i
        while lines[j].rstrip("\r\n").endswith("\\") and j + 1 < len(lines):
            j += 1
        out.append((i, j + 1))
        i = j + 1
    return out


class Cond:
    def __init__(self):
        self.branches = []   # [kind, cond_text, (s, e), body, indent]
        self.endif = None    # (s, e)


def parse_block(lines, spans, k, top):
    """Rend (liste d'éléments, index suivant) ; élément = (s, e) de texte ou Cond."""
    items = []
    while k < len(spans):
        s, e = spans[k]
        text = "".join(l.rstrip("\r\n").rstrip("\\") for l in lines[s:e])
        m = DIRECTIVE.match(text)
        if m and m.group(2) in ("elif", "else", "endif"):
            if top:
                raise ValueError("ligne %d : #%s sans #if" % (s + 1, m.group(2)))
            return items, k
        if m and m.group(2) in ("if", "ifdef", "ifndef"):
            c = Cond()
            kind, cond, indent = m.group(2), m.group(3), m.group(1)
            k += 1
            while True:
                body, k = parse_block(lines, spans, k, False)
                c.branches.append([kind, cond, (s, e), body, indent])
                if k >= len(spans):
                    raise ValueError("#if sans #endif (ligne %d)" % (s + 1))
                s, e = spans[k]
                text = "".join(l.rstrip("\r\n").rstrip("\\") for l in lines[s:e])
                m = DIRECTIVE.match(text)
                k += 1
                if m.group(2) == "endif":
                    c.endif = (s, e)
                    break
                kind, cond, indent = m.group(2), m.group(3), m.group(1)
            items.append(c)
            continue
        items.append((s, e))
        k += 1
    return items, k


def branch_expr(kind, cond):
    if kind == "ifdef":
        return "defined(%s)" % cond.split()[0]
    if kind == "ifndef":
        return "!defined(%s)" % cond.split()[0]
    return cond


def body_text(lines, body):
    out = []
    for it in body:
        if isinstance(it, Cond):
            for b in it.branches:
                out.append("".join(lines[b[2][0]:b[2][1]]))
                out.append(body_text(lines, b[3]))
            out.append("".join(lines[it.endif[0]:it.endif[1]]))
        else:
            out.append("".join(lines[it[0]:it[1]]))
    return "".join(out)


def rewrite(lines, items, path, auto, resid):
    out = []
    for it in items:
        if not isinstance(it, Cond):
            out.extend(lines[it[0]:it[1]])
            continue
        kept, unwrapped, dead_rest = [], None, False
        all_false_so_far = True
        for kind, cond, (s, e), body, indent in it.branches:
            line = s + 1
            if dead_rest:
                auto.append((line, "branche morte (précédente toujours vraie)", cond.strip()))
                continue
            if kind == "else":
                if all_false_so_far and not kept:
                    unwrapped = body
                    auto.append((line, "#else toujours pris", ""))
                else:
                    kept.append([kind, cond, (s, e), body, indent, None])
                break
            expr = branch_expr(kind, cond)
            try:
                v, simp, touched = evaluate(Parser(tokenize(expr)).parse())
            except ValueError as err:
                v, simp, touched = U, None, False
                if IAR_OTHER.search(expr) or any(x in expr for x in IAR_ARM_MACROS + ("__compiler_iar_arm__",)):
                    resid.append((line, "condition non analysée (%s)" % err, expr.strip()))
            if not touched:
                if IAR_OTHER.search(expr):
                    resid.append((line, "IAR M16C (code gelé, étape 6)", expr.strip()))
                elif any(x in expr for x in IAR_ARM_MACROS + ("__compiler_iar_arm__",)):
                    resid.append((line, "condition IAR non décidable", expr.strip()))
                kept.append([kind, cond, (s, e), body, indent, None])
                all_false_so_far = False
                continue
            if v == F:
                if re.search(r"_Pragma\s*\(|#\s*pragma", body_text(lines, body)):
                    resid.append((line, "branche IAR avec _Pragma/#pragma (placement, étape 5)",
                                  expr.strip()))
                    kept.append([kind, cond, (s, e), body, indent, None])
                    all_false_so_far = False
                else:
                    auto.append((line, "branche IAR retirée", expr.strip()))
                continue
            if v == T:
                if not kept:
                    unwrapped = body
                    auto.append((line, "garde toujours vraie levée", expr.strip()))
                else:
                    # new = "" : directive réécrite (la ligne d'origine est un #elif)
                    kept.append(["else", "", (s, e), body, indent, ""])
                    auto.append((line, "garde toujours vraie → #else", expr.strip()))
                dead_rest = True
                continue
            # indécidable mais simplifiée
            new = render(simp)
            auto.append((line, "condition simplifiée", "%s → %s" % (expr.strip(), new)))
            kept.append([kind, cond, (s, e), body, indent, new])
            all_false_so_far = False
        # émission
        for idx, (kind, cond, (s, e), body, indent, new) in enumerate(kept):
            first = idx == 0
            if new is None and ((first and kind in ("if", "ifdef", "ifndef")) or
                                (not first and kind in ("elif", "else"))):
                out.extend(lines[s:e])                     # directive inchangée
            else:
                nl = "\n"
                if kind == "else":
                    out.append("%s#else%s" % (indent, nl))
                else:
                    expr = new if new is not None else branch_expr(kind, cond).strip()
                    out.append("%s#%s %s%s" % (indent, "if" if first else "elif", expr, nl))
            out.extend(rewrite(lines, body, path, auto, resid))
        if kept:
            out.extend(lines[it.endif[0]:it.endif[1]])
        elif unwrapped is not None:
            out.extend(rewrite(lines, unwrapped, path, auto, resid))
    return out


def rule_garde_iar_arm(text, path):
    lines = text.splitlines(keepends=True)
    spans = logical_lines(lines)
    items, _ = parse_block(lines, spans, 0, True)
    auto, resid = [], []
    new = "".join(rewrite(lines, items, path, auto, resid))
    return new, auto, resid


# ---------------------------------------------------------------------------------------------
# intrinsics IAR → CMSIS-Core (Cortex-M)
# ---------------------------------------------------------------------------------------------
INTRINSICS = {
    "__disable_interrupt": "__disable_irq",
    "__enable_interrupt": "__enable_irq",
    "__no_operation": "__NOP",
    "__get_interrupt_state": "__get_PRIMASK",   # Cortex-M : état = PRIMASK
    "__set_interrupt_state": "__set_PRIMASK",
}
INTRINSICS_RESID = {
    "__sfe": "symbole de l'éditeur de liens (fin de section) : script .ld",
    "__section_begin": "symbole de l'éditeur de liens (début de section) : script .ld",
    "__section_end": "symbole de l'éditeur de liens (fin de section) : script .ld",
    "__get_CPSR": "CPSR : ARM7/ARM9 (code gelé)",
    "__set_CPSR": "CPSR : ARM7/ARM9 (code gelé)",
    "__MRC": "coprocesseur : ARM7/ARM9 (code gelé)",
    "__MCR": "coprocesseur : ARM7/ARM9 (code gelé)",
}


def rule_intrinsics_cmsis(text, path):
    auto, resid = [], []
    out = []
    for no, line in enumerate(text.splitlines(keepends=True), 1):
        code = line.split("//")[0]
        for iar, cmsis in INTRINSICS.items():
            if re.search(r"\b%s\s*\(" % iar, code):
                line = re.sub(r"\b%s(\s*\()" % iar, cmsis + r"\1", line)
                auto.append((no, "intrinsic %s → %s" % (iar, cmsis), line.strip()))
        for iar, why in INTRINSICS_RESID.items():
            if re.search(r"\b%s\s*\(" % iar, code):
                resid.append((no, why, line.strip()))
        if re.search(r"#\s*include\s*[<\"]intrinsics\.h[>\"]", code):
            resid.append((no, "en-tête IAR intrinsics.h : inclure CMSIS-Core", line.strip()))
        out.append(line)
    return "".join(out), auto, resid


RULES = {"garde-iar-arm": rule_garde_iar_arm, "intrinsics-cmsis": rule_intrinsics_cmsis}

# ---------------------------------------------------------------------------------------------
# fichiers, vérification gcc -E, rapport
# ---------------------------------------------------------------------------------------------


def clone_root():
    env = os.environ.get("LEPTON_CLONE")
    if env:
        return os.path.realpath(env)
    here = os.path.dirname(os.path.realpath(__file__))
    return os.path.realpath(os.path.join(here, "..", ".."))


def resolve(clone, rel):
    """Chemin relatif au trunk (sys/root/src/…) → fichier du clone (scion/…), jamais un lien."""
    rel = rel.strip()
    if rel.startswith("scion/"):
        rel = rel[len("scion/"):]
    path = os.path.join(clone, "scion", rel)
    if os.path.islink(path):
        raise SystemExit("refus : %s est un lien" % path)
    real = os.path.realpath(path)
    if not real.startswith(os.path.join(clone, "scion") + os.sep):
        raise SystemExit("refus : %s hors du clone" % path)
    return rel, real


def cpp_outputs(compile_db):
    db = json.load(open(compile_db))
    res = {}
    for ent in db:
        cmd = ent.get("command")
        if not cmd or not ent["file"].endswith(".c"):
            continue
        cmd = re.sub(r"\s-o\s+\S+", " ", cmd)
        cmd = re.sub(r"\s-c(\s|$)", r"\1", cmd)
        cmd = re.sub(r"\s-M[TFQ]\s+\S+", " ", cmd)      # -MT/-MF/-MQ <arg>
        cmd = re.sub(r"\s-MM?D(?=\s)", " ", cmd)          # -MD, -MMD
        # __DATE__/__TIME__ figés : seules les différences de code comptent
        cmd += (" -Wno-builtin-macro-redefined -D'__DATE__=\"Jan  1 1970\"'"
                " -D'__TIME__=\"00:00:00\"'")
        r = subprocess.run(cmd + " -E -P -o -", shell=True, cwd=ent["directory"],
                           capture_output=True, text=True)
        res[ent["file"]] = r.stdout if r.returncode == 0 else "ERREUR : " + r.stderr
    return res


def main():
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0],
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("files", nargs="*", help="fichiers relatifs au trunk (sys/root/src/…)")
    ap.add_argument("--files-from", help="liste de fichiers (un par ligne)")
    ap.add_argument("--rule", action="append", choices=sorted(RULES),
                    help="règle à appliquer (répétable ; défaut : toutes)")
    ap.add_argument("--apply", action="store_true", help="écrire (sinon simulation)")
    ap.add_argument("--report", help="rapport Markdown (automatiques et résiduels)")
    ap.add_argument("--cpp-snapshot", nargs=2, metavar=("COMPILE_DB", "JSON"),
                    help="enregistre la sortie gcc -E -P de chaque source")
    ap.add_argument("--cpp-compare", nargs=2, metavar=("COMPILE_DB", "JSON"),
                    help="compare la sortie gcc -E -P à un instantané ; 1 si différence")
    args = ap.parse_args()

    if args.cpp_snapshot:
        json.dump(cpp_outputs(args.cpp_snapshot[0]), open(args.cpp_snapshot[1], "w"))
        print("instantané gcc -E : %s" % args.cpp_snapshot[1])
        return 0
    if args.cpp_compare:
        before = json.load(open(args.cpp_compare[1]))
        after = cpp_outputs(args.cpp_compare[0])
        diff = sorted(f for f in set(before) | set(after) if before.get(f) != after.get(f))
        for f in diff:
            print("gcc -E différent : %s" % f)
        print("gcc -E : %d source(s) comparée(s), %d différente(s)" % (len(after), len(diff)))
        return 1 if diff else 0

    files = list(args.files)
    if args.files_from:
        files += [l.strip() for l in open(args.files_from)
                  if l.strip() and not l.lstrip().startswith("#")]
    rules = args.rule or sorted(RULES)
    clone = clone_root()
    report = []
    n_auto = n_resid = n_changed = 0
    for rel in files:
        if rel.startswith(TIERS) or not rel.endswith((".c", ".h")):
            continue
        rel, path = resolve(clone, rel)
        text = open(path, encoding="latin-1").read()
        new = text
        for r in rules:
            new, auto, resid = RULES[r](new, rel)
            for (line, what, detail) in auto:
                report.append((rel, r, "automatique", line, what, detail))
            for (line, what, detail) in resid:
                report.append((rel, r, "résiduel", line, what, detail))
            n_auto += len(auto)
            n_resid += len(resid)
        if new != text:
            n_changed += 1
            if args.apply:
                open(path, "w", encoding="latin-1").write(new)
    mode = "appliqué" if args.apply else "simulation"
    print("transform_iar (%s) : règles %s ; %d fichier(s) modifié(s), %d automatique(s), "
          "%d résiduel(s)" % (mode, ",".join(rules), n_changed, n_auto, n_resid))
    for row in report:
        print("  %s:%d [%s, %s] %s %s" % (row[0], row[3], row[1], row[2], row[4],
                                          ("— " + row[5]) if row[5] else ""))
    if args.report:
        with open(args.report, "w") as f:
            f.write("# Transformation des IAR-ismes — rapport\n\n")
            f.write("Généré par `tools/migration/transform_iar.py` — ne pas éditer à la main.  \n")
            f.write("Commande : `%s`\n\n" % " ".join(["transform_iar.py"] + sys.argv[1:]))
            f.write("Résumé : %d fichier(s) modifié(s), %d occurrence(s) automatique(s), "
                    "%d résiduelle(s).\n\n" % (n_changed, n_auto, n_resid))
            for cls in ("résiduel", "automatique"):
                rows = [r for r in report if r[2] == cls]
                f.write("## %s (%d)\n\n| Fichier | Ligne | Règle | Motif | Détail |\n"
                        "|---|---:|---|---|---|\n" % (cls.capitalize() + "s", len(rows)))
                for r in rows:
                    f.write("| `%s` | %d | %s | %s | `%s` |\n"
                            % (r[0], r[3], r[1], r[4], r[5].replace("|", "\\|")))
                f.write("\n")
    return 0


if __name__ == "__main__":
    sys.exit(main())
