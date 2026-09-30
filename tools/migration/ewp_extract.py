#!/usr/bin/env python3
"""ewp_extract.py — extraction des projets IAR (.ewp/.eww/.ewd) de l'arbre Lepton (étape 1, tâche 1).

Lecture seule du trunk ($LEPTON_TRUNK, défaut ~/lepton/trunk), liens symboliques suivis.
Pour chaque .ewp (fileVersion 1 à 4) et chaque configuration : sources C/C++/asm effectives
(fichiers exclus par configuration retirés, exclusion de groupe propagée aux fichiers du groupe),
includes, defines, options cpu/fpu, .icf/.xcl de liaison, bibliothèques liées, étapes custom
(pre/post-build, CUSTOM), surcharges d'options par fichier.

Normalisation des chemins :
  $PROJ_DIR$ -> répertoire du .ewp ; $WS_DIR$ -> répertoire du .eww ;
  c:\\tauon, C:/tauon (casse indifférente) -> racine du trunk (tauon_make_link.bat : « mklink /j c:\\tauon . ») ;
  $TOOLKIT_DIR$, $EW_DIR$, autre lecteur (x:) -> externe (installation IAR / poste Windows), non résolu ;
  « \\ » -> « / », « .. » réduits ; résolution insensible à la casse si le chemin exact n'existe pas.

Cœur : 1) --cpu du fichier settings/<projet>.<config>.driver.xcl généré par EWARM (s'il existe) ;
2) sinon table CoreVariant/Variant -> cœur construite À PARTIR des couples observés dans les .xcl
(provenance « xcl-corrélé ») ; 3) sinon nom de configuration (cortex-m4, arm926ejs…) ; 4) sinon
famille de la puce (OGChipSelectEditMenu) si OGCoreOrChip = 1. La provenance est toujours consignée.

Sorties (défaut $LEPTON_BUILD/etape-1/projets = ~/lepton/build/etape-1/projets) :
  <chemin_du_projet avec « / » -> « __ »>.json  un fichier par .ewp
  projets.json                                  index : projets, workspaces .eww, .ewd, tables cœur/FPU
Usage (depuis la racine du clone) :
  python3 tools/migration/ewp_extract.py [--out DIR] [--markdown doc/migration/inventaire-projets.md]
"""
import argparse
import collections
import datetime
import json
import os
import re
import sys
import xml.etree.ElementTree as ET

TRUNK = os.path.realpath(os.path.expanduser(os.environ.get("LEPTON_TRUNK", "~/lepton/trunk")))
BUILD = os.path.expanduser(os.environ.get("LEPTON_BUILD", "~/lepton/build"))

SRC_KIND = {".c": "c", ".cpp": "cpp", ".cc": "cpp", ".s": "asm", ".asm": "asm", ".s79": "asm",
            ".h": "h", ".hpp": "h", ".inc": "h", ".a": "lib", ".r79": "lib", ".lib": "lib"}

# Famille de puce -> cœur (données constructeur, utilisées seulement si OGCoreOrChip = 1).
CHIP_CORE = [(r"^STM32F4", "Cortex-M4"), (r"^STM32F2", "Cortex-M3"), (r"^STM32F1", "Cortex-M3"),
             (r"^STM32WL55JC_M4", "Cortex-M4"), (r"^LM3S", "Cortex-M3"), (r"^AT91SAM9", "ARM926EJ-S"),
             (r"^AT91SAM7", "ARM7TDMI"), (r"^AT91M", "ARM7TDMI"), (r"^ATSAMV7|^ATSAME7", "Cortex-M7"),
             (r"^ATSAMD2", "Cortex-M0+")]
CFG_CORE = [(r"cortex-m0\+", "Cortex-M0+"), (r"cortex-m4m7", "Cortex-M7"), (r"cortex-m4f", "Cortex-M4"),
            (r"cortex-m4", "Cortex-M4"), (r"cortex-m3", "Cortex-M3"), (r"cortex-m7", "Cortex-M7"),
            (r"arm926ejs", "ARM926EJ-S"), (r"samv71", "Cortex-M7")]

# Étiquetage des projets (famille / carte) d'après le chemin : sert au tableau, pas au périmètre.
BOARD_TAGS = [
    ("stm32f4-usb-core", "STM32F4 (USB device)"), ("discovery_f4-baseboard-modem", "STM32F4 Discovery + modem"),
    ("stm32f469i", "STM32F469I-EVAL"), ("olimex_p407", "Olimex STM32-P407"), ("stm32f407-discovery",
    "STM32F4-Discovery"), ("discovery_f4", "STM32F4-Discovery"), ("stm32f4xx", "STM32F4 (famille)"),
    ("stm32f1xx", "STM32F1 (famille)"), ("stm32wl", "STM32WL55 Nucleo"), ("samd20", "SAMD20 Xplained Pro"),
    ("same70", "SAME70 Xplained"), ("samv7", "SAMV71 Xplained Ultra"), ("lm3s", "Stellaris LM3S"),
    ("stellaris", "Stellaris LM3S"), ("cmsis", "générique Cortex-M (LM3S)"), ("at91sam9261", "AT91SAM9261 (ARM9)"),
    ("at91sam7x", "AT91SAM7X (ARM7)"), ("at91m55800", "AT91M55800 (ARM7)"), ("nfc", "NXP PN7150 (NFC)"),
    ("tauon/tauon", "noyau Lepton (multi-cœur)")]
STM32F4_KEYS = ("stm32f4", "discovery_f4", "olimex_p407", "stm32f469i", "stm32f407")


def walk(root):
    for d, dirs, files in os.walk(root, followlinks=True):
        dirs.sort()
        for f in sorted(files):
            yield os.path.join(d, f)


_ci_cache = {}


def resolve_ci(path):
    """Résout un chemin absolu ; si absent, composant par composant sans tenir compte de la casse.
    Retourne (chemin_réel | None, casse_différente)."""
    if os.path.exists(path):
        return path, False
    cur = "/"
    for p in path.strip("/").split("/"):
        cand = os.path.join(cur, p)
        if os.path.exists(cand):
            cur = cand
            continue
        try:
            if cur not in _ci_cache:
                _ci_cache[cur] = {e.lower(): e for e in os.listdir(cur)}
            m = _ci_cache[cur].get(p.lower())
        except OSError:
            m = None
        if m is None:
            return None, False
        cur = os.path.join(cur, m)
    return cur, True


DRIVE_TAUON = re.compile(r"^[a-zA-Z]:[\\/]+tauon(?=[\\/]|$)", re.I)
DRIVE = re.compile(r"^[a-zA-Z]:([\\/]|$)")


def norm_path(raw, proj_dir, ws_dir=None):
    """Chemin IAR -> {raw, path (relatif au trunk) | None, external, exists, case}."""
    r = {"raw": raw}
    if raw is None or not raw.strip():
        r.update(path=None, external=None, exists=False, case=False)
        return r
    s = raw.strip().strip('"')
    s = s.replace("$PROJ_DIR$", proj_dir).replace("$WS_DIR$", ws_dir or proj_dir)
    ext = None
    m = re.search(r"\$([A-Z_]+)\$", s)
    if m:
        ext = m.group(0)
    elif DRIVE_TAUON.match(s):
        s = TRUNK + s[DRIVE_TAUON.match(s).end():]
    elif DRIVE.match(s):
        ext = s[:2].lower()
    if ext:
        r.update(path=None, external=ext, exists=False, case=False)
        return r
    s = os.path.normpath(os.path.join(proj_dir, s.replace("\\", "/")))
    real, case = resolve_ci(s)
    rel = os.path.relpath(real or s, TRUNK)
    if rel.startswith(".."):
        r.update(path=None, external="hors-trunk", exists=bool(real), case=case)
    else:
        r.update(path=rel, external=None, exists=bool(real), case=case)
    return r


def options(settings_el):
    out = {}
    data = settings_el.find("data")
    if data is None:
        return out
    for o in data.findall("option"):
        out[o.findtext("name")] = [(st.text or "").strip() for st in o.findall("state")]
    return out


def nonempty(lst):
    return [x for x in lst if x]


def custom_steps(settings_el):
    """Contenu non vide de CUSTOM / BUILDACTION, toutes générations (y compris <buildActions> v4)."""
    steps = {}
    for e in settings_el.iter():
        if e.tag in ("prebuild", "postbuild", "cmdline", "extensions", "outputs", "inputs") \
                and (e.text or "").strip():
            steps[e.tag] = e.text.strip()
    for ba in settings_el.iter("buildAction"):
        steps.setdefault("buildActions", []).append({k.tag: (k.text or "").strip() for k in ba})
    return steps


def driver_xcl(proj_dir, proj_base, cfg):
    """--cpu/--fpu du settings/<préfixe>.<cfg>.driver.xcl ; préfixe = plus long préfixe commun avec le .ewp."""
    sdir = os.path.join(proj_dir, "settings")
    if not os.path.isdir(sdir):
        return None
    suffix = ("." + cfg + ".driver.xcl").lower()
    best = None
    for f in os.listdir(sdir):
        if f.lower().endswith(suffix):
            pre = f[: -len(suffix)].lower()
            score = len(os.path.commonprefix([pre, proj_base.lower()]))
            if score and (best is None or score > best[0]):
                best = (score, f)
    if best is None:
        return None
    txt = open(os.path.join(sdir, best[1]), encoding="latin-1").read()
    res = {"file": os.path.relpath(os.path.join(sdir, best[1]), TRUNK)}
    for k in ("cpu", "fpu"):
        m = re.search(r'"--%s=([^"]+)"' % k, txt)
        if m:
            res[k] = m.group(1)
    return res


def parse_group(el, proj_dir, group_path, inherited_excl, files):
    for child in el:
        if child.tag not in ("group", "file"):
            continue
        excl = set(inherited_excl)
        ex = child.find("excluded")
        if ex is not None:
            excl |= {c.text for c in ex.findall("configuration")}
        if child.tag == "group":
            parse_group(child, proj_dir, group_path + [child.findtext("name")], excl, files)
            continue
        raw = child.findtext("name")
        n = norm_path(raw, proj_dir)
        ext = os.path.splitext((n["path"] or raw or "").replace("\\", "/"))[1]
        kind = "asm" if ext == ".S" else SRC_KIND.get(ext.lower(), "autre")
        over = {}
        for c in child.findall("configuration"):
            for s in c.findall("settings"):
                keep = {k: nonempty(v) for k, v in options(s).items()
                        if k in ("CCDefines", "CCIncludePath2", "ADefines", "AUserIncludes", "PreInclude",
                                 "IExtraOptions")}
                if keep:
                    over.setdefault(c.findtext("name"), {})[s.findtext("name")] = keep
        f = dict(n, kind=kind, group="/".join(g or "" for g in group_path), excluded=sorted(excl))
        if over:
            f["overrides"] = over
        files.append(f)


def board_tag(rel):
    low = rel.lower()
    for k, v in BOARD_TAGS:
        if k in low:
            return v
    return "?"


def parse_ewp(path, ws_map):
    proj_dir = os.path.dirname(path)
    rel = os.path.relpath(path, TRUNK)
    root = ET.parse(path).getroot()
    fv = root.findtext("fileVersion")
    proj = {"project": rel, "fileVersion": int(fv) if fv and fv.isdigit() else fv,
            "workspaces": sorted(ws_map.get(rel, [])), "board": board_tag(rel),
            "stm32f4": any(k in rel.lower() for k in STM32F4_KEYS),
            "configurations": [], "files": []}
    parse_group(root, proj_dir, [], set(), proj["files"])
    base = os.path.splitext(os.path.basename(path))[0]
    proj["mentions_mklepton"] = bool(re.search("mklepton", open(path, errors="replace").read(), re.I))
    for c in root.findall("configuration"):
        cname = c.findtext("name")
        st = {s.findtext("name"): s for s in c.findall("settings")}
        opt = {k: options(v) for k, v in st.items()}
        g, cc, aa = opt.get("General", {}), opt.get("ICCARM", {}), opt.get("AARM", {})
        il, xl = opt.get("ILINK"), opt.get("XLINK")
        ar = opt.get("IARCHIVE") or opt.get("XAR") or {}

        def first(d, k):
            return (d.get(k) or [None])[0] if d else None
        out_bin = first(g, "GOutputBinary")
        output = {"0": "executable", "1": "bibliotheque"}.get(out_bin, out_bin)
        cfg = {
            "name": cname, "toolchain": c.findtext("toolchain/name"), "debug": c.findtext("debug"),
            "output": output,
            "ew_version": first(g, "OGLastSavedByProductVersion") or first(g, "OGProductVersion"),
            "cpu": {"chip": (first(g, "OGChipSelectEditMenu") or "").replace("\t", " | ") or None,
                    "OGCoreOrChip": first(g, "OGCoreOrChip"), "CoreVariant": first(g, "CoreVariant"),
                    "Variant": first(g, "Variant"), "GProcessorMode": first(g, "GProcessorMode"),
                    "FPU": first(g, "FPU"), "FPU2": first(g, "FPU2"), "NrRegs": first(g, "NrRegs"),
                    "driver_xcl": driver_xcl(proj_dir, base, cname)},
            "defines": nonempty(cc.get("CCDefines", [])),
            "asm_defines": nonempty(aa.get("ADefines", [])),
            "includes": [norm_path(x, proj_dir) for x in nonempty(cc.get("CCIncludePath2", []))],
            "asm_includes": [norm_path(x, proj_dir) for x in nonempty(aa.get("AUserIncludes", []))],
            "preinclude": nonempty(cc.get("PreInclude", [])),
            "extra_options": nonempty(cc.get("IExtraOptions", [])) if first(cc, "IExtraOptionsCheck") == "1" else [],
            "archive_output": first(ar, "IarchiveOutput") or first(ar, "XAROutput"),
            "link": None, "custom_steps": {},
        }
        if il:
            ov = first(il, "IlinkIcfOverride")
            icf = first(il, "IlinkIcfFile")
            cfg["link"] = {"linker": "ILINK", "override": ov,
                           "icf": norm_path(icf, proj_dir) if (ov == "1" and icf) else None,
                           "icf_default": None if ov == "1" else icf,
                           "libs_option": nonempty(il.get("IlinkAdditionalLibs", [])),
                           "entry": first(il, "IlinkProgramEntryLabel")}
        elif xl:
            ov = first(xl, "XclOverride")
            xcl = first(xl, "XclFile")
            cfg["link"] = {"linker": "XLINK", "override": ov,
                           "icf": norm_path(xcl, proj_dir) if (ov == "1" and xcl) else None,
                           "icf_default": None if ov == "1" else xcl, "libs_option": [], "entry": None}
        for k in ("CUSTOM", "BUILDACTION"):
            if k in st:
                s = custom_steps(st[k])
                if s:
                    cfg["custom_steps"][k] = s
        live = [f for f in proj["files"] if cname not in f["excluded"]]
        srcs = [f for f in live if f["kind"] in ("c", "cpp", "asm")]
        cfg["sources"] = sorted({f["path"] or f["raw"] for f in srcs})
        cfg["headers"] = sorted({f["path"] or f["raw"] for f in live if f["kind"] == "h"})
        cfg["libs"] = sorted({f["path"] or f["raw"] for f in live if f["kind"] == "lib"})
        cfg["n_sources"] = {k: sum(1 for f in srcs if f["kind"] == k) for k in ("c", "cpp", "asm")}
        cfg["n_excluded"] = sum(1 for f in proj["files"] if cname in f["excluded"])
        cfg["missing_sources"] = sorted({f["path"] or f["raw"] for f in srcs if not f["exists"]})
        proj["configurations"].append(cfg)
    proj["missing_files"] = sorted({f["path"] or f["raw"] for f in proj["files"]
                                    if not f["exists"] and f["kind"] != "lib"})
    proj["external_files"] = sorted({f["raw"] for f in proj["files"] if f["external"]})
    proj["case_mismatch"] = sorted({f["path"] for f in proj["files"] if f["case"]})
    proj["per_file_overrides"] = sorted({f["path"] or f["raw"] for f in proj["files"] if f.get("overrides")})
    return proj


def parse_eww(path):
    ws_dir = os.path.dirname(path)
    root = ET.parse(path).getroot()
    projs = [norm_path(p.findtext("path"), ws_dir, ws_dir) for p in root.iter("project")]
    return {"workspace": os.path.relpath(path, TRUNK), "projects": projs,
            "batch_builds": [b.findtext("name") for b in root.iter("batchDefinition")]}


def name_core(cname):
    for rx, v in CFG_CORE:
        if re.search(rx, cname, re.I):
            return v
    return None


def deduce_cores(projects):
    """Tables CoreVariant/Variant -> cœur et FPU2 -> fpu, apprises des .driver.xcl ; puis cœur par config."""
    cv_tab, fpu_tab = collections.defaultdict(collections.Counter), collections.defaultdict(collections.Counter)
    for p in projects:
        for c in p["configurations"]:
            x, cpu = c["cpu"]["driver_xcl"], c["cpu"]
            if x and x.get("cpu"):
                key = ("CoreVariant", cpu["CoreVariant"]) if cpu["CoreVariant"] is not None else ("Variant", cpu["Variant"])
                cv_tab[key][x["cpu"]] += 1
                if x.get("fpu") and cpu["FPU2"] is not None:
                    fpu_tab[cpu["FPU2"]][x["fpu"]] += 1
    # Seconde table : code -> cœur appris des configurations dont le NOM désigne le cœur.
    name_tab = collections.defaultdict(collections.Counter)
    for p in projects:
        for c in p["configurations"]:
            cpu = c["cpu"]
            key = ("CoreVariant", cpu["CoreVariant"]) if cpu["CoreVariant"] is not None else ("Variant", cpu["Variant"])
            if name_core(c["name"]) and key[1] is not None:
                name_tab[key][name_core(c["name"])] += 1
    for p in projects:
        for c in p["configurations"]:
            cpu = c["cpu"]
            x = cpu["driver_xcl"] or {}
            key = ("CoreVariant", cpu["CoreVariant"]) if cpu["CoreVariant"] is not None else ("Variant", cpu["Variant"])
            core, src = None, None
            if x.get("cpu"):
                core, src = x["cpu"], "xcl"
            elif cpu["OGCoreOrChip"] == "1" and cpu["chip"]:
                for rx, v in CHIP_CORE:
                    if re.search(rx, cpu["chip"]):
                        core, src = v, "puce"
                        break
            if core is None and key in cv_tab and len(cv_tab[key]) == 1:
                core, src = next(iter(cv_tab[key])), "xcl-corrélé"
            if core is None:
                core = name_core(c["name"])
                src = "nom-config" if core else None
            if core is None and key in name_tab and len(name_tab[key]) == 1:
                core, src = next(iter(name_tab[key])), "nom-config-corrélé"
            if core is None and cpu["chip"]:
                for rx, v in CHIP_CORE:
                    if re.search(rx, cpu["chip"]):
                        core, src = v, "puce (OGCoreOrChip≠1)"
                        break
            fpu, fsrc = None, None
            if x.get("fpu"):
                fpu, fsrc = x["fpu"], "xcl"
            elif cpu["FPU2"] in fpu_tab and len(fpu_tab[cpu["FPU2"]]) == 1:
                fpu, fsrc = next(iter(fpu_tab[cpu["FPU2"]])), "xcl-corrélé"
            elif cpu["FPU2"] == "0" or cpu["FPU"] == "0":
                fpu, fsrc = "None", "FPU=0"
            if fpu is None:
                fpu = "FPU2=%s" % cpu["FPU2"] if cpu["FPU2"] is not None else "FPU=%s" % cpu["FPU"]
            c["core"] = {"core": core or "?", "source": src or "indéterminé",
                         "fpu": fpu, "fpu_source": fsrc or "code IAR brut"}
    return ({"%s=%s" % k: dict(v) for k, v in cv_tab.items()}, {k: dict(v) for k, v in fpu_tab.items()},
            {"%s=%s" % k: dict(v) for k, v in name_tab.items()})


# ----------------------------------------------------------------------------- Markdown
def md_escape(s):
    return str(s).replace("|", "\\|")


def short_inc(i):
    if i["path"]:
        return i["path"].replace("sys/root/src/", "src/") + ("" if i["exists"] else " (absent)")
    return "%s [externe %s]" % (i["raw"], i["external"])


def role(p):
    low = p["project"].lower()
    if "usb-core" in low:
        return "STM32F4, USB device (optionnel)"
    if "baseboard-modem" in low:
        return "variante STM32F4 non retenue"
    if p["stm32f4"]:
        return "base F439 (STM32F4)"
    if "tauon/tauon" in low:
        return "noyau (config. cortex-m4 → F439)"
    if any(k in low for k in ("samd20", "stm32f1xx", "lm3s", "stellaris", "tauon-basic_cmsis")):
        return "candidat étape 6 (M0+/M3)"
    if any(k in low for k in ("at91sam9", "at91sam7", "at91m55800")):
        return "gelé (ARM7/ARM9)"
    if any(k in low for k in ("samv7", "same70")):
        return "M7 Atmel (carte non retenue)"
    if "stm32wl" in low:
        return "STM32WL (carte non listée)"
    if "nfc" in low:
        return "périphérique optionnel"
    return "?"


def write_markdown(projects, index, dest, cv_tab, fpu_tab, name_tab):
    L = []
    w = L.append
    fv = collections.Counter(p["fileVersion"] for p in projects)
    w("# Inventaire des projets IAR (étape 1, tâche 1)")
    w("")
    w("Généré par `tools/migration/ewp_extract.py` le %s (trunk `%s`). Ne pas éditer à la main : "
      "rejouer `python3 tools/migration/ewp_extract.py --markdown doc/migration/inventaire-projets.md` "
      "depuis la racine du clone. JSON détaillé par projet : `$LEPTON_BUILD/etape-1/projets/`."
      % (datetime.date.today().isoformat(), os.path.basename(TRUNK)))
    w("")
    w("## Synthèse")
    w("")
    w("- `.ewp` : **%d** (fileVersion : %s) ; `.eww` : **%d** ; `.ewd` : **%d** (aucun fichier de configuration "
      "du débogueur dans l'arbre)." % (len(projects), ", ".join("v%s = %d" % (k, v) for k, v in sorted(fv.items(), key=str)),
                                        index["n_eww"], index["n_ewd"]))
    nsteps = sum(1 for p in projects for c in p["configurations"] if c["custom_steps"])
    nmk = sum(1 for p in projects if p["mentions_mklepton"])
    w("- Étapes custom (pre/post-build, CUSTOM, buildActions) : **%d** configuration(s) non vide(s) ; "
      "projets citant `mklepton` : **%d**. La génération mklepton (`kernel_mkconf.h`, `dev_mkconf.c`, "
      "`bin_mkconf.c`, `dev_dskimg.[ch]` sous `src/kernel/core/arch/<arch>/`) est donc lancée hors des "
      "projets IAR ; ces fichiers sont référencés mais absents de l'arbre (voir « Sources absentes »)." % (nsteps, nmk))
    w("- Tous les chemins absolus des projets sont en `c:\\tauon\\…` (ou `C:/tauon/…`) : "
      "`tauon_make_link.bat` crée la jonction `c:\\tauon` → racine de l'arbre ; ils sont ramenés au trunk.")
    w("- Cœur par configuration : provenance indiquée (`xcl` = option `--cpu` du `settings/*.driver.xcl` "
      "généré par EWARM ; `xcl-corrélé` = même code CoreVariant/Variant qu'une config. dont le `.xcl` existe ; "
      "`nom-config` = déduit du nom de configuration ; `puce` = famille de la puce sélectionnée).")
    w("")
    w("Table apprise des `.driver.xcl` (code IAR → cœur / FPU), seule base des provenances `xcl-corrélé` :")
    w("")
    w("| Code IAR | Cœur(s) observé(s) (nb de .xcl) |")
    w("|---|---|")
    for k in sorted(cv_tab):
        w("| %s | %s |" % (k, ", ".join("%s (%d)" % kv for kv in cv_tab[k].items())))
    for k in sorted(fpu_tab):
        w("| FPU2=%s | %s |" % (k, ", ".join("%s (%d)" % kv for kv in fpu_tab[k].items())))
    w("")
    w("Table apprise des noms de configuration (provenance `nom-config-corrélé`) :")
    w("")
    w("| Code IAR | Cœur(s) d'après le nom de configuration (nb de config.) |")
    w("|---|---|")
    for k in sorted(name_tab):
        w("| %s | %s |" % (k, ", ".join("%s (%d)" % kv for kv in name_tab[k].items())))
    w("")
    w("HYPOTHÈSE À VALIDER : les codes CoreVariant/Variant non couverts par un `.xcl` (ex. `Variant` des "
      "projets EWARM 4.x–6.x, `FPU2=4`, `FPU2=7`) sont interprétés par le nom de configuration ou la puce ; "
      "aucune table officielle IAR n'est disponible dans l'arbre.")
    w("")
    w("### Tableau synthétique")
    w("")
    w("Colonnes : fV = fileVersion ; EW = version EWARM du dernier enregistrement (max. des configurations) ; "
      "src = nombre max. de sources C/asm d'une configuration ; abs. = fichiers référencés absents "
      "(hors bibliothèques de sortie).")
    w("")
    w("| Projet (sous `sys/`) | fV | EW | Carte / famille | Configurations : cœur, sortie | src | abs. | Rôle proposé |")
    w("|---|---|---|---|---|---|---|---|")
    for p in projects:
        ew = max((c["ew_version"] or "" for c in p["configurations"]), default="")
        cfgs = "<br>".join("%s : %s%s, %s" % (md_escape(c["name"]), c["core"]["core"],
                                              "" if c["core"]["fpu"] in ("None", "?") else " " + c["core"]["fpu"],
                                              "lib" if c["output"] == "bibliotheque" else "exe")
                           for c in p["configurations"])
        nsrc = max((sum(c["n_sources"].values()) for c in p["configurations"]), default=0)
        name = p["project"][4:] if p["project"].startswith("sys/") else p["project"]
        flag = " **[F4]**" if p["stm32f4"] else ""
        w("| `%s`%s | %s | %s | %s | %s | %d | %d | %s |" % (md_escape(name), flag, p["fileVersion"], ew,
                                                             p["board"], cfgs, nsrc, len(p["missing_files"]), role(p)))
    w("")
    w("## Signalements")
    w("")
    f4 = [p for p in projects if p["stm32f4"]]
    w("### Projets STM32F4 (base de la NUCLEO-F439ZI)")
    w("")
    for p in f4:
        w("- `%s` — %s" % (p["project"], p["board"]))
    w("")
    w("Ces projets s'appuient sur le noyau `prj/iar/arch/arm/tauon/tauon_*.ewp`, configuration "
      "`tauon-kernel-cortex-m4-debug` (bibliothèque `building/output/tauon_7.80/tauon-kernel-cortex-m4-debug/Lib/tauon_7.80.a` "
      "liée par les applications `tauon-basic_stm32f4*`/`stm32f469i`), et sur `dev_stm32f4xx_7.20` (config. `Debug`).")
    w("")
    w("### Cartes de l'étape 6")
    w("")
    w("- **Olimex STM32-P407** (M4F) : `prj/iar/bsp/olimex_p407/bsp_olimex_p407_7.30.ewp` et application "
      "`user/tauon-basic/prj/iar/arch/arm/tauon-basic_stm32f4-olimex_p407/…_7.20.ewp` ; même famille que la F439.")
    w("- **Discovery F7** : **aucun projet STM32F7 dans l'arbre** (ni `.ewp`, ni répertoire `stm32f7*`). Le seul "
      "code Cortex-M7 existant est Atmel SAMV71/SAME70 (`dev/arch/cortexm/at91samv7x`, `bsp/samv71xplained_ultra`, "
      "`bsp/same70xplained`) et les configurations `tauon-kernel-cortex-m7-debug` / `…-m4m7-freertosv9-debug` du noyau.")
    w("- **M3** (à choisir) : `dev_stm32f1xx_6.21.ewp` (STM32F1), `dev_lm3s_6.21.ewp` + `driverlib.ewp` + "
      "`tauon-basic_lm3s_6.21.ewp` / `tauon-basic_cmsis_6.21.ewp` (Stellaris LM3S) ; noyau `tauon-kernel-cortex-m3-debug`.")
    w("- **M0/M0+** (à choisir) : `dev_at91samd20_7.20.ewp`, `bsp_samd20xplained_pro_7.30.ewp`, "
      "`tauon-basic_at91samd20_7.20.ewp` (SAMD20) ; noyau `tauon-kernel-cortex-m0+-freertos-debug` (FreeRTOS seulement).")
    w("")
    w("### Sources et fichiers référencés absents de l'arbre")
    w("")
    miss = collections.defaultdict(set)
    for p in projects:
        for m in p["missing_files"]:
            key = m if not m.startswith("sys/") else "/".join(m.split("/")[:7])
            miss[key].add(os.path.basename(p["project"]))
    w("Regroupés par répertoire (7 niveaux) ; nombre de projets citant :")
    w("")
    w("| Répertoire / fichier absent | Projets |")
    w("|---|---|")
    for k in sorted(miss):
        w("| `%s` | %d |" % (md_escape(k), len(miss[k])))
    w("")
    w("Remarques : `embOSARM7_332`, `embOSARM7_360/src`, `embOSCXM3_380/382` sont des versions d'embOS IAR "
      "absentes (autres versions présentes) ; `core/arch/{arm,cortexm}/*_mkconf.*` et `dev_dskimg.*` sont des "
      "sorties de mklepton (cf. `src/kernel/core/arch/cortexm/mklepton-output-generation.md`).")
    w("")
    ext = sorted({(e, os.path.basename(p["project"])) for p in projects for e in p["external_files"]})
    if ext:
        w("Fichiers de projet hors trunk ou sur un autre poste :")
        w("")
        for e, pn in ext:
            w("- `%s` (%s)" % (e, pn))
        w("")
    extinc = collections.Counter()
    for p in projects:
        for c in p["configurations"]:
            for i in c["includes"] + c["asm_includes"]:
                if i["external"]:
                    extinc[i["external"]] += 1
                elif not i["exists"]:
                    extinc["absent"] += 1
    w("Chemins d'include non résolus (occurrences, toutes configurations) : %s." %
      ", ".join("%s = %d" % kv for kv in sorted(extinc.items())))
    w("")
    w("### Autres anomalies")
    w("")
    w("- `prj/iar/arch/arm/stm32f4-usb-core/Backup of stm32f4_usb_core.ewp` : copie de sauvegarde EWARM "
      "(compte dans les 46 `.ewp`).")
    w("- Plusieurs générations du même projet coexistent (`tauon.ewp`, `tauon_6.10` … `tauon_9.50` ; "
      "`dev_stm32f4xx_6.21/7.20/8.40` ; `…_8.40`/`…_9.50`) ; les configurations `Release` des BSP/pilotes sont "
      "des squelettes (sans include ni define propre, cœur non renseigné).")
    w("- Puce sélectionnée incohérente avec le cœur (ex. `LM3S9D96` dans toutes les configurations du noyau, "
      "`STM32F207ZG` dans `freertos-debug` des applications STM32F4) : le cœur effectif est donné par CoreVariant "
      "lorsque OGCoreOrChip = 0.")
    ov = [(p["project"], len(p["per_file_overrides"])) for p in projects if p["per_file_overrides"]]
    if ov:
        w("- Surcharges d'options par fichier : %s." % "; ".join("`%s` (%d fichier(s))" % (os.path.basename(a), b) for a, b in ov))
    w("")
    w("## Détail par projet")
    w("")
    for p in projects:
        w("### `%s`" % p["project"])
        w("")
        w("fileVersion %s ; carte : %s ; workspaces : %s ; fichiers déclarés : %d ; absents : %d."
          % (p["fileVersion"], p["board"], ", ".join("`%s`" % os.path.basename(x) for x in p["workspaces"]) or "aucun",
             len(p["files"]), len(p["missing_files"])))
        w("")
        cfgs = p["configurations"]
        common = None
        for c in cfgs:
            s = [short_inc(i) for i in c["includes"]]
            common = set(s) if common is None else common & set(s)
        common = common or set()
        if common and len(cfgs) > 1:
            w("- Includes communs : %s" % ", ".join("`%s`" % md_escape(x) for x in sorted(common)))
        for c in cfgs:
            core = c["core"]
            link = c["link"] or {}
            if c["output"] == "bibliotheque":
                lk = "bibliothèque → `%s`" % md_escape(c["archive_output"] or "?")
            elif link.get("icf"):
                lk = "%s `%s`%s" % ("ICF" if link["linker"] == "ILINK" else "XCL", md_escape(link["icf"]["path"] or link["icf"]["raw"]),
                                    "" if link["icf"]["exists"] else " (absent)")
            else:
                lk = "%s par défaut EWARM (`%s`)" % ("ICF" if link.get("linker") == "ILINK" else "XCL", md_escape(link.get("icf_default")))
            incs = [short_inc(i) for i in c["includes"] if short_inc(i) not in common or len(cfgs) == 1]
            w("- **%s** — %s (%s), FPU %s (%s) ; puce `%s` ; EW %s ; sources c/cpp/asm %d/%d/%d, exclus %d, absents %d ; %s"
              % (md_escape(c["name"]), core["core"], core["source"], core["fpu"], core["fpu_source"],
                 md_escape(c["cpu"]["chip"] or "-"), c["ew_version"], c["n_sources"]["c"], c["n_sources"]["cpp"],
                 c["n_sources"]["asm"], c["n_excluded"], len(c["missing_sources"]), lk))
            if c["defines"]:
                w("  - defines : %s" % " ".join("`%s`" % md_escape(d) for d in c["defines"]))
            if c["asm_defines"]:
                w("  - defines asm : %s" % " ".join("`%s`" % md_escape(d) for d in c["asm_defines"]))
            if incs:
                w("  - includes : %s" % ", ".join("`%s`" % md_escape(x) for x in incs))
            if c["preinclude"] or c["extra_options"]:
                w("  - préinclusion / options : %s" % md_escape(" ".join(c["preinclude"] + c["extra_options"])))
            if c["libs"]:
                w("  - bibliothèques liées : %s" % ", ".join("`%s`" % md_escape(x) for x in c["libs"]))
            if c["custom_steps"]:
                w("  - étapes custom : `%s`" % md_escape(json.dumps(c["custom_steps"], ensure_ascii=False)))
        w("")
    os.makedirs(os.path.dirname(os.path.abspath(dest)), exist_ok=True)
    with open(dest, "w", encoding="utf-8") as fh:
        fh.write("\n".join(L) + "\n")


def main():
    ap = argparse.ArgumentParser(description="Extraction des projets IAR de l'arbre Lepton")
    ap.add_argument("--out", default=os.path.join(BUILD, "etape-1", "projets"))
    ap.add_argument("--markdown", default=None, help="écrit aussi l'inventaire Markdown")
    a = ap.parse_args()
    if not os.path.isdir(TRUNK):
        sys.exit("trunk introuvable : %s (LEPTON_TRUNK)" % TRUNK)
    os.makedirs(a.out, exist_ok=True)
    ewp, eww, ewd = [], [], []
    for p in walk(TRUNK):
        low = p.lower()
        (ewp if low.endswith(".ewp") else eww if low.endswith(".eww") else ewd if low.endswith(".ewd") else []).append(p)
    workspaces = [parse_eww(p) for p in eww]
    ws_map = {}
    for w in workspaces:
        for pr in w["projects"]:
            if pr["path"]:
                ws_map.setdefault(pr["path"], []).append(w["workspace"])
    projects = [parse_ewp(p, ws_map) for p in ewp]
    projects.sort(key=lambda p: p["project"])
    cv_tab, fpu_tab, name_tab = deduce_cores(projects)
    for pr in projects:
        with open(os.path.join(a.out, pr["project"].replace("/", "__") + ".json"), "w", encoding="utf-8") as fh:
            json.dump(pr, fh, indent=1, ensure_ascii=False)
    index = {"trunk": TRUNK, "n_ewp": len(ewp), "n_eww": len(eww), "n_ewd": len(ewd),
             "ewd": [os.path.relpath(p, TRUNK) for p in ewd], "core_table": cv_tab, "fpu_table": fpu_tab,
             "core_table_from_names": name_tab,
             "workspaces": workspaces,
             "projects": [{"project": p["project"], "json": p["project"].replace("/", "__") + ".json",
                           "fileVersion": p["fileVersion"], "board": p["board"], "stm32f4": p["stm32f4"],
                           "configurations": [{"name": c["name"], "core": c["core"], "output": c["output"]}
                                              for c in p["configurations"]]} for p in projects]}
    with open(os.path.join(a.out, "projets.json"), "w", encoding="utf-8") as fh:
        json.dump(index, fh, indent=1, ensure_ascii=False)
    fv = collections.Counter(p["fileVersion"] for p in projects)
    print("ewp=%d eww=%d ewd=%d fileVersion=%s -> %s" % (len(ewp), len(eww), len(ewd),
                                                        dict(sorted(fv.items(), key=str)), a.out))
    if a.markdown:
        write_markdown(projects, index, a.markdown, cv_tab, fpu_tab, name_tab)
        print("markdown -> %s" % a.markdown)


if __name__ == "__main__":
    main()
