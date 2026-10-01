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
                    atome inconnu restant IAR), IAR M16C (règle garde-cible-gelee), branche retirée
                    contenant un _Pragma/#pragma (placement mémoire à reporter, étape 5) — sauf
                    « #pragma data_alignment » doublé par l'alignement GCC de la déclaration
                    suivante (__ALIGN_BEGIN/__ALIGN_END, aligned) : branche retirée.
  garde-cible-gelee branches des cibles gelées (décisions D2a et D3a du 2026-10-01) : IAR M16C
                    (__compiler_iar_m16c__, __IAR_SYSTEMS_ICC), Win32, ARM7/ARM9, eCos
                    (CPU_WIN32, CPU_ARM7, CPU_ARM9, CPU_M16C62, __KERNEL_UCORE_ECOS, valeurs
                    de __tauon_cpu_core__ et __tauon_cpu_device__ correspondantes) : même
                    évaluation, prédicats faux.
  garde-compilateur gardes de compilateur sous GCC seul (D3a) : __GNUC__ et __compiler_gnuc__
                    vrais ; __CC_ARM, _MSC_VER, __compiler_keil_arm__, __compiler_win32__ faux.
                    Les branches GCC héritées de la simulation Linux sont déballées sans
                    correction (valeurs à revoir : handoff de l'étape 3).
  Ces deux règles copient d'abord le fichier d'origine à l'identique sous --legacy-dir (défaut
  scion/legacy/<chemin>, classé gelé, supprimé à l'étape 6) dès qu'elles retirent du code ;
  jamais d'écrasement d'une copie existante.
  header-iar        en-têtes de la bibliothèque ou des périphériques IAR (intrinsics.h, yvals.h,
                    ysizet.h, DLib_*.h, io<puce>.h) : signalés, toujours résiduels.
  intrinsics-cmsis  intrinsics IAR → CMSIS-Core (Cortex-M) ; résiduels : symboles de l'éditeur de
                    liens (__sfe, __section_begin/end), accès CPSR et coprocesseur (ARM7/9).
  mot-cle-iar       mots-clés IAR → macros de compiler.h : « __packed struct|union » →
                    « struct|union __lepton_packed » ; __no_init, __root, __weak, __ramfunc,
                    __noreturn → __lepton_*. Ajoute #include "kernel/core/compiler.h" si le
                    fichier ne l'inclut pas. Résiduels : __packed hors struct/union (pointeur :
                    accès non aligné), définition de compatibilité (#define __packed …).
  pragma-iar        _Pragma("location=\\"S\\"") → __lepton_section("S") (résiduel en plus si la
                    section S n'existe dans aucun ld/*.ld) ; #pragma optimize, diag_*, rtmodel
                    retirés ; autres pragmas IAR (location, data_alignment, language, inline,
                    required, segment, memory…) résiduels.
  prototype-static  (portabilité GCC 14, pas un IAR-isme) fonction définie « static » après un
                    prototype non static du même fichier : « static » ajouté au prototype (erreur
                    GCC « static declaration follows non-static declaration », tolérée par IAR).
                    Prototype dans un en-tête : hors règle (correction manuelle).
  symbole-iar       symboles de la bibliothèque IAR DLIB (__iar_*) et de l'éditeur de liens
                    (__ICFEDIT_*) : signalés, toujours résiduels.

Code tiers (classement d'audit_iar.py, ORIGINE_TIERS, et liste TIERS) : jamais transformé
(décision D1a du 2026-10-01 : justifié dans residuel-etape4.md, --tiers-from-audit).

Vérification associée (--cpp-snapshot / --cpp-compare) : sortie « gcc -E -P » de chaque source
de compile_commands.json avant et après ; une règle de gardes correcte la laisse identique.

Exemples :
  transform_iar.py --rule garde-iar-arm --files-from chaine.txt              # simulation
  transform_iar.py --rule garde-iar-arm --files-from chaine.txt --apply \\
                   --report doc/migration/residuel-etape3.md
"""
import argparse
import csv
import glob
import json
import os
import re
import shutil
import subprocess
import sys

sys.path.insert(0, os.path.dirname(os.path.realpath(__file__)))
from audit_iar import ORIGINE_TIERS  # noqa: E402  (même classement tiers/Lepton que l'audit)

# code tiers vendored dans l'arbre : jamais transformé (CLAUDE.md)
TIERS = ("sys/root/src/kernel/core/ucore/", "sys/root/src/kernel/net/lwip/api/",
         "sys/root/src/kernel/net/lwip/core/", "sys/root/src/kernel/net/lwip/include/",
         "sys/root/src/kernel/net/lwip/netif/", "sys/root/src/kernel/net/uip",
         "sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/",
         "sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/driverlib/")

# ---------------------------------------------------------------------------------------------
# expressions de préprocesseur : logique à trois valeurs
# ---------------------------------------------------------------------------------------------
T, F, U = "T", "F", "U"
IAR_ARM_MACROS = ("__ICCARM__", "__IAR_SYSTEMS_ICC__")
IAR_OTHER = re.compile(r"__compiler_iar_m16c__|\b__IAR_SYSTEMS_ICC\b(?!__)")


class Preds:
    """Prédicats décidés sous GCC seul.
    macros : macros non définies (defined → faux, valeur 0) ; vrais : macros définies (defined →
    vrai) ; faux_val / vrai_val : {variable: prédicat(valeur)} pour « variable ==/!= valeur »
    (ex. __tauon_compiler__ == __compiler_iar_arm__) ; motifs : texte qui signale le contexte."""
    def __init__(self, nom, macros=(), vrais=(), faux_val=None, vrai_val=None, motifs=()):
        self.nom, self.macros, self.vrais = nom, tuple(macros), tuple(vrais)
        self.faux_val, self.vrai_val = faux_val or {}, vrai_val or {}
        self.motifs = re.compile("|".join([r"\b%s\b" % re.escape(x)
                                           for x in self.macros + self.vrais] + list(motifs)))

    def mentionne(self, expr):
        return bool(self.motifs.search(expr))

    def decide_rel(self, var, val):
        """Valeur de « var == val » : T, F ou None (non décidé)."""
        if var in self.vrai_val and self.vrai_val[var](val):
            return T
        if var in self.faux_val and self.faux_val[var](val):
            return F
        return None


ARM = Preds("IAR ARM", IAR_ARM_MACROS, faux_val={"__tauon_compiler__": lambda v: v == "__compiler_iar_arm__"},
            motifs=[r"\b__compiler_iar_arm__\b"])
# cibles gelées (code-gele.md ; décisions D2a et D3a) : IAR M16C, Win32, ARM7/ARM9, eCos
CIBLES_GELEES_COEURS = ("__tauon_cpu_core_arm_arm7tdmi__", "__tauon_cpu_core_arm_arm926ejs__",
                        "__tauon_cpu_core_win32_simulation__", "__tauon_cpu_core_m16c__")
GELE = Preds("cible gelée",
             ("__IAR_SYSTEMS_ICC", "CPU_WIN32", "CPU_ARM7", "CPU_ARM9", "CPU_M16C62",
              "__KERNEL_UCORE_ECOS"),
             faux_val={"__tauon_compiler__": lambda v: v == "__compiler_iar_m16c__",
                       "__tauon_cpu_core__": lambda v: v in CIBLES_GELEES_COEURS,
                       "__tauon_cpu_device__": lambda v: v.startswith(
                           ("__tauon_cpu_device_arm7_", "__tauon_cpu_device_arm9_",
                            "__tauon_cpu_device_win32_simulation__"))},
             motifs=[r"\b__compiler_iar_m16c__\b", r"\b__tauon_cpu_device_arm[79]_\w+",
                     r"\b__tauon_cpu_device_win32_simulation__\b"]
             + [r"\b%s\b" % c for c in CIBLES_GELEES_COEURS])
# compilateurs : GCC seul (compiler.h) ; Keil et Visual C (simulation Win32) jamais
GCC = Preds("compilateur", ("__CC_ARM", "_MSC_VER"), ("__GNUC__",),
            faux_val={"__tauon_compiler__": lambda v: v in ("__compiler_keil_arm__", "__compiler_win32__")},
            vrai_val={"__tauon_compiler__": lambda v: v == "__compiler_gnuc__"},
            motifs=[r"\b__compiler_(keil_arm|win32|gnuc)__\b"])
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


def value_of(n, ctx=ARM):
    """Valeur entière connue d'un opérande sous GCC, ou None."""
    n = strip_paren(n)
    if n.op != "atom":
        return None
    t = n.args[0]
    if t in ctx.macros:
        return 0                                   # non défini sous GCC
    m = re.fullmatch(r"(0[xX][0-9a-fA-F]+|\d+)[uUlL]*", t)
    return int(m.group(1), 0) if m else None


def evaluate(n, ctx=ARM):
    """Rend (valeur T/F/U, nœud simplifié, touché) ; touché : un prédicat de ctx a été décidé."""
    op = n.op
    if op == "paren":
        v, s, t = evaluate(n.args[0], ctx)
        return v, (s if s.op in ("atom", "defined", "paren", "not") else Node("paren", s)), t
    if op in ("defined", "atom"):
        if n.args[0] in ctx.macros:
            return F, n, True
        if n.args[0] in ctx.vrais:
            return T, n, True
        return U, n, False
    if op == "not":
        v, s, t = evaluate(n.args[0], ctx)
        return {T: F, F: T, U: U}[v], Node("not", s), t
    if op == "rel":
        a, rel, b = n.args
        names = {strip_paren(a).args[0] if strip_paren(a).op == "atom" else None,
                 strip_paren(b).args[0] if strip_paren(b).op == "atom" else None}
        if rel in ("==", "!=") and None not in names and len(names) == 2:
            x, y = [strip_paren(z).args[0] for z in (a, b)]
            v = ctx.decide_rel(x, y) or ctx.decide_rel(y, x)
            if v is not None:
                return (v if rel == "==" else {T: F, F: T}[v]), n, True
        va, vb = value_of(a, ctx), value_of(b, ctx)
        iar = any(strip_paren(x).op == "atom" and strip_paren(x).args[0] in ctx.macros
                  for x in (a, b))
        if iar and va is not None and vb is not None:
            r = {"==": va == vb, "!=": va != vb, "<": va < vb, ">": va > vb,
                 "<=": va <= vb, ">=": va >= vb}[rel]
            return (T if r else F), n, True
        return U, n, False
    if op in ("and", "or"):
        va, sa, ta = evaluate(n.args[0], ctx)
        vb, sb, tb = evaluate(n.args[1], ctx)
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


# pragma IAR d'alignement doublé par l'alignement GCC de la déclaration qui suit le #endif
RE_ALIGN_GCC = re.compile(r"__ALIGN_BEGIN|__ALIGN_END|__lepton_align\s*\(|aligned\s*\(")
RE_PRAGMA = re.compile(r"_Pragma\s*\(|#\s*pragma\b(.*)")


def pragmas_redondants(texte, apres):
    """Vrai si la branche ne contient que des « #pragma data_alignment » et que la ligne suivant
    le #endif porte l'alignement GCC équivalent."""
    prags = [m.group(1) or "_Pragma" for m in RE_PRAGMA.finditer(texte)]
    return (bool(prags) and all(re.match(r"\s*data_alignment\b", p) for p in prags)
            and bool(RE_ALIGN_GCC.search(apres)))


def rewrite(lines, items, path, auto, resid, ctx=ARM):
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
                v, simp, touched = evaluate(Parser(tokenize(expr)).parse(), ctx)
            except ValueError as err:
                v, simp, touched = U, None, False
                if ctx.mentionne(expr):
                    resid.append((line, "condition non analysée (%s)" % err, expr.strip()))
            if not touched:
                if ctx is ARM and IAR_OTHER.search(expr):
                    resid.append((line, "IAR M16C (règle garde-cible-gelee)", expr.strip()))
                elif ctx.mentionne(expr):
                    resid.append((line, "condition %s non décidable" % ctx.nom, expr.strip()))
                kept.append([kind, cond, (s, e), body, indent, None])
                all_false_so_far = False
                continue
            if v == F:
                texte = body_text(lines, body)
                apres = "".join(lines[it.endif[1]:it.endif[1] + 1])
                if ctx is ARM and RE_PRAGMA.search(texte) and not pragmas_redondants(texte, apres):
                    resid.append((line, "branche IAR avec _Pragma/#pragma (placement, étape 5)",
                                  expr.strip()))
                    kept.append([kind, cond, (s, e), body, indent, None])
                    all_false_so_far = False
                elif ctx is ARM and RE_PRAGMA.search(texte):
                    auto.append((line, "branche IAR retirée (data_alignment doublé par "
                                 "l'alignement GCC)", expr.strip()))
                else:
                    auto.append((line, "branche %s retirée" % ctx.nom, expr.strip()))
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
            out.extend(rewrite(lines, body, path, auto, resid, ctx))
        if kept:
            out.extend(lines[it.endif[0]:it.endif[1]])
        elif unwrapped is not None:
            out.extend(rewrite(lines, unwrapped, path, auto, resid, ctx))
    return out


def _gardes(text, path, ctx):
    lines = text.splitlines(keepends=True)
    spans = logical_lines(lines)
    items, _ = parse_block(lines, spans, 0, True)
    auto, resid = [], []
    new = "".join(rewrite(lines, items, path, auto, resid, ctx))
    return new, auto, resid


def rule_garde_iar_arm(text, path):
    return _gardes(text, path, ARM)


def rule_garde_cible_gelee(text, path):
    """Branches des cibles gelées ; la copie à l'identique est faite par main() (--legacy-dir)."""
    return _gardes(text, path, GELE)


def rule_garde_compilateur(text, path):
    """Gardes de compilateur sous GCC seul (GCC vrai ; Keil, Visual C faux) ; copie d'origine par
    main() si du code non GCC est retiré."""
    return _gardes(text, path, GCC)


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
        out.append(line)
    return "".join(out), auto, resid


# ---------------------------------------------------------------------------------------------
# texte de code : commentaires et chaînes masqués (mêmes positions)
# ---------------------------------------------------------------------------------------------


def masquer(text):
    """Remplace commentaires et littéraux (chaîne, caractère) par des espaces, sauts de ligne
    conservés : les positions du texte masqué sont celles du texte d'origine."""
    out, i, n = [], 0, len(text)
    while i < n:
        c = text[i]
        if text.startswith("/*", i):
            j = text.find("*/", i + 2)
            j = n if j < 0 else j + 2
        elif text.startswith("//", i):
            j = text.find("\n", i)
            j = n if j < 0 else j
        elif c in "\"'":
            j = i + 1
            while j < n and text[j] != c and text[j] != "\n":
                j += 2 if text[j] == "\\" else 1
            j = min(n, j + 1)
        else:
            out.append(c)
            i += 1
            continue
        out.append("".join(ch if ch == "\n" else " " for ch in text[i:j]))
        i = j
    return "".join(out)


def num_ligne(text, pos):
    return text.count("\n", 0, pos) + 1


def ligne_de(text, pos):
    a = text.rfind("\n", 0, pos) + 1
    b = text.find("\n", pos)
    return text[a:b if b >= 0 else len(text)]


def lignes_sous_garde_iar(text):
    """Numéros de ligne (1-based) situés dans une branche dont la condition mentionne un
    prédicat IAR (ARM ou M16C) : laissées aux règles de gardes, jamais retouchées par
    les règles de mots-clés et de pragmas."""
    lines = text.splitlines(keepends=True)
    try:
        items, _ = parse_block(lines, logical_lines(lines), 0, True)
    except ValueError:
        return set()
    out = set()

    def corps(body):
        for it in body:
            if isinstance(it, Cond):
                for b in it.branches:
                    out.update(range(b[2][0] + 1, b[2][1] + 1))
                    corps(b[3])
                out.update(range(it.endif[0] + 1, it.endif[1] + 1))
            else:
                out.update(range(it[0] + 1, it[1] + 1))

    def visiter(body):
        for it in body:
            if not isinstance(it, Cond):
                continue
            for kind, cond, span, b, indent in it.branches:
                expr = branch_expr(kind, cond)
                if ARM.mentionne(expr) or GELE.mentionne(expr):
                    corps(b)
                else:
                    visiter(b)
    visiter(items)
    return out


def est_directive(masque, pos):
    a = masque.rfind("\n", 0, pos) + 1
    return masque[a:pos].lstrip().startswith("#")


# ---------------------------------------------------------------------------------------------
# en-têtes et symboles IAR : signalés (résiduels)
# ---------------------------------------------------------------------------------------------
RE_HEADER_IAR = re.compile(
    r"^[ \t]*#[ \t]*include[ \t]*[<\"]((?:\w+/)*(?:intrinsics|yvals|ysizet|DLib_\w+|"
    r"io(?:at91|m16c|stm32|lm3s|sam)\w*|cmsis_iar|iccarm_builtin)\.h)[>\"]", re.M)


def rule_header_iar(text, path):
    resid = [(num_ligne(text, m.start()), "en-tête IAR %s : équivalent CMSIS/newlib ou branche "
              "gelée à traiter à la main" % m.group(1), m.group(0).strip())
             for m in RE_HEADER_IAR.finditer(text)]
    return text, [], resid


RE_SYMBOLE_IAR = re.compile(r"\b(__iar_\w+|__ICFEDIT_\w+)")


def rule_symbole_iar(text, path):
    masque = masquer(text)
    resid = []
    for m in RE_SYMBOLE_IAR.finditer(masque):
        resid.append((num_ligne(text, m.start()), "symbole IAR %s (bibliothèque DLIB / éditeur de "
                      "liens) : équivalent newlib, Lepton ou .ld à choisir" % m.group(1),
                      ligne_de(text, m.start()).strip()))
    return text, [], resid


# ---------------------------------------------------------------------------------------------
# mots-clés IAR → macros de compiler.h
# ---------------------------------------------------------------------------------------------
MOTS_CLES = {"__no_init": "__lepton_no_init", "__root": "__lepton_used",
             "__weak": "__lepton_weak", "__ramfunc": "__lepton_ramfunc",
             "__noreturn": "__lepton_noreturn"}
RE_MOT_CLE = re.compile(r"\b(__packed|%s)\b" % "|".join(MOTS_CLES))
RE_INCLUDE = re.compile(r"^[ \t]*#[ \t]*include\b.*$", re.M)
INCLUDE_COMPILER = '#include "kernel/core/compiler.h"'


def ajouter_include_compiler(text, auto, resid):
    """Insère l'inclusion de compiler.h après la première inclusion hors #if (niveau 0)."""
    if re.search(r"#\s*include\s*[<\"](kernel/core/)?(kernel_)?compiler\.h[>\"]", text):
        return text
    profondeur, pos = 0, 0
    for line in text.splitlines(keepends=True):
        d = DIRECTIVE.match(line.rstrip("\r\n"))
        if d and d.group(2) in ("if", "ifdef", "ifndef"):
            profondeur += 1
        elif d and d.group(2) == "endif":
            profondeur -= 1
        pos += len(line)
        # niveau 0, ou niveau 1 dans un fichier d'en-tête (garde d'inclusion #ifndef X_H)
        if RE_INCLUDE.match(line) and (profondeur == 0 or
                                       (profondeur == 1 and text.lstrip().startswith(("#ifndef", "/*", "//")))):
            nl = "\r\n" if line.endswith("\r\n") else "\n"
            auto.append((num_ligne(text, pos - 1) + 1, "inclusion de compiler.h ajoutée", INCLUDE_COMPILER))
            return text[:pos] + INCLUDE_COMPILER + nl + text[pos:]
    # en-tête sans inclusion : après la garde d'inclusion « #ifndef X / #define X »
    g = re.search(r"^[ \t]*#[ \t]*ifndef[ \t]+(\w+)[ \t]*\r?\n[ \t]*#[ \t]*define[ \t]+\1\b.*?(\r?\n)",
                  text, re.M)
    if g and not text[:g.start()].strip("\r\n\t ").replace("*/", "").count("#"):
        auto.append((num_ligne(text, g.end()), "inclusion de compiler.h ajoutée (après la garde)",
                     INCLUDE_COMPILER))
        return text[:g.end()] + INCLUDE_COMPILER + g.group(2) + text[g.end():]
    resid.append((1, "compiler.h à inclure à la main (aucune inclusion de niveau 0)", ""))
    return text


def rule_mot_cle_iar(text, path):
    masque = masquer(text)
    auto, resid = [], []
    morceaux, dernier = [], 0
    gardees = lignes_sous_garde_iar(text)
    for m in RE_MOT_CLE.finditer(masque):
        no, mot = num_ligne(text, m.start()), m.group(1)
        if no in gardees:
            continue
        ligne = ligne_de(text, m.start()).strip()
        if est_directive(masque, m.start()):
            resid.append((no, "%s dans une directive (définition de compatibilité ?)" % mot, ligne))
            continue
        if mot == "__packed":
            suite = re.match(r"\s*(struct|union)\b", masque[m.end():])
            if not suite:
                resid.append((no, "__packed hors struct/union (pointeur : accès non aligné, "
                              "sémantique à vérifier)", ligne))
                continue
            # « __packed struct » → « struct __lepton_packed »
            fin = m.end() + suite.end()
            morceaux.append(text[dernier:m.start()])
            morceaux.append(suite.group(1) + " __lepton_packed")
            dernier = fin
            auto.append((no, "__packed %s → %s __lepton_packed" % ((suite.group(1),) * 2), ligne))
            continue
        morceaux.append(text[dernier:m.start()])
        morceaux.append(MOTS_CLES[mot])
        dernier = m.end()
        auto.append((no, "%s → %s" % (mot, MOTS_CLES[mot]), ligne))
    if not auto:
        return text, auto, resid
    morceaux.append(text[dernier:])
    new = ajouter_include_compiler("".join(morceaux), auto, resid)
    return new, auto, resid


# ---------------------------------------------------------------------------------------------
# pragmas IAR
# ---------------------------------------------------------------------------------------------
PRAGMAS_RETIRES = {
    "optimize": "#pragma optimize retiré (ETAPE-4 : sauf nécessité démontrée)",
    "diag_suppress": "diagnostic IAR retiré", "diag_default": "diagnostic IAR retiré",
    "diag_warning": "diagnostic IAR retiré", "diag_error": "diagnostic IAR retiré",
    "diag_remark": "diagnostic IAR retiré",
    "rtmodel": "attribut de modèle d'exécution IAR retiré (sans équivalent GCC)",
}
PRAGMAS_RESIDUELS = {
    "location": "placement : __lepton_section sur la déclaration suivante",
    "data_alignment": "alignement : __lepton_align sur la déclaration suivante",
    "language": "extensions du langage IAR",
    "inline": "inline=forced : __attribute__((always_inline)) à poser",
    "required": "dépendance de symbole pour l'éditeur de liens",
    "segment": "segment IAR : script .ld", "section": "section IAR : script .ld",
    "memory": "modèle mémoire IAR (M16C)", "vector": "vecteur d'interruption IAR",
    "type_attribute": "attribut de type IAR", "object_attribute": "attribut d'objet IAR",
    "dataseg": "segment de données IAR", "constseg": "segment de constantes IAR",
    "swi_number": "appel SWI IAR", "basic_template_matching": "extension IAR",
    "calls": "graphe d'appels IAR", "call_graph_root": "graphe d'appels IAR",
    "default_function_attributes": "attributs par défaut IAR",
    "default_variable_attributes": "attributs par défaut IAR",
}
RE_PRAGMA_LIGNE = re.compile(r"^([ \t]*)#[ \t]*pragma[ \t]+(\w+)\b.*(?:\r?\n|$)", re.M)
RE_PRAGMA_OP = re.compile(r'_Pragma\s*\(\s*"\s*(\w+)\s*=?\s*(?:\\"([^"\\]*)\\")?[^)]*\)')


def sections_ld():
    racine = os.path.join(clone_root(), "scion", "ld")
    noms = set()
    for f in glob.glob(os.path.join(racine, "*.ld")):
        with open(f) as fh:
            txt = fh.read()
        noms.update(re.findall(r"\*\(\s*\.?([\w.]+)", txt))
        noms.update(re.findall(r"^\s*\.?([\w.]+)\s*(?:\(NOLOAD\))?\s*:", txt, re.M))
    return noms


def rule_pragma_iar(text, path):
    auto, resid = [], []
    masque = masquer(text)
    # lignes #pragma (directives : le masque garde le mot-clé du pragma)
    gardees = lignes_sous_garde_iar(text)
    morceaux, dernier = [], 0
    for m in RE_PRAGMA_LIGNE.finditer(masque):
        nom, no = m.group(2), num_ligne(text, m.start())
        if no in gardees:
            continue
        ligne = ligne_de(text, m.start()).strip()
        if nom in PRAGMAS_RETIRES:
            morceaux.append(text[dernier:m.start()])
            dernier = m.end()
            auto.append((no, PRAGMAS_RETIRES[nom], ligne))
        elif nom in PRAGMAS_RESIDUELS:
            resid.append((no, "#pragma %s : %s" % (nom, PRAGMAS_RESIDUELS[nom]), ligne))
    morceaux.append(text[dernier:])
    text = "".join(morceaux)
    # opérateur _Pragma("location=\"S\"") (dans le texte d'origine : la chaîne est l'argument)
    morceaux, dernier = [], 0
    sections = None
    gardees = lignes_sous_garde_iar(text)
    for m in RE_PRAGMA_OP.finditer(text):
        nom, arg, no = m.group(1), m.group(2), num_ligne(text, m.start())
        if no in gardees:
            continue
        if nom == "location" and arg:
            morceaux.append(text[dernier:m.start()])
            morceaux.append('__lepton_section("%s")' % arg)
            dernier = m.end()
            auto.append((no, "_Pragma location → __lepton_section", arg))
            if sections is None:
                sections = sections_ld()
            if arg.lstrip(".") not in {x.lstrip(".") for x in sections}:
                resid.append((no, "section %s absente des scripts ld/*.ld (carte, étape 5)" % arg,
                              ligne_de(text, m.start()).strip()))
        elif nom in PRAGMAS_RESIDUELS or nom in PRAGMAS_RETIRES:
            resid.append((no, "_Pragma %s : %s" % (nom, PRAGMAS_RESIDUELS.get(nom, PRAGMAS_RETIRES.get(nom))),
                          ligne_de(text, m.start()).strip()))
    morceaux.append(text[dernier:])
    new = "".join(morceaux)
    if any(a[1].startswith("_Pragma location") for a in auto):
        new = ajouter_include_compiler(new, auto, resid)
    return new, auto, resid


RE_DEF_STATIC = re.compile(r"^[ \t]*static\b[^;{}()=#]*?\b(\w+)\s*\([^;{}]*\)\s*\{", re.M)


def rule_prototype_static(text, path):
    """Ajoute « static » aux prototypes non static d'une fonction définie static plus loin."""
    if not path.endswith(".c"):
        return text, [], []
    masque = masquer(text)
    auto, ajouts = [], []
    for d in RE_DEF_STATIC.finditer(masque):
        nom = d.group(1)
        proto = re.compile(r"^([ \t]*)(?!static\b)(?!return\b)((?:extern[ \t]+)?[A-Za-z_][^;{}()=#]*?\b%s"
                           r"\s*\([^;{}]*\)\s*;)" % re.escape(nom), re.M)
        for m in proto.finditer(masque, 0, d.start()):
            ajouts.append((m.start(2), m.group(2).startswith("extern")))
            auto.append((num_ligne(text, m.start()), "prototype rendu static (%s)" % nom,
                         ligne_de(text, m.start()).strip()))
    for pos, ext in sorted(set(ajouts), reverse=True):
        if ext:
            text = text[:pos] + "static" + text[pos + len("extern"):]
        else:
            text = text[:pos] + "static " + text[pos:]
    return text, sorted(auto), []


RULES = {"garde-iar-arm": rule_garde_iar_arm, "garde-cible-gelee": rule_garde_cible_gelee,
         "garde-compilateur": rule_garde_compilateur,
         "header-iar": rule_header_iar, "intrinsics-cmsis": rule_intrinsics_cmsis,
         "mot-cle-iar": rule_mot_cle_iar, "pragma-iar": rule_pragma_iar,
         "prototype-static": rule_prototype_static,
         "symbole-iar": rule_symbole_iar}
# ordre d'application : gardes d'abord (une branche IAR retirée n'a plus de mots-clés à traiter)
ORDRE = ["garde-cible-gelee", "garde-iar-arm", "garde-compilateur", "intrinsics-cmsis", "mot-cle-iar", "pragma-iar",
         "header-iar", "symbole-iar", "prototype-static"]

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
        # sources en Latin-1 (arbre hérité) : décodage sans perte, identique avant/après
        r = subprocess.run(cmd + " -E -P -o -", shell=True, cwd=ent["directory"],
                           capture_output=True, encoding="latin-1")
        res[ent["file"]] = r.stdout if r.returncode == 0 else "ERREUR : " + r.stderr
    return res


def main():
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0],
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("files", nargs="*", help="fichiers relatifs au trunk (sys/root/src/…)")
    ap.add_argument("--files-from", help="liste de fichiers (un par ligne)")
    ap.add_argument("--perimetre-actif", metavar="CSV",
                    help="ajoute les .c/.h de l'ensemble actif de perimetre.csv")
    ap.add_argument("--legacy-dir", default="legacy",
                    help="garde-cible-gelee, garde-compilateur : copie d'origine sous scion/<DIR>/<chemin> (défaut legacy)")
    ap.add_argument("--tiers-from-audit", metavar="AUDIT_CSV",
                    help="rapport : ajoute les IAR-ismes du code tiers actif (non transformé, D1a)")
    ap.add_argument("--quiet", "-q", action="store_true", help="résumé seulement")
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
    if args.perimetre_actif:
        for row in csv.DictReader(open(args.perimetre_actif, encoding="utf-8")):
            f = row["fichier"]
            if row["ensemble"] == "actif" and f.endswith((".c", ".h")):
                files.append(f if f.startswith(("sys/", "tools/")) else "sys/root/" + f)
    rules = [r for r in ORDRE if r in (args.rule or ORDRE)]
    clone = clone_root()
    report = []
    n_auto = n_resid = n_changed = 0
    for rel in sorted(set(files)):
        if est_tiers(rel) or not rel.endswith((".c", ".h")):
            continue
        rel, path = resolve(clone, rel)
        with open(path, encoding="latin-1", newline="") as fh:   # fins de ligne conservées
            text = fh.read()
        new = text
        for r in rules:
            avant = new
            new, auto, resid = RULES[r](new, rel)
            if r in COPIE_LEGACY and code_retire(avant, new):
                copie = copie_legacy(clone, args.legacy_dir, rel, text, args.apply)
                auto.insert(0, (1, "copie à l'identique (code gelé)", copie))
            for (line, what, detail) in auto:
                report.append((rel, r, "automatique", line, what, detail))
            for (line, what, detail) in resid:
                report.append((rel, r, "résiduel", line, what, detail))
            n_auto += len(auto)
            n_resid += len(resid)
        if new != text:
            n_changed += 1
            if args.apply:
                with open(path, "w", encoding="latin-1", newline="") as fh:
                    fh.write(new)
    mode = "appliqué" if args.apply else "simulation"
    print("transform_iar (%s) : règles %s ; %d fichier(s) modifié(s), %d automatique(s), "
          "%d résiduel(s)" % (mode, ",".join(rules), n_changed, n_auto, n_resid))
    for row in ([] if args.quiet else report):
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
            if args.tiers_from_audit:
                ecrire_tiers(f, args.tiers_from_audit)
    return 0


COPIE_LEGACY = ("garde-cible-gelee", "garde-compilateur")


RE_COND = re.compile(r"^\s*#\s*(if|ifdef|ifndef|elif|else|endif)\b")


def code_retire(avant, apres):
    """Vrai si des lignes ont disparu hors directives conditionnelles (#if … #endif) : du code
    ou des définitions d'une branche retirée."""
    def code(t):
        return [l for l in t.splitlines() if l.strip() and not RE_COND.match(l)]
    return len(code(apres)) < len(code(avant))


def est_tiers(rel):
    return rel.startswith(TIERS) or bool(ORIGINE_TIERS.search(rel))


def copie_legacy(clone, legacy_dir, rel, text, apply):
    """Copie à l'identique du fichier d'origine (code gelé, D2a) ; jamais d'écrasement."""
    dest = os.path.join(clone, "scion", legacy_dir, rel)
    if apply and not os.path.exists(dest):
        os.makedirs(os.path.dirname(dest), exist_ok=True)
        with open(dest, "w", encoding="latin-1", newline="") as fh:
            fh.write(text)
    return os.path.join(legacy_dir, rel)


def ecrire_tiers(f, audit_csv):
    """Section du rapport : IAR-ismes du code tiers actif, non transformés (décision D1a)."""
    rows = [r for r in csv.DictReader(open(audit_csv, encoding="utf-8"))
            if r["ensemble"] == "actif" and r["severite"] == "iar" and r["origine"] == "tiers"]
    f.write("## Code tiers non modifié — justifiés (%d)\n\n" % len(rows))
    f.write("Décision D1a (2026-10-01) : le code vendored (CMSIS, HAL/driverlib ST, FatFs, yaffs…) "
            "n'est pas transformé ; ses branches IAR sont inactives sous GCC (en-têtes "
            "multi-compilateurs du fournisseur). Source : `%s`.\n\n" % os.path.basename(audit_csv))
    f.write("| Fichier | Ligne | Catégorie | Motif |\n|---|---:|---|---|\n")
    for r in rows:
        f.write("| `%s` | %s | %s | %s |\n" % (r["fichier"], r["ligne"], r["categorie"],
                                               r["motif"].replace("|", "\\|")))
    f.write("\n")


if __name__ == "__main__":
    sys.exit(main())
