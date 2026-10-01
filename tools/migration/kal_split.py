#!/usr/bin/env python3
"""kal_split.py — décomposition du KAL (ETAPE-4 tâche 3), rejouable, à contenu constant.

Opère sur le clone (scion/sys/root/src/kernel/core/kal.h), jamais sur le trunk. Les branches
sont repérées par leur condition préprocesseur (lignes logiques, continuations « \\ » comprises),
pas par numéro de ligne. Chaque étape vérifie ses préconditions et s'arrête sinon.

Étapes (une par commit, dans cet ordre) :
  gel             copie d'origine de kal.h et kal.c sous scion/legacy/ (décisions D2a/D3a) ;
                  retire les branches de premier niveau eCos (×3), Win32, M16C IAR, et le
                  bloc USE_DEBUG_KAL (Win32) ; supprime kal.c (entièrement sous CPU_WIN32).
  anciens-coeurs  retire, dans les branches embOS et FreeRTOS, les sous-branches ARM7/ARM9
                  (cœurs arm7tdmi/arm926ejs, puces AT91 : en-têtes ioat91*, PIT, profileur).
  extraire        éclate kal.h selon les axes :
                    kal/arch/armv7m/kal_arch.h        __va_list_copy AAPCS, SysTick (__stop_sched)
                    kal/arch/host/kal_arch.h          __va_list_copy du noyau statique
                    kal/backend/embos/kal_backend.h   branche embOS
                    kal/backend/static/kal_backend.h  branche du noyau statique
                    kal/backend/freertos/kal_backend.h  branche FreeRTOS telle quelle (étape 7)
                    kal/contrat.h                     branche #else (spécification, non incluse)
                  kal.h devient le dispatcher : « #include "kal_arch.h" » puis « "kal_backend.h" »,
                  trouvés par les chemins d'inclusion posés par cmake/isa/ et cmake/kal/.

Usage : kal_split.py <étape> [--clone <racine du clone>] [--dry-run]
Vérification : transform_iar.py --cpp-compare (gcc -E -P identique attendu à chaque étape).
"""
import argparse
import os
import re
import shutil
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
CORE = "sys/root/src/kernel/core"

DIRECTIVE = re.compile(r"^\s*#\s*(if|ifdef|ifndef|elif|else|endif)\b(.*)$", re.S)


# --- analyse des groupes conditionnels -----------------------------------------------------

def logical_directives(lines):
    """[(debut, fin_exclue, mot_cle, condition normalisée)] des directives conditionnelles."""
    out, i = [], 0
    while i < len(lines):
        j = i
        text = lines[i]
        while text.rstrip("\n").endswith("\\") and j + 1 < len(lines):
            j += 1
            text = text.rstrip("\n")[:-1] + " " + lines[j]
        m = DIRECTIVE.match(text)
        if m:
            cond = re.sub(r"//.*", "", m.group(2))
            cond = re.sub(r"/\*.*?\*/", "", cond)
            out.append((i, j + 1, m.group(1), " ".join(cond.split())))
        i = j + 1
    return out


class Group:
    def __init__(self, depth, parent):
        self.depth, self.parent, self.arms, self.endif = depth, parent, [], None

    def arm_body(self, k):
        """(début, fin) du corps du bras k (hors en-tête)."""
        end = self.arms[k + 1][0] if k + 1 < len(self.arms) else self.endif[0]
        return self.arms[k][1], end


def parse(lines):
    groups, stack = [], []
    for start, end, kw, cond in logical_directives(lines):
        if kw in ("if", "ifdef", "ifndef"):
            g = Group(len(stack), stack[-1] if stack else None)
            g.arms.append((start, end, kw, cond))
            groups.append(g)
            stack.append(g)
        elif kw in ("elif", "else"):
            stack[-1].arms.append((start, end, kw, cond))
        else:
            stack.pop().endif = (start, end)
    if stack:
        raise SystemExit("kal_split : #if non fermé")
    return groups


def find_arms(lines, pred, depth=None):
    """Bras dont la condition satisfait pred : [(groupe, indice)]."""
    res = []
    for g in parse(lines):
        if depth is not None and g.depth != depth:
            continue
        for k, arm in enumerate(g.arms):
            if pred(arm[3], g, k):
                res.append((g, k))
    return res


def remove_arm(lines, g, k):
    """Retire le bras k du groupe g ; renvoie les nouvelles lignes."""
    arms = g.arms
    if len(arms) == 1:
        return lines[:arms[0][0]] + lines[g.endif[1]:]
    if k == 0:
        nxt = arms[1]
        if nxt[2] == "elif":
            head = re.sub(r"#(\s*)elif\b", r"#\1if", lines[nxt[0]], count=1)
            return lines[:arms[0][0]] + [head] + lines[nxt[0] + 1:]
        if nxt[2] == "else" and len(arms) == 2:
            # le corps du #else devient inconditionnel
            return (lines[:arms[0][0]] + lines[nxt[1]:g.endif[0]] + lines[g.endif[1]:])
        raise SystemExit("kal_split : cas de retrait non géré")
    start = arms[k][0]
    end = arms[k + 1][0] if k + 1 < len(arms) else g.endif[0]
    if k == len(arms) - 1 and arms[k][2] == "else":
        return lines[:start] + lines[end:]
    return lines[:start] + lines[end:]


def remove_all(lines, pred, depth=None, label=""):
    n = 0
    while True:
        found = find_arms(lines, pred, depth)
        if not found:
            break
        g, k = found[0]
        lines = remove_arm(lines, g, k)
        n += 1
    print("  %-48s %d bras retiré(s)" % (label, n))
    return lines, n


# --- fichiers --------------------------------------------------------------------------------

def read(path):
    with open(path, encoding="utf-8", newline="") as f:
        return f.read().splitlines(keepends=True)


def write(path, lines, dry):
    if dry:
        print("  (dry-run) écrirait %s" % path)
        return
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, "w", encoding="utf-8", newline="") as f:
        f.writelines(lines)


def legacy_copy(clone, rel, dry):
    src = os.path.join(clone, "scion", rel)
    dst = os.path.join(clone, "scion", "legacy", rel)
    if os.path.exists(dst):
        print("  copie d'origine déjà présente : legacy/%s" % rel)
        return
    if not dry:
        os.makedirs(os.path.dirname(dst), exist_ok=True)
        shutil.copy2(src, dst)
    print("  copie d'origine : legacy/%s" % rel)


def top(lines):
    """Groupe de premier niveau du KAL (profondeur 1, sous la garde _KAL_H)."""
    gs = [g for g in parse(lines) if g.depth == 1 and len(g.arms) > 2]
    if len(gs) != 1:
        raise SystemExit("kal_split : groupe de premier niveau introuvable ou ambigu")
    return gs[0]


# --- étape gel -------------------------------------------------------------------------------

def is_gele_top(cond, g, k):
    return g.depth == 1 and (
        "__KERNEL_UCORE_ECOS" in cond
        or re.search(r"CPU_ARM[79]\b", cond) is not None
        or cond.startswith("defined(CPU_WIN32)")
        or "CPU_M16C62" in cond)


def step_gel(clone, dry):
    kal_h = os.path.join(clone, "scion", CORE, "kal.h")
    kal_c = os.path.join(clone, "scion", CORE, "kal.c")
    lines = read(kal_h)
    if not find_arms(lines, is_gele_top, 1):
        raise SystemExit("gel : aucune branche gelée (étape déjà appliquée ?)")
    legacy_copy(clone, CORE + "/kal.h", dry)
    lines, _ = remove_all(lines, is_gele_top, 1, "branches eCos, Win32, M16C")
    lines, _ = remove_all(lines, lambda c, g, k: k == 0 and c == "USE_DEBUG_KAL", 1,
                          "bloc USE_DEBUG_KAL (Win32)")
    write(kal_h, lines, dry)
    if os.path.exists(kal_c):
        body = "".join(read(kal_c))
        if "#ifdef CPU_WIN32" not in body:
            raise SystemExit("gel : kal.c n'est plus entièrement Win32, vérifier à la main")
        legacy_copy(clone, CORE + "/kal.c", dry)
        if dry:
            print("  (dry-run) supprimerait kal.c")
            return
        subprocess.run(["git", "-C", clone, "rm", "-q", "scion/" + CORE + "/kal.c"], check=True)
        print("  kal.c supprimé (copie sous legacy/)")


# --- étape anciens-coeurs --------------------------------------------------------------------

ANCIENS = re.compile(r"__tauon_cpu_device_arm[79]_|__tauon_cpu_core_arm_arm(7tdmi|926ejs)__")
CORTEXM_NE = re.compile(r"^\(__tauon_cpu_core__ != __tauon_cpu_core_arm_cortexM3__\)")


def is_ancien(cond, g, k):
    if g.depth < 2:
        return False
    if CORTEXM_NE.match(cond):
        return True  # bloc réservé aux cœurs non Cortex-M (OS_Priv.h)
    if not ANCIENS.search(cond):
        return False
    # bras ne citant que des cibles gelées (aucun cœur Cortex-M dans la condition)
    return "cortexM" not in cond


def step_anciens(clone, dry):
    kal_h = os.path.join(clone, "scion", CORE, "kal.h")
    lines = read(kal_h)
    if find_arms(lines, is_gele_top, 1):
        raise SystemExit("anciens-coeurs : appliquer d'abord l'étape gel")
    if not find_arms(lines, is_ancien):
        raise SystemExit("anciens-coeurs : rien à retirer (étape déjà appliquée ?)")
    lines, _ = remove_all(lines, is_ancien, None, "sous-branches ARM7/ARM9/AT91")
    # commentaires d'en-tête des blocs de profilage AT91 retirés (orphelins), avec la ligne vide
    orphan = re.compile(r"^\s*//profiler macros for arm[79] \(at91")
    out, n, i = [], 0, 0
    while i < len(lines):
        if orphan.match(lines[i]):
            n += 1
            i += 2 if i + 1 < len(lines) and not lines[i + 1].strip() else 1
            continue
        out.append(lines[i])
        i += 1
    print("  %-48s %d" % ("commentaires orphelins retirés", n))
    write(kal_h, out, dry)


# --- étape extraire --------------------------------------------------------------------------

LICENCE_END = "*/\n"


def licence(lines):
    i = lines.index(LICENCE_END)
    return lines[:i + 1]


def header(lic, guard, desc):
    return lic + ["\n", "\n"] + ["//" + d + "\n" for d in desc] + [
        "#ifndef %s\n" % guard, "#define %s\n" % guard, "\n"]


def footer(guard):
    return ["\n", "#endif //%s\n" % guard]


def take(body, pred_start, n_lines=None, until=None):
    """Retire de body un passage commençant à la première ligne satisfaisant pred_start."""
    for i, l in enumerate(body):
        if pred_start(l):
            if n_lines is not None:
                j = i + n_lines
            else:
                j = i
                while not until(body[j]):
                    j += 1
                j += 1
            return body[:i] + body[j:], body[i:j]
    raise SystemExit("extraire : passage introuvable")


def step_extraire(clone, dry):
    kal_h = os.path.join(clone, "scion", CORE, "kal.h")
    lines = read(kal_h)
    if find_arms(lines, is_gele_top, 1) or find_arms(lines, is_ancien):
        raise SystemExit("extraire : appliquer d'abord gel et anciens-coeurs")
    g = top(lines)
    conds = [a[3] for a in g.arms]
    kinds = [a[2] for a in g.arms]
    if not (len(g.arms) == 4 and "USE_KERNEL_STATIC" in conds[0]
            and "__KERNEL_UCORE_EMBOS" in conds[1] and "__KERNEL_UCORE_FREERTOS" in conds[2]
            and kinds[3] == "else"):
        raise SystemExit("extraire : branches inattendues : %r" % conds)
    body = [lines[slice(*g.arm_body(k))] for k in range(4)]
    static, embos, freertos, contrat = body
    lic = licence(lines)
    kal = os.path.join(clone, "scion", CORE, "kal")
    va = lambda l: l.lstrip().startswith("#define __va_list_copy(")

    # arch/host : __va_list_copy du noyau statique
    static, va_host = take(static, va, 1)
    # arch/armv7m : __va_list_copy AAPCS et SysTick (bloc conditionnel M3/M4/M7, toujours vrai
    # sur armv7m : seules les définitions sont reprises)
    embos, va_armv7m = take(embos, va, 1)
    embos, systick = take(embos, lambda l: l.lstrip().startswith("//GD all Cortex-M3"),
                          until=lambda l: l.lstrip().startswith("#endif"))
    systick = [systick[0]] + [l for l in systick[1:]
                              if l.lstrip().startswith("#define")]

    files = {
        "arch/host/kal_arch.h": (
            "_KAL_ARCH_HOST_H",
            ["KAL, axe ISA : hôte x86 (noyau statique de mklepton, -m32).",
             "Extrait de kal.h (kal_split.py, étape extraire) ; sélectionné par cmake/isa/host.cmake."],
            va_host),
        "arch/armv7m/kal_arch.h": (
            "_KAL_ARCH_ARMV7M_H",
            ["KAL, axe ISA : ARMv7-M (Cortex-M3, M4, M7), indépendant du micro-noyau.",
             "Extrait de kal.h (kal_split.py, étape extraire) ; sélectionné par cmake/isa/armv7m.cmake."],
            va_armv7m + ["\n"] + systick),
        "backend/embos/kal_backend.h": (
            "_KAL_BACKEND_EMBOS_H",
            ["KAL, axe micro-noyau : embOS (Segger), TCB et trame de pile embOS.",
             "Extrait de kal.h (kal_split.py, étape extraire) ; sélectionné par cmake/kal/embos.cmake.",
             "S'appuie sur kal_arch.h (inclus avant par kal.h)."],
            embos),
        "backend/static/kal_backend.h": (
            "_KAL_BACKEND_STATIC_H",
            ["KAL, axe micro-noyau : noyau statique sans ordonnanceur (mklepton).",
             "Extrait de kal.h (kal_split.py, étape extraire) ; sélectionné par cmake/kal/static.cmake."],
            static),
        "backend/freertos/kal_backend.h": (
            "_KAL_BACKEND_FREERTOS_H",
            ["KAL, axe micro-noyau : FreeRTOS. Branche de kal.h extraite telle quelle (kal_split.py),",
             "hors ARM7/ARM9 ; non compilée avant l'étape 7 (aucun cmake/kal/freertos.cmake).",
             "Ses #if de cœur (cpu_regs_t M0 / M3-M7, SysTick) sont à répartir dans kal/arch/ à l'étape 7.",
             "Condition d'origine : " + conds[2]],
            freertos),
        "contrat.h": (
            "_KAL_CONTRAT_H",
            ["Contrat du KAL : branche #else de kal.h (documentation Doxygen, macros vides).",
             "Spécification de ce que chaque kal/backend/<micro-noyau>/kal_backend.h doit définir.",
             "Non inclus par kal.h : référence pour l'ajout d'un micro-noyau (ajout-coeur.md §5)."],
            contrat),
    }
    for rel, (guard, desc, content) in files.items():
        path = os.path.join(kal, rel)
        if os.path.exists(path):
            raise SystemExit("extraire : %s existe déjà" % path)
        write(path, header(lic, guard, desc) + content + footer(guard), dry)
        print("  kal/%s : %d ligne(s) reprise(s)" % (rel, len(content)))

    dispatch = [
        "//Dispatcher du KAL (ETAPE-4 tâche 3, kal_split.py) : l'ISA et le micro-noyau sont choisis\n",
        "//par les chemins d'inclusion, posés par cmake/isa/<isa>.cmake (kal/arch/<famille>/) et par\n",
        "//cmake/kal/<backend>.cmake (kal/backend/<backend>/). Contrat : kal/contrat.h.\n",
        "#include \"kal_arch.h\"\n",
        "#include \"kal_backend.h\"\n",
    ]
    new = lines[:g.arms[0][0]] + dispatch + lines[g.endif[1]:]
    write(kal_h, new, dry)
    print("  kal.h : %d -> %d lignes" % (len(lines), len(new)))


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("etape", choices=["gel", "anciens-coeurs", "extraire"])
    ap.add_argument("--clone", default=os.path.abspath(os.path.join(HERE, "..", "..")))
    ap.add_argument("--dry-run", action="store_true")
    a = ap.parse_args()
    {"gel": step_gel, "anciens-coeurs": step_anciens, "extraire": step_extraire}[a.etape](
        a.clone, a.dry_run)


if __name__ == "__main__":
    sys.exit(main())
