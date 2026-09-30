#!/usr/bin/env python3
"""build_closure.py — matrice fichier × projet et délimitation du périmètre (étape 1, tâche 2).

Entrées : JSON produits par tools/migration/ewp_extract.py ($LEPTON_BUILD/etape-1/projets/) et le
trunk ($LEPTON_TRUNK, défaut ~/lepton/trunk, liens suivis, lecture seule).

Méthode :
 1. Chaque configuration de chaque projet reçoit un ensemble (actif / différé / gelé) par CONFIG_RULES.
 2. Fermeture par configuration : sources et en-têtes déclarés (existants), puis fermeture des
    #include (guillemets : répertoire du fichier puis includes de la configuration ; chevrons :
    includes de la configuration). Toutes les directives sont suivies, conditions #if ignorées
    (sur-approximation) ; résolution insensible à la casse (projets Windows).
 3. Ensemble d'un fichier source de l'arbre (.c .h .s .S .asm .s79 .cpp), dans l'ordre :
    a) règle de nature (HARD_RULES : simulations, ARM7/ARM9, M16C, backends/ports remplacés…) ;
    b) sinon le meilleur ensemble des configurations qui le déclarent ou l'incluent
       (actif > différé > gelé) ;
    c) sinon, pour un en-tête : ensemble majoritaire des fichiers classés en b) du même répertoire
       (puis du parent) — « rattaché par répertoire » ;
    d) sinon règle de répertoire (SOFT_RULES) ; e) sinon « hors-projet ».
 4. Volumétrie : cloc (--by-file, --skip-uniqueness) s'il est installé, sinon repli interne :
    lignes non vides hors commentaires (/* */, //, et « ; » « @ » en tête de ligne pour l'asm).

Chemins des sorties : relatifs à sys/root/ pour ce qui est sous sys/root/, sinon relatifs à la
racine du trunk (tools/…, sys/user/…).

Sorties : doc/migration/matrice-fichiers-projets.csv, perimetre.csv, perimetre.md, code-gele.md ;
intermédiaires dans $LEPTON_BUILD/etape-1/projets/ (closure.json, cloc-*.csv).
Usage (depuis la racine du clone, après ewp_extract.py) :
  python3 tools/migration/build_closure.py [--doc doc/migration] [--no-cloc]
"""
import argparse
import collections
import csv
import datetime
import json
import os
import re
import shutil
import subprocess
import sys

TRUNK = os.path.realpath(os.path.expanduser(os.environ.get("LEPTON_TRUNK", "~/lepton/trunk")))
BUILD = os.path.expanduser(os.environ.get("LEPTON_BUILD", "~/lepton/build"))
PDIR = os.path.join(BUILD, "etape-1", "projets")
SRC_EXT = {".c", ".h", ".s", ".S", ".asm", ".s79", ".cpp"}
RANK = {"actif": 3, "différé": 2, "gelé": 1}

# ---------------------------------------------------------------------------- règles
# (regex sur le chemin du projet, regex sur le nom de configuration, ensemble, justification)
CONFIG_RULES = [
    (r"at91sam9261|at91sam7x|at91m55800a", r".", "gelé", "projet ARM7/ARM9 (cible abandonnée)"),
    (r"tauon/tauon\.ewp$", r".", "gelé", "noyau EWARM 4.x, AT91SAM9261 (ARM9)"),
    (r"tauon/tauon_", r"arm926ejs", "gelé", "noyau, configuration ARM926EJ-S"),
    (r"samv7|same70", r".", "différé", "Cortex-M7 Atmel SAMV71/SAME70, référence M7 (décision 2026-09-30)"),
    (r"tauon/tauon_(7\.80|8\.40|9\.50)\.ewp$", r"^tauon-kernel-cortex-m4-debug$", "actif",
     "noyau, configuration Cortex-M4 embOS (socle)"),
    (r"dev_stm32f4xx_(7\.20|8\.40)\.ewp$", r"^Debug$", "actif", "pilotes STM32F4 (config. liée par les applications F4)"),
    (r"bsp/(discovery_f4|olimex_p407|stm32f469i-eval)/", r"^Debug$", "actif", "BSP STM32F4"),
    (r"tauon-basic_(stm32f4-olimex_p407|stm32f407-discovery|stm32f469i-eval)", r"^Debug$", "actif",
     "application STM32F4 embOS"),
    (r"tauon/tauon_", r"freertos", "différé", "noyau, configuration FreeRTOS (étape 7)"),
    (r"tauon/tauon_", r"cortex-m(3|7|0)", "différé", "noyau, configuration d'un autre cœur (étape 6)"),
    (r"tauon/tauon_", r".", "différé", "noyau, génération ancienne ou config. lib-debug (Cortex-M3)"),
    (r"tauon-basic_(stm32f4|stm32f407|stm32f469)", r"freertos", "différé", "application STM32F4 FreeRTOS (étape 7)"),
    (r"stm32f1xx|lm3s|stellaris|tauon-basic_cmsis", r".", "différé", "candidat M3 (étape 6)"),
    (r"samd20", r".", "différé", "candidat M0+ (étape 6)"),
    (r"stm32wl", r".", "différé", "STM32WL55 Nucleo, carte non listée (à décider)"),
    (r"baseboard-modem", r".", "différé", "variante STM32F4 + modem, non retenue"),
    (r"usb-core", r".", "différé", "USB device STM32F4, hors paliers des étapes 3-5"),
    (r"nfc", r".", "différé", "périphérique NFC PN7150 optionnel"),
    (r".", r".", "différé", "génération ancienne ou configuration Release (squelette)"),
]

# Règles de nature : priment sur les projets.  (regex sur chemin relatif au trunk, ensemble, justification)
HARD_RULES = [
    (r"^sys/root/src/kernel/dev/arch/(arm7|arm9)/", "gelé", "ARM7/ARM9 (cible abandonnée)"),
    (r"^sys/root/src/kernel/dev/arch/(gnu32|win32)/", "gelé", "simulation Linux/Windows"),
    (r"^sys/root/src/kernel/core/arch/win32/", "gelé", "simulation Windows"),
    (r"^sys/root/src/kernel/core/(windows|windef|winnt)\.h$", "gelé", "en-têtes Windows de la simulation"),
    (r"^sys/root/prj/vc-2010/", "gelé", "projet Visual Studio de la simulation"),
    (r"^tools/virtual_cpu/", "gelé", "simulation (virtual_cpu)"),
    (r"^tools/host/win32/", "gelé", "outillage hôte Windows (WinPcap)"),
    (r"^tools/host/debian/ecos/", "gelé", "port eCos (backend core-ecos absent de l'arbre)"),
    (r"^tools/mklepton/src/mklepton-w32\.c$", "gelé", "variante Windows de mklepton"),
    (r"^sys/root/src/kernel/net/lwip/ports/m16c/", "gelé", "M16C (cible abandonnée)"),
    (r"^sys/root/src/kernel/core/ucore/(embOSARM7|embOSW32)", "gelé", "embOS IAR ARM7/ARM9/Win32"),
    (r"^sys/root/src/kernel/core/ucore/embOSCX", "gelé",
     "embOS IAR Cortex-M (Segger) remplacé par le port GCC (third_party/embos) ; lecture seule (tâche 4)"),
    (r"^sys/user/tauon_sampleapp/hal/board_atmel_at91sam9261-ek/", "gelé", "carte AT91SAM9261-EK (ARM9)"),
    (r"^(sys/root/src/kernel/dev/arch/cortexm/k60n512|sys/user/tauon_sampleapp/hal/board_freescale_twrk60n512)/",
     "gelé", "Freescale K60 (TWR-K60N512), carte abandonnée, sans projet IAR (proposition)"),
    (r"^sys/root/src/kernel/(dev/arch/cortexm/at91samv7x|dev/bsp/(same70xplained|samv71xplained_ultra)|dev/arch/at91/softpack-lib)/",
     "différé", "Cortex-M7 Atmel SAMV71/SAME70, référence M7 (décision 2026-09-30)"),
    (r"^sys/root/src/kernel/core/(core-freertos|ucore/freeRTOS_)", "différé", "backend FreeRTOS (étape 7)"),
]

SOFT_RULES = [
    (r"^sys/root/src/kernel/dev/arch/at91/(at91lib|dev_at91_)", "gelé", "Atmel AT91 ARM7/ARM9"),
    (r"^sys/root/src/kernel/dev/arch/at91/asf/", "différé", "Atmel ASF SAMD20, candidat M0+ (étape 6)"),
    (r"^sys/root/src/kernel/dev/(arch/cortexm/at91samd20|bsp/samd20xplained_pro)/", "différé", "SAMD20, candidat M0+ (étape 6)"),
    (r"^sys/root/src/kernel/dev/arch/cortexm/(stm32f1xx|stellaris)/", "différé", "candidat M3 (étape 6)"),
    (r"^sys/root/src/kernel/dev/(arch/cortexm/stm32wlxx|bsp/stm32wl55jci_nucleo)/", "différé",
     "STM32WL55 Nucleo, carte non listée (à décider)"),
    (r"^sys/root/src/kernel/dev/bsp/discovery_f4-baseboard-modem/", "différé", "variante STM32F4 + modem"),
    (r"^sys/root/src/(kernel/dev/arch/all/i2c/nxp-nfc|lib/lib-nxpnfc)/", "différé", "périphérique NFC optionnel"),
    (r"^sys/root/src/kernel/(usb/stm32f4-usb-core|core/usb/stm32_usb_core)/", "différé", "USB device STM32F4"),
    (r"^tools/mklepton/src/", "actif", "mklepton, outil hôte porté à l'étape 2"),
]


def first_rule(rules, path):
    for rx, s, j in rules:
        if re.search(rx, path):
            return s, j
    return None


def config_set(project, cfg):
    for prx, crx, s, j in CONFIG_RULES:
        if re.search(prx, project) and re.search(crx, cfg, re.I):
            return s, j


def disp(p):
    """Chemin affiché : relatif à sys/root/ si possible, sinon au trunk."""
    return p[len("sys/root/"):] if p.startswith("sys/root/") else p


def short(project):
    return os.path.splitext(os.path.basename(project))[0]


# ---------------------------------------------------------------------------- arbre et includes
def scan_tree():
    files, lower = [], {}
    for d, dirs, fs in os.walk(TRUNK, followlinks=True):
        dirs.sort()
        for f in sorted(fs):
            rel = os.path.relpath(os.path.join(d, f), TRUNK)
            lower.setdefault(rel.lower(), rel)
            if os.path.splitext(f)[1] in SRC_EXT or os.path.splitext(f)[1].lower() in {".s", ".asm", ".s79"}:
                files.append(rel)
    return files, lower


INC_RE = re.compile(r'^[ \t]*#[ \t]*include[ \t]*([<"])([^>"\n]+)[>"]', re.M)
_inc_cache = {}


def includes_of(rel):
    if rel not in _inc_cache:
        try:
            txt = open(os.path.join(TRUNK, rel), encoding="latin-1").read()
        except OSError:
            txt = ""
        _inc_cache[rel] = [(m.group(1), m.group(2).strip().replace("\\", "/")) for m in INC_RE.finditer(txt)]
    return _inc_cache[rel]


def closure(start, inc_dirs, lower, stats):
    seen = set(start)
    todo = list(start)
    res_cache = {}
    while todo:
        f = todo.pop()
        cur = os.path.dirname(f)
        for kind, name in includes_of(f):
            key = (cur if kind == '"' else None, name)
            if key not in res_cache:
                dirs = ([cur] if kind == '"' else []) + inc_dirs
                hit = None
                for d in dirs:
                    cand = os.path.normpath(os.path.join(d, name)).lower()
                    if cand in lower:
                        hit = lower[cand]
                        break
                res_cache[key] = hit
            hit = res_cache[key]
            if hit is None:
                stats["unresolved"] += 1
                continue
            if hit not in seen:
                seen.add(hit)
                if os.path.splitext(hit)[1] in SRC_EXT or os.path.splitext(hit)[1].lower() in {".s", ".asm", ".s79"}:
                    todo.append(hit)
    return seen


# ---------------------------------------------------------------------------- volumétrie
def count_internal(rel):
    try:
        txt = open(os.path.join(TRUNK, rel), encoding="latin-1").read()
    except OSError:
        return 0
    asm = os.path.splitext(rel)[1].lower() in (".s", ".asm", ".s79")
    n, in_c = 0, False
    for line in txt.splitlines():
        s, code = line, False
        i = 0
        while i < len(s):
            if in_c:
                j = s.find("*/", i)
                if j < 0:
                    i = len(s)
                else:
                    in_c, i = False, j + 2
                continue
            if s.startswith("/*", i):
                in_c, i = True, i + 2
                continue
            if s.startswith("//", i):
                break
            if asm and s[i] in ";@" and not s[:i].strip():
                break
            if not s[i].isspace():
                code = True
            i += 1
        n += code
    return n


def volumetry(files, use_cloc):
    cloc = shutil.which("cloc") if use_cloc else None
    if cloc:
        os.makedirs(PDIR, exist_ok=True)
        lst = os.path.join(PDIR, "cloc-list.txt")
        rep = os.path.join(PDIR, "cloc-by-file.csv")
        with open(lst, "w") as fh:
            fh.write("\n".join(os.path.join(TRUNK, f) for f in files) + "\n")
        ver = subprocess.run([cloc, "--version"], capture_output=True, text=True).stdout.strip()
        subprocess.run([cloc, "--by-file", "--csv", "--quiet", "--skip-uniqueness", "--follow-links",
                        "--force-lang=Assembly,s79", "--force-lang=Assembly,asm", "--force-lang=Assembly,s",
                        "--force-lang=Assembly,S",
                        "--list-file=" + lst, "--report-file=" + rep], check=True, capture_output=True)
        loc = {}
        with open(rep, encoding="utf-8", errors="replace") as fh:
            for row in csv.reader(fh):
                if len(row) >= 5 and row[1] and row[0] != "SUM" and row[4].isdigit():
                    loc[os.path.relpath(row[1], TRUNK)] = int(row[4])
        missing = [f for f in files if f not in loc]
        for f in missing:
            loc[f] = count_internal(f)
        meth = ("cloc %s (--by-file --skip-uniqueness, colonne « code ») ; %d fichier(s) non reconnu(s) "
                "par cloc comptés par le repli interne" % (ver, len(missing)))
        return loc, meth
    loc = {f: count_internal(f) for f in files}
    return loc, ("repli interne (cloc absent) : lignes non vides hors commentaires /* */ et //, "
                 "et hors lignes asm commençant par « ; » ou « @ » ; chaînes non analysées")


def dir_key(p):
    q = p.split("/")
    pre = "/".join(q[:3])
    if p.startswith("sys/root/src/kernel/"):
        sub = q[4]
        if sub == "core":
            if len(q) > 6 and q[5] in ("ucore", "arch", "usb"):
                return "/".join(q[:7])
            if len(q) > 5 and (q[5].startswith("core-") or q[5] == "net"):
                return "/".join(q[:6])
            return "sys/root/src/kernel/core"
        if sub == "dev":
            if len(q) > 7 and q[5] == "arch":
                return "/".join(q[:8])
            if len(q) > 6 and q[5] == "bsp":
                return "/".join(q[:7])
            return "sys/root/src/kernel/dev"
        return "/".join(q[:6]) if len(q) > 6 else "/".join(q[:5])
    if p.startswith("sys/root/src/"):
        return "/".join(q[:5]) if q[3] == "lib" and len(q) > 5 else "/".join(q[:4])
    if p.startswith("sys/user/tauon_sampleapp/hal/"):
        return "/".join(q[:5])
    if p.startswith("sys/user/"):
        return "/".join(q[:3])
    if p.startswith("tools/"):
        return "/".join(q[:3]) if len(q) > 3 else "/".join(q[:2])
    return pre


def top_key(p):
    q = p.split("/")
    if p.startswith("sys/root/src/kernel/"):
        return "/".join(q[:5])
    if p.startswith("sys/root/"):
        return "/".join(q[:4])
    return "/".join(q[:2])


# ---------------------------------------------------------------------------- principal
def main():
    ap = argparse.ArgumentParser(description="Matrice fichier × projet et périmètre")
    ap.add_argument("--doc", default="doc/migration")
    ap.add_argument("--no-cloc", action="store_true", help="forcer le repli interne")
    a = ap.parse_args()
    idx_path = os.path.join(PDIR, "projets.json")
    if not os.path.isfile(idx_path):
        sys.exit("lancer d'abord tools/migration/ewp_extract.py (%s absent)" % idx_path)
    index = json.load(open(idx_path))
    projects = [json.load(open(os.path.join(PDIR, p["json"]))) for p in index["projects"]]
    files, lower = scan_tree()
    fileset = set(files)

    # 1-2. fermeture par configuration
    cfg_info = []                       # (projet, config, ensemble, justification)
    decl = collections.defaultdict(lambda: collections.defaultdict(set))   # f -> projet -> {cfg}
    incl = collections.defaultdict(lambda: collections.defaultdict(set))
    stats = collections.Counter()
    for p in projects:
        for c in p["configurations"]:
            s, j = config_set(p["project"], c["name"])
            cfg_info.append((p["project"], c["name"], s, j))
            start = {x for x in c["sources"] + c["headers"] if x in lower.values() or x.lower() in lower}
            start = {lower.get(x.lower(), x) for x in start}
            dirs = [i["path"] for i in c["includes"] + c["asm_includes"] if i["path"] and i["exists"]]
            for x in c["sources"] + c["headers"]:
                decl[x][p["project"]].add(c["name"])
            for x in closure(start, dirs, lower, stats) - start:
                incl[x][p["project"]].add(c["name"])
    cset = {(pr, cf): (s, j) for pr, cf, s, j in cfg_info}
    ncfg = collections.Counter(pr for pr, _, _, _ in cfg_info)

    # 3. ensembles
    per = {}
    for f in files:
        r = first_rule(HARD_RULES, f)
        best = None
        for src, how in ((decl, "déclaré"), (incl, "inclus")):
            for pr, cfgs in src.get(f, {}).items():
                for cf in cfgs:
                    s, j = cset[(pr, cf)]
                    cand = (RANK[s], how == "déclaré", s, "%s par %s/%s (%s)" % (how, short(pr), cf, j))
                    if best is None or cand[:2] > best[:2]:
                        best = cand
        if r:
            just = "règle : " + r[1]
            if best and RANK[best[2]] > RANK[r[0]]:
                just += " [prime sur : %s]" % best[3]
                stats["conflit:" + dir_key(f)] += 1
            per[f] = (r[0], just, "règle")
        elif best:
            per[f] = (best[2], best[3], "projet")
    for f in files:
        if f in per or not f.endswith(".h"):
            continue
        d = os.path.dirname(f)
        for level in (d, os.path.dirname(d)):
            votes = collections.Counter(per[g][0] for g in files if g in per and per[g][2] == "projet"
                                        and os.path.dirname(g) == level) if level else None
            if votes:
                s = max(votes, key=lambda k: (votes[k], RANK[k]))
                per[f] = (s, "rattaché par répertoire (%s)" % disp(level), "répertoire")
                break
    for f in files:
        if f in per:
            continue
        r = first_rule(SOFT_RULES, f)
        per[f] = (r[0], "règle de répertoire : " + r[1], "règle") if r else \
            ("hors-projet", "non déclaré par un projet, non inclus, sans règle", "aucun")

    loc, method = volumetry(files, not a.no_cloc)

    # sorties CSV
    os.makedirs(a.doc, exist_ok=True)
    with open(os.path.join(a.doc, "perimetre.csv"), "w", newline="", encoding="utf-8") as fh:
        wr = csv.writer(fh)
        wr.writerow(["fichier", "ensemble", "justification", "lignes_code", "nb_projets"])
        for f in sorted(files):
            s, j, _ = per[f]
            wr.writerow([disp(f), s, j, loc.get(f, 0), len(set(decl.get(f, {})) | set(incl.get(f, {})))])
    cols = [p["project"] for p in projects]
    rows = sorted(set(decl) | set(incl))
    with open(os.path.join(a.doc, "matrice-fichiers-projets.csv"), "w", newline="", encoding="utf-8") as fh:
        wr = csv.writer(fh)
        wr.writerow(["fichier", "existe", "ensemble"] + [disp(c) for c in cols])
        for f in rows:
            cells = []
            for pr in cols:
                d, i = decl.get(f, {}).get(pr, set()), incl.get(f, {}).get(pr, set()) - decl.get(f, {}).get(pr, set())
                parts = []
                if d:
                    parts.append("*" if len(d) == ncfg[pr] else ";".join(sorted(d)))
                if i:
                    parts.append("i:" + ("*" if len(i) == ncfg[pr] else ";".join(sorted(i))))
                cells.append(" ".join(parts))
            ex = f in fileset or os.path.exists(os.path.join(TRUNK, f))
            wr.writerow([disp(f), int(ex), per[f][0] if f in per else ""] + cells)
    with open(os.path.join(PDIR, "closure.json"), "w", encoding="utf-8") as fh:
        json.dump({"configs": cfg_info, "stats": stats, "method": method}, fh, ensure_ascii=False, indent=1)

    write_perimetre_md(a.doc, files, per, loc, method, cfg_info, projects, decl, stats, rows)
    write_gele_md(a.doc, files, per, loc, method)
    tot = collections.Counter()
    for f in files:
        tot[per[f][0]] += loc.get(f, 0)
    print("fichiers=%d matrice=%d lignes=%s méthode=%s" % (len(files), len(rows), dict(tot), method.split(" (")[0]))


# ---------------------------------------------------------------------------- Markdown
SETS = ["actif", "différé", "gelé", "hors-projet"]


def table_by(keyf, files, per, loc):
    agg = collections.defaultdict(lambda: collections.Counter())
    for f in files:
        k = keyf(f)
        s = per[f][0]
        agg[k][s + "#"] += 1
        agg[k][s] += loc.get(f, 0)
    return agg


def md_table(w, agg, label):
    w("| %s | %s | total lignes |" % (label, " | ".join("%s (fich. / lignes)" % s for s in SETS)))
    w("|---|" + "---|" * (len(SETS) + 1))
    for k in sorted(agg):
        v = agg[k]
        w("| `%s` | %s | %d |" % (disp(k), " | ".join("%d / %d" % (v[s + "#"], v[s]) if v[s + "#"] else "–" for s in SETS),
                                 sum(v[s] for s in SETS)))


def cfg_sources(projects, proj_rx, cfg_name):
    out = set()
    for p in projects:
        if re.search(proj_rx, p["project"]):
            for c in p["configurations"]:
                if c["name"] == cfg_name:
                    out |= set(c["sources"])
    return out


def grouped(w, paths, per, loc):
    g = collections.defaultdict(list)
    for f in sorted(paths):
        g[os.path.dirname(f)].append(f)
    for d in sorted(g):
        items = []
        for f in g[d]:
            tag = "" if f not in per else ("" if per[f][0] == "actif" else " [%s]" % per[f][0])
            items.append(os.path.basename(f) + tag + ("" if f in per or os.path.exists(os.path.join(TRUNK, f)) else " [ABSENT]"))
        w("- `%s/` : %s" % (disp(d), ", ".join(items)))


def write_perimetre_md(doc, files, per, loc, method, cfg_info, projects, decl, stats, rows):
    L = []
    w = L.append
    tot = collections.Counter()
    for f in files:
        tot[per[f][0]] += loc.get(f, 0)
        tot[per[f][0] + "#"] += 1
    w("# Périmètre de la migration (étape 1, tâche 2)")
    w("")
    w("Généré par `tools/migration/build_closure.py` le %s. Rejouer depuis la racine du clone : "
      "`python3 tools/migration/ewp_extract.py && python3 tools/migration/build_closure.py`. "
      "Détail par fichier : `perimetre.csv` ; matrice : `matrice-fichiers-projets.csv`." % datetime.date.today())
    w("")
    w("**Volumétrie — méthode : %s.** Fichiers pris en compte : `.c .h .s .S .asm .s79 .cpp` de tout le trunk "
      "(%d fichiers)." % (method, len(files)))
    w("")
    w("## Chiffres par ensemble")
    w("")
    w("| Ensemble | Fichiers | Lignes de code |")
    w("|---|---|---|")
    for s in SETS:
        w("| %s | %d | %d |" % (s, tot[s + "#"], tot[s]))
    w("| **total** | %d | %d |" % (len(files), sum(tot[s] for s in SETS)))
    w("")
    w("Provenance du classement : %s." % ", ".join(
        "%s = %d" % (k, v) for k, v in sorted(collections.Counter(per[f][2] for f in files).items())))
    w("")
    w("## Méthode")
    w("")
    w("1. Chaque configuration des 46 projets reçoit un ensemble (tableau ci-dessous).")
    w("2. Fermeture par configuration : fichiers déclarés puis `#include` suivis récursivement avec les "
      "includes de la configuration ; **toutes** les directives sont suivies, conditions `#if` ignorées "
      "(sur-approximation : un en-tête inclus sous `#ifdef` d'un autre cœur tombe dans l'ensemble de la "
      "configuration) ; %d résolutions non abouties, cumulées sur toutes les configurations (en-têtes de "
      "la bibliothèque IAR, fichiers absents)." % stats["unresolved"])
    w("3. Un fichier prend, dans l'ordre : la règle de nature (qui prime), sinon le meilleur ensemble des "
      "configurations qui le déclarent ou l'incluent (actif > différé > gelé), sinon (en-tête) l'ensemble "
      "majoritaire de son répertoire, sinon une règle de répertoire, sinon `hors-projet`.")
    w("")
    w("### Ensembles des configurations")
    w("")
    w("| Projet | Configuration | Ensemble | Justification |")
    w("|---|---|---|---|")
    for pr, cf, s, j in cfg_info:
        w("| `%s` | %s | %s | %s |" % (short(pr), cf, s, j))
    w("")
    w("### Règles de nature (priment sur les projets)")
    w("")
    for rx, s, j in HARD_RULES:
        w("- `%s` → **%s** : %s" % (rx, s, j))
    w("")
    w("Règles de répertoire (fichiers non atteints par un projet) :")
    w("")
    for rx, s, j in SOFT_RULES:
        w("- `%s` → **%s** : %s" % (rx, s, j))
    w("")
    conf = sorted((k[len("conflit:"):], v) for k, v in stats.items() if k.startswith("conflit:"))
    if conf:
        w("Fichiers qu'une configuration active ou différée atteint mais qu'une règle de nature classe plus bas "
          "(à vérifier : inclusions conditionnelles probables, ou code remplacé) :")
        w("")
        for k, v in conf:
            w("- `%s` : %d" % (disp(k), v))
        w("")
    w("## Volumétrie par répertoire principal")
    w("")
    md_table(w, table_by(top_key, files, per, loc), "Répertoire")
    w("")
    w("### Détail")
    w("")
    md_table(w, table_by(dir_key, files, per, loc), "Répertoire")
    w("")

    # listes dérivées
    kern = set()
    for v in ("7.80", "8.40", "9.50"):
        kern |= cfg_sources(projects, r"tauon/tauon_%s\.ewp$" % re.escape(v), "tauon-kernel-cortex-m4-debug")
    soc = re.compile(r"sys/root/src/kernel/(dev/arch/cortexm/|dev/bsp/)|ucore/cmsis[^/]*/Device/")
    socle = {f for f in kern if not soc.search(f)}
    kern_soc = sorted(kern - socle)
    dev = cfg_sources(projects, r"dev_stm32f4xx_7\.20\.ewp$", "Debug")
    bsp = cfg_sources(projects, r"bsp/olimex_p407/", "Debug")
    app = cfg_sources(projects, r"tauon-basic_stm32f4-olimex_p407", "Debug")
    w("## Liste de sources dérivée : `mps2-an386` (QEMU, Cortex-M4)")
    w("")
    w("Aucun projet IAR. Dérivation : union des sources de la configuration `tauon-kernel-cortex-m4-debug` "
      "de `tauon_7.80/8.40/9.50.ewp` (listes identiques à `sbin/read.c` près, présent dans 8.40 seulement) "
      "**moins** tout fichier propre à un SoC ou une carte (`dev/arch/cortexm/`, `dev/bsp/`, "
      "`ucore/cmsis*/Device/`). Retirés : %s." % (", ".join("`%s`" % disp(f) for f in kern_soc) or "aucun"))
    w("")
    w("- Sources retenues : **%d** (%d lignes) ; ensembles : %s." % (
        len(socle), sum(loc.get(f, 0) for f in socle),
        ", ".join("%s = %d" % kv for kv in sorted(collections.Counter(per.get(f, ("absent",))[0] for f in socle).items()))))
    w("- Manquent (à écrire ou à fournir) : pilote UART CMSDK (console `lsh`), pilote Ethernet LAN9118 "
      "(aucun pilote `lan9118`/`smsc911x`/CMSDK dans l'arbre, vérifié par recherche), startup + script de "
      "liaison GCC et `RTOSInit` (paquet embOS GCC), `board`/`cpu` MPS2 (horloge SysTick), et les sorties "
      "mklepton (`kernel/core/arch/cortexm/{kernel_mkconf.h,dev_mkconf.c,bin_mkconf.c,dev_dskimg.[ch]}`).")
    w("- HYPOTHÈSE À VALIDER : les pilotes génériques `dev/arch/all/*` et `dev/arch/cmsis/*` de la "
      "configuration noyau sont conservés tels quels (ils se compilent sans SoC ; leur instanciation dépend "
      "de `dev_mkconf.c` généré).")
    w("")
    grouped(w, socle, per, loc)
    w("")
    w("## Liste de sources dérivée : NUCLEO-F439ZI")
    w("")
    w("Dérivation : socle ci-dessus + fichiers SoC de la configuration noyau + `dev_stm32f4xx_7.20.ewp` "
      "(Debug, bibliothèque liée par les applications F4) + `bsp_olimex_p407_7.30.ewp` (Debug) + application "
      "`tauon-basic_stm32f4-olimex_p407_7.20.ewp` (Debug).")
    w("")
    w("HYPOTHÈSE À VALIDER : l'Olimex STM32-P407 est retenue comme projet STM32F4 le plus proche (Ethernet "
      "présent comme sur la NUCLEO-F439ZI ; BSP le plus complet : `uart_3`, `spi_3` ; même famille "
      "F40x/F41x que la base de registres F42x/F43x pour UART/ETH). La STM32F4-Discovery n'a pas d'Ethernet "
      "embarqué ; la STM32F469I-EVAL est une F469. Brochage, horloges (180 MHz) et PHY sont à reprendre "
      "pour la F439 (étape 5).")
    w("")
    f439 = socle | set(kern_soc) | dev | bsp | app
    w("- Sources : **%d** (%d lignes existantes) ; ensembles : %s." % (
        len(f439), sum(loc.get(f, 0) for f in f439),
        ", ".join("%s = %d" % kv for kv in sorted(collections.Counter(per.get(f, ("absent",))[0] for f in f439).items()))))
    w("- Éléments `[gelé]` de la liste (embOS IAR : `RTOSInit_STM32F4x_CMSIS.c`, `OS_Error.c`, `xmtx*.c`, "
      "`JLINKMEM_Process.c`, `main.c` d'exemple et `os7m_tl__sp.a`) : à remplacer par leurs équivalents du "
      "paquet embOS GCC ; `startup_stm32f4xx.s` (syntaxe IAR) : à remplacer par un startup GCC.")
    w("- Éléments `[ABSENT]` : sorties mklepton à générer (étape 2).")
    w("- Anomalie : le noyau (`tauon-kernel-cortex-m4-debug`) compile avec les en-têtes `embOSCXM4_518/inc` "
      "puis `embOSCXM4_440/Inc`, alors que les applications F4 lient `embOSCXM4_386/Lib/os7m_tl__sp.a` "
      "et ses sources `arch/cmsis` : versions d'embOS incohérentes, à examiner en tâche 4.")
    w("- Anomalie : la configuration noyau Cortex-M4 déclare `dev/arch/cortexm/stellaris/drivers/dev_lm3s_cpu/"
      "dev_lm3s_cpu.c` (pilote CPU LM3S) ; conservé dans la liste par fidélité au projet, à retirer ou "
      "remplacer par `dev_stm32f4xx_cpu_x.c` à l'étape 5 (HYPOTHÈSE À VALIDER).")
    w("")
    grouped(w, f439 - socle, per, loc)
    w("")
    ho = table_by(dir_key, [f for f in files if per[f][0] == "hors-projet"], per, loc)
    w("## Fichiers hors-projet (principaux répertoires)")
    w("")
    w("Non déclarés, non inclus, sans règle : bibliothèques tierces partiellement utilisées (CMSIS, HAL "
      "CubeMX, uIP/lwIP), outils et applications d'exemple. À revoir si un palier en a besoin.")
    w("")
    for k, v in sorted(ho.items(), key=lambda kv: -kv[1]["hors-projet"])[:25]:
        w("- `%s` : %d fichiers, %d lignes" % (disp(k), v["hors-projet#"], v["hors-projet"]))
    w("")
    with open(os.path.join(doc, "perimetre.md"), "w", encoding="utf-8") as fh:
        fh.write("\n".join(L) + "\n")


def write_gele_md(doc, files, per, loc, method):
    L = []
    w = L.append
    w("# Code gelé — décidé le 2026-09-30")
    w("")
    w("Généré par `tools/migration/build_closure.py` le %s. Rien n'est supprimé ni déplacé (suppression : décision de l'étape 6). Le code gelé n'est "
      "ni modifié, ni compilé, ni audité au-delà de l'inventaire. Volumétrie : %s." % (datetime.date.today(), method))
    w("")
    gel = [f for f in files if per[f][0] == "gelé"]
    w("Total gelé : **%d fichiers, %d lignes de code**." % (len(gel), sum(loc.get(f, 0) for f in gel)))
    w("")
    w("## Par motif")
    w("")
    by = collections.defaultdict(list)
    for f in gel:
        j = per[f][1]
        j = re.sub(r" \[prime sur.*\]$", "", j)
        j = re.sub(r"^(déclaré|inclus) par [^ ]+ \((.*)\)$", r"projets : \2", j)
        by[j].append(f)
    w("| Motif | Fichiers | Lignes | Répertoires principaux |")
    w("|---|---|---|---|")
    for j, fs in sorted(by.items(), key=lambda kv: -sum(loc.get(f, 0) for f in kv[1])):
        dirs = collections.Counter(dir_key(f) for f in fs)
        w("| %s | %d | %d | %s |" % (j, len(fs), sum(loc.get(f, 0) for f in fs),
                                     ", ".join("`%s` (%d)" % (disp(d), n) for d, n in dirs.most_common(6))
                                     + (" …" if len(dirs) > 6 else "")))
    w("")
    w("## Par répertoire")
    w("")
    agg = collections.defaultdict(lambda: [0, 0])
    for f in gel:
        agg[dir_key(f)][0] += 1
        agg[dir_key(f)][1] += loc.get(f, 0)
    w("| Répertoire | Fichiers | Lignes |")
    w("|---|---|---|")
    for d in sorted(agg):
        w("| `%s` | %d | %d |" % (disp(d), agg[d][0], agg[d][1]))
    w("")
    w("Décision utilisateur du 2026-09-30 : proposition acceptée, sauf Cortex-M7 Atmel SAMV71/SAME70 "
      "(point 3), maintenu en différé comme référence M7. Les points ci-dessous sont ceux de la proposition.")
    w("")
    w("## Points de la proposition")
    w("")
    w("1. **Cibles abandonnées actées** (décision 2026-09-29 : ARM7, ARM9, M16C) et **simulations** "
      "(`dev/arch/gnu32`, `dev/arch/win32`, `core/arch/win32`, `tools/virtual_cpu`, `prj/vc-2010`) : "
      "gel proposé sans réserve. Réserve : `dev/arch/gnu32` sert au noyau statique hôte de mklepton "
      "(`prj/scons/arch/synthetic/x86_static`) — gel à confirmer après la tâche 5.")
    w("2. **embOS IAR Cortex-M** (`core/ucore/embOSCXM*`) : proposé gelé car remplacé par le port GCC Segger "
      "(`third_party/embos`) ; conservé en lecture pour la comparaison d'API (tâche 4).")
    w("3. **Cortex-M7 Atmel** (SAMV71/SAME70 : `dev/arch/cortexm/at91samv7x`, `dev/bsp/same70xplained`, "
      "`dev/bsp/samv71xplained_ultra`, `dev/arch/at91/softpack-lib`) : cartes non retenues ; seul code M7 "
      "de l'arbre (référence possible pour la Discovery F7, qui n'a aucun projet).")
    w("4. **Freescale K60** (`dev/arch/cortexm/k60n512`, `user/tauon_sampleapp/hal/board_freescale_twrk60n512`) : "
      "carte abandonnée, sans projet IAR (build scons seulement).")
    w("5. **eCos** (`tools/host/debian/ecos`) et **WinPcap** (`tools/host/win32`) : outillage hôte sans "
      "usage dans la cible Linux/GCC.")
    w("6. **Non gelés, laissés en différé faute de décision** : STM32WL55 Nucleo (`stm32wlxx`, projets "
      "EWARM 9.50 les plus récents de l'arbre), variante `discovery_f4-baseboard-modem`, NFC PN7150, USB "
      "device STM32F4. `prj/scons` n'est pas gelé (build du noyau statique mklepton).")
    w("7. **Candidats étape 6 (différé)** : M3 = STM32F1 (`dev/arch/cortexm/stm32f1xx`, recommandé : même "
      "famille ST que la base) ou Stellaris LM3S (`stellaris`, produit TI ancien) ; M0+ = SAMD20 "
      "(`samd20xplained_pro`, seul BSP M0+ de l'arbre). HYPOTHÈSE À VALIDER : proposition, cartes non choisies.")
    w("")
    with open(os.path.join(doc, "code-gele.md"), "w", encoding="utf-8") as fh:
        fh.write("\n".join(L) + "\n")


if __name__ == "__main__":
    main()
