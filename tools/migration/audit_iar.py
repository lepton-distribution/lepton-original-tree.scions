#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""audit_iar.py — audit des IAR-ismes de l'arbre Lepton (étape 1, tâche 3).

Lecture seule : parcourt le trunk scion (liens symboliques suivis) et produit
  doc/migration/audit-iar.csv  (détail fichier:ligne)
  doc/migration/audit-iar.md   (synthèse)
relativement à la racine du clone (répertoire parent de tools/).

Usage (depuis la racine du clone) :
  python3 tools/migration/audit_iar.py                 # audit complet + fichiers
  python3 tools/migration/audit_iar.py --summary       # totaux seulement, rien n'est écrit
  python3 tools/migration/audit_iar.py --perimetre heuristique
Options :
  --trunk DIR        racine du trunk (défaut : $LEPTON_TRUNK, sinon ~/lepton/trunk)
  --perimetre MODE   auto (défaut) : doc/migration/perimetre.csv s'il existe, repli
                     heuristique pour les fichiers qu'il ne couvre pas ;
                     csv : perimetre.csv seul (fichiers non couverts -> non_classe) ;
                     heuristique : règles par chemin (voir REGLES_HEURISTIQUES).
  --perimetre-file F chemin de perimetre.csv (colonnes fichier, ensemble, justification ;
                     chemins relatifs au trunk ; une entrée répertoire couvre son sous-arbre).
  --out-dir DIR      répertoire de sortie (défaut : doc/migration du clone).

Sévérités (colonne `severite`) :
  iar         IAR-isme à traiter : seul compté dans la métrique « total périmètre actif » ;
  autre       autre compilateur (Keil/ARMCC, MSVC…) : à retirer, compté à part ;
  a_verifier  construction portable ou tolérée par GCC (CMSIS, #pragma pack/weak,
              garde __GNUC__, définition de compatibilité) : à revoir, non comptée ;
  info        fichier de projet IAR (.ewp, .icf…) ou asm GNU : inventaire, non compté.
Python 3, bibliothèque standard uniquement.
"""

import argparse
import collections
import csv
import datetime
import os
import re
import sys
import unicodedata

CLONE_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
DEFAULT_OUT = os.path.join(CLONE_ROOT, "doc", "migration")
DEFAULT_PERIMETRE = os.path.join(DEFAULT_OUT, "perimetre.csv")

ENSEMBLES = ("actif", "differe", "gele", "hors_projet", "non_classe")
SEVERITES = ("iar", "autre", "a_verifier", "info")

EXT_C = {".c", ".h", ".cpp", ".hpp", ".hxx", ".inl", ".cc", ".cxx"}
EXT_ASM = {".s", ".s79", ".asm", ".inc", ".s43", ".msa"}  # .S : ext() conserve la casse
EXT_PROJET_IAR = {".ewp", ".eww", ".ewd", ".ewt", ".dep", ".icf", ".xcl", ".mac",
                  ".dni", ".wsdt", ".cspy", ".board", ".flash"}
DIRS_EXCLUS = {".git", "building"}  # building/ : emplacement de génération scion (vide)

# ---------------------------------------------------------------- motifs
KW_IAR = ["__no_init", "__root", "__ramfunc", "__weak", "__packed", "__irq", "__fiq",
          "__swi", "__intrinsic", "__noreturn", "__task", "__arm", "__thumb", "__nested",
          "__interwork", "__monitor", "__stackless", "__big_endian", "__little_endian",
          # M16C / anciens ICC
          "__interrupt", "__far", "__near", "__huge", "__tiny", "__data16", "__data20",
          "__sfr", "__regbank_interrupt", "__fast_interrupt"]
RE_KW = re.compile(r"\b(" + "|".join(KW_IAR) + r")\b")
# mots-clés définis de façon portable par CMSIS / HAL ST (compat GCC)
KW_MACRO_CMSIS = {"__weak", "__packed", "__noreturn"}
RE_CHEMIN_CMSIS = re.compile(r"(cubemx_hal_driver|/cmsis(-5)?/|/CMSIS/|usb-core|STM32_USB)",
                             re.I)

PRAGMA_IAR = {"location", "section", "optimize", "data_alignment", "vector",
              "type_attribute", "object_attribute", "diag_suppress", "diag_default",
              "diag_error", "diag_warning", "diag_remark", "language", "required",
              "segment", "inline", "rtmodel", "module_name", "public_equ", "bitfields",
              "calls", "call_graph_root", "default_function_attributes",
              "default_variable_attributes", "stack_protect", "no_stack_protect", "unroll",
              "swi_number", "basic_template_matching", "include_alias", "constseg",
              "dataseg", "memory", "system_include", "printf_args", "scanf_args",
              "cstat_disable", "cstat_enable", "cstat_restore", "cstat_suppress",
              "no_epilogue", "instantiate", "no_pch", "keep_definition", "dynamic"}
PRAGMA_GCC_OK = {"pack", "weak", "once", "message", "push_macro", "pop_macro", "gcc",
                 "redefine_extname", "gcc_diagnostic"}
RE_PRAGMA = re.compile(r"^\s*#\s*pragma\s+([A-Za-z_]\w*)(.*)$")
RE_UPRAGMA = re.compile(r"\b_Pragma\s*\(")

INTR_IAR = ["__enable_interrupt", "__disable_interrupt", "__get_interrupt_state",
            "__set_interrupt_state", "__no_operation", "__enable_fiq", "__disable_fiq",
            "__enable_irq_iar", "__get_CPSR", "__set_CPSR", "__MCR", "__MRC", "__MCR2",
            "__MRC2", "__get_LR", "__set_LR", "__get_PC", "__get_SP", "__set_SP",
            "__segment_begin", "__segment_end", "__segment_size", "__section_begin",
            "__section_end", "__section_size", "__sfb", "__sfe", "__sfs",
            "__iar_builtin_\\w+", "__get_SB", "__set_SB", "__software_interrupt",
            "__disable_irq_iar", "__LDREX_IAR"]
RE_INTR_IAR = re.compile(r"\b(" + "|".join(INTR_IAR) + r")\s*\(")
INTR_CMSIS = ["__DSB", "__ISB", "__DMB", "__WFI", "__WFE", "__SEV", "__NOP", "__CLZ",
              "__REV", "__REV16", "__REVSH", "__RBIT", "__LDREXB", "__LDREXH", "__LDREXW",
              "__STREXB", "__STREXH", "__STREXW", "__CLREX", "__BKPT", "__SSAT", "__USAT",
              "__enable_irq", "__disable_irq", "__enable_fault_irq", "__disable_fault_irq"]
RE_INTR_CMSIS = re.compile(r"\b(" + "|".join(INTR_CMSIS) + r"|__get_[A-Za-z]\w*|__set_[A-Za-z]\w*)\s*\(")
SYM_IAR = re.compile(r"\b(__vector_table|__low_level_init|__iar_\w+|__cmain|__ICFEDIT_\w+|"
                     r"_DLIB_\w+|__DLIB_\w+|__ICCARM_INTRINSICS_\w*)\b")
SYM_DLIB_IO = re.compile(r"\b(__write|__read|__lseek|__close|__open|__remove|__rename|"
                         r"__getzone|__time32|__exit)\s*\(")

HDR_IAR = re.compile(r"""^\s*\#\s*include\s*[<"]\s*(
      (?:[\w./]*/)?(?:intrinsics|yfuns|yvals|ysizet|ycheck|ystdio|LowLevelIOInterface|
        DLib_\w+|xlocale|xencoding_limits|iar_dlmalloc|cmsis_iar|iccarm_builtin|
        arm_itm|inarm|in430|intr7000|DLib_Product\w*|cmsis_iar_\w*)\.h
    | (?:ST|NXP|Atmel|Freescale|TexasInstruments|Renesas)/io\w+\.h
    | io(?:at91|stm32|lpc|m16c|m32c|sam|lm3s|k\d0|mk\d|str\d|efm32|nrf|mb9|tms|xmc)\w*\.h
    )\s*[>"]""", re.X | re.I)
RE_INCLUDE = re.compile(r"^\s*#\s*include\b")

GUARD_IAR = re.compile(r"\b(__ICCARM__|__IAR_SYSTEMS_ICC__|__IAR_SYSTEMS_ASM__|__ICCAVR__|"
                       r"__ICC430__|__ICCM16C__|__IAR_SYSTEMS_ICC|__compiler_iar_arm__|"
                       r"__compiler_iar_m16c__|__IAR\w*__)\b")
GUARD_GCC = re.compile(r"\b(__GNUC__|__compiler_gnuc__|__clang__|__GNUC_MINOR__)\b")
GUARD_AUTRE = re.compile(r"\b(__CC_ARM|__ARMCC_VERSION|__compiler_keil_arm__|_MSC_VER|"
                         r"__compiler_win32__|WIN32|_WIN32|__TASKING__|__TI_ARM__|"
                         r"__TI_COMPILER_VERSION__|__CSMC__|__GHS__|__CWCC__)\b")
GUARD_ABSTR = re.compile(r"\b(__tauon_compiler__)\b")
RE_PP_COND = re.compile(r"^\s*#\s*(if|ifdef|ifndef|elif)\b")
RE_PP_DEF = re.compile(r"^\s*#\s*(define|undef)\s+(\w+)")
RE_ABSTR_LEPTON = re.compile(r"\b(__compiler_directive__packed|__kernel_compiler_\w+)\b")

# ---------------------------------------------------------------- ventilation heuristique
# Première règle qui correspond (chemin relatif au trunk, recherche insensible à la casse).
REGLES_HEURISTIQUES = [
    # --- gelé : copies d'origine des fichiers actifs dont les branches gelées ont été retirées
    (r"^legacy/", "gele", "copie d'origine (transform_iar.py, décisions D2a/D3a), supprimée à l'étape 6"),
    # --- gelé : ARM7, ARM9, M16C, simulations, eCos, outils Windows
    (r"(^|/)(arm7|arm9)(/|$)|embosarm7|at91sam9|at91sam7|at91m55800|(^|/)at91lib(/|$)|"
     r"dev_at91_(mci|rtt|usbdp)|sam9xe|atmelsam9", "gele", "ARM7/ARM9 (AT91)"),
    (r"m16c", "gele", "M16C"),
    (r"(^|/)(win32|gnu32)(/|$)|embosw32|(^|/)virtual_cpu(/|$)|(^|/)vc-2010(/|$)|"
     r"(^|/)prj/vc(/|$)|wpdpack|mklepton-w32", "gele", "simulation Windows/Linux"),
    (r"(^|/)ecos(/|$)|board_freescale_twrk60n512/lib/install", "gele",
     "eCos (hors plan) — HYPOTHÈSE À VALIDER"),
    # --- différé : autres Cortex-M, FreeRTOS (étape 7), options
    (r"core-freertos|freertos_", "differe", "FreeRTOS (étape 7)"),
    (r"embOSCXM3_|embOSCXM4_440|embOSCXM4_518|embOSCXM7_", "differe",
     "embOS IAR autre cœur/version (remplacé par le port GCC Segger)"),
    (r"stm32f1|stm32f2|stm32wl|samd20|samv7|samv71|same70|at91samd20|at91samv7x|k60|"
     r"stellaris|lm3s|(^|/)asf(/|$)|softpack|armcm0|armcm3|armcm7|armcm23|armcm33|armsc|"
     r"armv8m", "differe", "autre carte / cœur Cortex-M (étape 6)"),
    (r"nxp-?nfc|nxpnfc|pn7150", "differe", "bibliothèque NFC optionnelle — HYPOTHÈSE À VALIDER"),
    (r"^sys/user/tauon_sampleapp/", "differe", "application d'exemple hors socle"),
    # --- actif : socle Cortex-M + STM32F4
    (r"^sys/root/src/kernel/core/ucore/embOSCXM4_386/", "actif",
     "embOS IAR utilisé par les projets STM32F4 (remplacé par le port GCC)"),
    (r"^sys/root/src/kernel/core/ucore/cmsis-5/device/arm/armcm4/", "actif",
     "CMSIS-5 ARMCM4 (mps2-an386)"),
    (r"^sys/root/src/kernel/core/ucore/cmsis/device/(?!st/stm32f4xx)", "differe",
     "CMSIS device autre famille"),
    (r"^sys/root/src/kernel/core/ucore/cmsis-5/", "differe", "CMSIS-5 autre cœur"),
    (r"^sys/root/src/kernel/core/", "actif", "noyau / KAL / CMSIS commun"),
    (r"^sys/root/src/kernel/(fs|net|usb)/", "actif", "VFS / réseau / USB STM32F4"),
    (r"^sys/root/src/kernel/dev/(dev_\w+|arch/all|arch/cmsis|arch/cortexm/stm32f4xx)/",
     "actif", "pilotes logiciels / communs / STM32F4"),
    (r"^sys/root/src/kernel/dev/bsp/(discovery_f4|discovery_f4-baseboard-modem|olimex_p407|"
     r"stm32f469i-eval)/", "actif", "BSP STM32F4"),
    (r"^sys/root/src/(lib|bin|sbin)/", "actif", "lib / bin / sbin"),
    (r"^sys/root/prj/(iar|scons|config)/", "actif", "projets (source d'information)"),
    (r"^sys/user/tauon-basic/", "actif", "application tauon-basic (STM32F4 / commun)"),
    (r"^tools/mklepton/", "actif", "mklepton (porté à l'étape 2)"),
    (r"^tools/", "differe", "outil hôte non classé — HYPOTHÈSE À VALIDER"),
]
REGLES_COMPILEES = [(re.compile(p, re.I), e, j) for p, e, j in REGLES_HEURISTIQUES]

ORIGINE_TIERS = re.compile(
    r"(^|/)(ucore|cubemx_hal_driver|hal_driver|cmsis|cmsis-5|CMSIS|driverlib|at91lib|asf|softpack-lib|"
    r"lwip|uip|uip2\.5|fatfs/core|yaffs|mongoose|stm32f4-usb-core|lib-nxpnfc|WpdPack\w*|ecos|"
    r"install|nanox|"
    # pile radio ST SubGHz_Phy et utilitaires STM32CubeWL (décision 2026-10-05) ; la couche
    # d'adaptation stm32_radio_target/ (liaison au pilote Lepton) reste du code Lepton
    r"radio_subghz_phy(?!/stm32_radio_target/)|stm32wlxx/Utilities)(/|$)", re.I)
RE_GENERE = re.compile(r"(^|/)((bin|dev|kernel|fs)_mkconf\.[ch]|dev_dskimg\.[ch])$")


def norm_ensemble(v):
    v = unicodedata.normalize("NFKD", v.strip().lower())
    v = "".join(c for c in v if not unicodedata.combining(c))
    if v.startswith("act"):
        return "actif"
    if v.startswith("diff"):
        return "differe"
    if v.startswith("gel"):
        return "gele"
    if v.startswith("hors"):
        return "hors_projet"
    return "non_classe"


class Ventilateur:
    def __init__(self, mode, fichier_csv):
        self.mode = mode
        self.table = {}
        self.dirs = []
        self.source_csv = None
        if mode in ("auto", "csv") and os.path.isfile(fichier_csv):
            self.source_csv = fichier_csv
            with open(fichier_csv, newline="", encoding="utf-8") as f:
                sample = f.read(4096)
                f.seek(0)
                try:
                    dialect = csv.Sniffer().sniff(sample, delimiters=",;\t")
                except csv.Error:
                    dialect = csv.excel
                for row in csv.DictReader(f, dialect=dialect):
                    row = {(k or "").strip().lower(): (v or "") for k, v in row.items()}
                    p = row.get("fichier", "").strip().replace("\\", "/")
                    while p.startswith("./"):
                        p = p[2:]
                    if not p:
                        continue
                    # chemins relatifs à sys/root/ (src/…, prj/…) ou au trunk (sys/user, tools)
                    if p.split("/", 1)[0] in ("src", "prj"):
                        p = "sys/root/" + p
                    ens = norm_ensemble(row.get("ensemble", ""))
                    just = row.get("justification", "").strip()
                    if p.endswith("/"):
                        self.dirs.append((p, ens, just))
                    else:
                        self.table[p] = (ens, just)
            self.dirs.sort(key=lambda t: -len(t[0]))
        elif mode == "csv":
            sys.exit("audit_iar: --perimetre csv mais %s absent" % fichier_csv)

    @property
    def libelle(self):
        if self.source_csv and self.mode == "auto":
            return "perimetre.csv (%s) + repli heuristique pour les fichiers non couverts" % \
                os.path.relpath(self.source_csv, CLONE_ROOT)
        if self.source_csv:
            return "perimetre.csv seul (%s)" % os.path.relpath(self.source_csv, CLONE_ROOT)
        return ("HEURISTIQUE PAR CHEMIN (perimetre.csv absent) — provisoire, à relancer "
                "quand doc/migration/perimetre.csv existera")

    def classer(self, rel):
        if self.source_csv:
            if rel in self.table:
                return self.table[rel] + ("csv",)
            for d, e, j in self.dirs:
                if rel.startswith(d):
                    return e, j, "csv"
            if self.mode == "csv":
                return "non_classe", "absent de perimetre.csv", "csv"
        for rx, e, j in REGLES_COMPILEES:
            if rx.search(rel):
                return e, j, "heuristique"
        return "differe", "défaut heuristique", "heuristique"


# ---------------------------------------------------------------- analyse lexicale C
# commentaire // (avec continuation), /* */, chaîne "…", caractère '…' (non fermés : fin de ligne)
RE_LEX = re.compile(r"""(?P<com>//[^\n]*(?:\\\n[^\n]*)*|/\*.*?(?:\*/|\Z))
                      |(?P<q>["'])(?P<s>(?:\\.|(?!(?P=q))[^\\\n])*)(?P<f>(?P=q))?""",
                    re.S | re.X)


RE_NON_NL = re.compile(r"[^\n]+")


def _blanc(s):
    if "\n" not in s:
        return " " * len(s)
    return RE_NON_NL.sub(lambda m: " " * len(m.group(0)), s)


def strip_c(text):
    """Remplace commentaires et contenus de chaînes/caractères par des espaces
    (lignes conservées). Retourne (code, chaines) ; chaines = [(offset, ligne, contenu)]."""
    out = []
    chaines = []
    last = 0
    for m in RE_LEX.finditer(text):
        out.append(text[last:m.start()])
        if m.group("com") is not None:
            out.append(_blanc(m.group("com")))
        else:
            q = m.group("q")
            contenu = m.group("s")
            if q == '"':
                chaines.append((m.start(), text.count("\n", 0, m.start()) + 1, contenu))
            out.append(q + _blanc(contenu) + (m.group("f") or ""))
        last = m.end()
    out.append(text[last:])
    return "".join(out), chaines


def extrait(s):
    s = " ".join(s.strip().split())
    return s[:140]


# ---------------------------------------------------------------- analyseurs
def analyse_pragma(nom, reste):
    n = nom.lower()
    if n in PRAGMA_IAR or n.startswith("diag_") or n.startswith("cstat"):
        return "pragma", "#pragma " + nom, "iar"
    if n in PRAGMA_GCC_OK:
        return "pragma", "#pragma " + nom, "a_verifier"
    return "pragma_autre", "#pragma " + nom, "autre"


def analyse_c(rel, text, mklepton_src):
    hits = []
    code, chaines = strip_c(text)
    lignes_code = code.split("\n")
    lignes_brutes = text.split("\n")
    chemin_cmsis = bool(RE_CHEMIN_CMSIS.search("/" + rel))

    def add(no, cat, motif, sev):
        brut = lignes_brutes[no - 1] if no - 1 < len(lignes_brutes) else ""
        hits.append((no, cat, motif, sev, extrait(brut)))

    for idx, l in enumerate(lignes_code):
        no = idx + 1
        if "#" not in l and "__" not in l and "@" not in l:
            continue
        m_def = RE_PP_DEF.match(l)
        est_def = bool(m_def)
        # pragmas
        m = RE_PRAGMA.match(l)
        if m:
            cat, motif, sev = analyse_pragma(m.group(1), m.group(2))
            add(no, cat, motif, sev)
            continue
        # includes
        if RE_INCLUDE.match(l):
            brut = lignes_brutes[idx]
            mh = HDR_IAR.match(brut)
            if mh:
                add(no, "header", mh.group(1).strip(), "iar")
            continue
        # gardes
        if RE_PP_COND.match(l):
            g = GUARD_IAR.findall(l)
            if g:
                add(no, "garde_iar", " ".join(sorted(set(g))), "iar")
            g = GUARD_AUTRE.findall(l)
            if g:
                add(no, "garde_autre", " ".join(sorted(set(g))), "autre")
            g = GUARD_GCC.findall(l)
            if g:
                add(no, "garde_gcc", " ".join(sorted(set(g))), "a_verifier")
            g = GUARD_ABSTR.findall(l)
            if g and not GUARD_IAR.search(l) and not GUARD_AUTRE.search(l):
                add(no, "garde_compilateur", " ".join(sorted(set(g))), "a_verifier")
            continue
        if est_def and (GUARD_IAR.search(m_def.group(2)) or GUARD_AUTRE.search(m_def.group(2))
                        or GUARD_GCC.search(m_def.group(2))):
            add(no, "garde_compilateur", "#define " + m_def.group(2), "a_verifier")
            continue
        # mots-clés étendus
        for kw in sorted(set(RE_KW.findall(l))):
            if est_def and m_def.group(2) == kw:
                add(no, "mot_cle", kw + " (définition de compatibilité)", "a_verifier")
            elif kw in KW_MACRO_CMSIS and chemin_cmsis:
                add(no, "mot_cle", kw + " (macro CMSIS/HAL)", "a_verifier")
            else:
                add(no, "mot_cle", kw, "iar")
        # placement @ (hors commentaires/chaînes ; C n'a pas d'autre usage de @)
        if "@" in l and not l.lstrip().startswith("#"):
            add(no, "placement_@", "placement @", "iar")
        # intrinsics
        for it in sorted(set(RE_INTR_IAR.findall(l))):
            sev = "a_verifier" if (est_def and m_def.group(2) == it) else "iar"
            add(no, "intrinsic", it, sev)
        for it in sorted(set(RE_INTR_CMSIS.findall(l))):
            add(no, "intrinsic_cmsis", it, "a_verifier")
        for s in sorted(set(SYM_IAR.findall(l))):
            add(no, "symbole_iar", s, "iar")
        for s in sorted(set(SYM_DLIB_IO.findall(l))):
            add(no, "symbole_dlib_io", s, "a_verifier")
        for s in sorted(set(RE_ABSTR_LEPTON.findall(l))):
            if not est_def:
                add(no, "abstraction_lepton", s, "a_verifier")
    # _Pragma("...")
    for m in RE_UPRAGMA.finditer(code):
        suiv = [c for c in chaines if c[0] >= m.end()]
        if not suiv:
            continue
        off, ligne, contenu = suiv[0]
        mm = re.match(r"\s*([A-Za-z_]\w*)", contenu.replace('\\"', '"'))
        if not mm:
            continue
        cat, motif, sev = analyse_pragma(mm.group(1), "")
        no = code.count("\n", 0, m.start()) + 1
        add(no, cat, "_Pragma " + mm.group(1), sev)
    # modèles de génération mklepton : IAR-ismes dans les chaînes émises
    if mklepton_src:
        for off, ligne, contenu in chaines:
            # lignes physiques (continuation \) ; \n échappé = fin de ligne logique (\x01) ;
            # commentaires du texte émis (en-tête de licence, adresses e-mail) exclus
            c = contenu.replace("\\\n", "\n").replace("\\n", "\x01").replace('\\"', '"')
            c = strip_c(c)[0]
            for i, phys in enumerate(c.split("\n")):
                for sl in phys.split("\x01"):
                    mp = re.match(r"\s*#\s*pragma\s+(\w+)", sl)
                    motifs = []
                    if mp:
                        motifs.append("#pragma " + mp.group(1))
                    motifs += RE_KW.findall(sl)
                    if "@" in sl and not sl.lstrip().startswith("#"):
                        motifs.append("placement @")
                    for mo in motifs:
                        add(ligne + i, "modele_mklepton", mo, "iar")
    return hits


RE_GNU_DIR = re.compile(r"^\s*\.(section|global|globl|syntax|thumb|text|data|word|equ|set|"
                        r"type|size|align|balign|cpu|fpu|thumb_func|weak|extern|macro|end)\b",
                        re.I)
RE_IAR_DIR = re.compile(r"^\s*(?:\w+\s*:?\s+)?(MODULE|NAME|RSEG|ASEG|PUBWEAK|SECTION\s+\S+:\S*|"
                        r"DC32|DC16|DC8|DS32|DS8|CODE32|CODE16|THUMB|ARM|REQUIRE|"
                        r"SECTION_TYPE|ALIGNROM|ALIGNRAM|PUBLIC|EXTERN|IMPORT|EXPORT|END|"
                        r"LTORG|CFI)(?=\s|$)")
RE_IAR_FORT = re.compile(r"^\s*(?:\w+\s*:?\s+)?(MODULE|RSEG|ASEG|PUBWEAK|SECTION\s+\S+:\S*|DC32|"
                         r"CODE32|CODE16|REQUIRE|ALIGNROM|NAME)(?=\s|$)")
RE_KEIL_FORT = re.compile(r"^\s*(?:\w+\s+)?(AREA|PRESERVE8|THUMB\s*$|ENDP|PROC|"
                          r"EXPORT\s+\w+\s+\[WEAK\])")


def role_asm(rel, text):
    nom = os.path.basename(rel).lower()
    roles = []
    t = text
    if "startup" in nom or "cstartup" in nom or re.search(r"\bReset_Handler\b|__iar_program_start|"
                                                             r"\?cstartup|_start\b", t):
        roles.append("démarrage")
    if re.search(r"__vector_table|__Vectors|\.intvec|_vectors|\bVectors\b|isr_vector|"
                 r"interrupt_vector", t):
        roles.append("vecteurs")
    # commutation : code qui manipule PSP ou symboles de port connus (pas une simple
    # entrée PendSV_Handler dans une table de vecteurs)
    if re.search(r"portasm|OS_Switch|vPortSVCHandler|xPortPendSVHandler|OS_ChangeTask|"
                 r"\bmrs\s+\w+\s*,\s*psp\b|\bmsr\s+psp\b|stmdb\s+\w+!\s*,\s*\{r4", t + nom, re.I) and "hardfault" not in nom:
        roles.append("commutation de contexte")
    if "hardfault" in nom:
        roles.append("HardFault")
    if "rtt" in nom:
        roles.append("SEGGER RTT")
    if "syscall" in nom:
        roles.append("stub d'appels système")
    if "bootloader" in nom:
        roles.append("bootloader")
    if nom.endswith(".inc"):
        roles.append("include asm")
    if re.search(r"SDRAM|remap|EBI_|PMC_", t) and "démarrage" in roles:
        roles.append("init. horloges/mémoire")
    return ", ".join(roles) or "indéterminé"


def analyse_asm(rel, text):
    hits = []
    lignes = text.split("\n")
    n_gnu = n_iar = n_iar_fort = n_keil = 0
    # « ; » d'abord (commentaire IAR/ARMASM : des bannières « ;/**** » ouvriraient un faux
    # bloc C), puis /* */ (GNU et fichiers préprocessés)
    sans_com = "\n".join(re.sub(r";.*$", "", l) for l in text.split("\n"))
    sans_com = re.sub(r"/\*.*?\*/", lambda m: re.sub(r"[^\n]", " ", m.group(0)), sans_com,
                      flags=re.S)
    for l in sans_com.split("\n"):
        if l.lstrip().startswith("#"):
            continue
        l2 = re.sub(r";.*$", "", l)
        l2 = re.sub(r"//.*$", "", l2)
        if RE_GNU_DIR.match(l2):
            n_gnu += 1
        if RE_IAR_DIR.match(l2):
            n_iar += 1
        if RE_IAR_FORT.match(l2):
            n_iar_fort += 1
        if RE_KEIL_FORT.match(l2):
            n_keil += 1
    garde_iar = bool(re.search(r"__IAR_SYSTEMS_ASM__|__ICCARM__|_CCIAR\b", text))
    if garde_iar and n_gnu and (n_iar_fort or n_keil):
        syntaxe, cat, sev = "multi-assembleur (gardes)", "asm_multi", "a_verifier"
    elif n_keil > max(n_iar_fort, 0) and n_keil >= n_gnu:
        syntaxe, cat, sev = "ARMASM (Keil)", "asm_armcc", "autre"
    elif n_iar_fort or (n_iar > n_gnu and n_iar >= 2):
        syntaxe, cat, sev = "IAR", "asm_iar", "iar"
    elif n_gnu:
        syntaxe, cat, sev = "GNU", "asm_gnu", "info"
    else:
        syntaxe, cat, sev = "indéterminée", "asm_inconnu", "a_verifier"
    role = role_asm(rel, text)
    hits.append((1, cat, "%s — %s" % (syntaxe, role), sev, "%d lignes" % len(lignes)))
    # gardes préprocesseur dans les .S / .s
    for idx, l in enumerate(lignes):
        if RE_PP_COND.match(l):
            g = GUARD_IAR.findall(l)
            if g:
                hits.append((idx + 1, "garde_iar", " ".join(sorted(set(g))), "iar", extrait(l)))
    return hits, syntaxe, role


RE_WINPATH = re.compile(r"\b[a-zA-Z]:[/\\]")


def analyse_xml(rel, text):
    hits = []
    for idx, l in enumerate(text.split("\n")):
        motifs = []
        if RE_WINPATH.search(l):
            motifs.append(("chemin Windows absolu", "a_verifier"))
        if re.search(r"\.(icf|xcl|ewp|mac)\b", l, re.I):
            motifs.append(("référence projet/linker IAR", "iar"))
        for kw in set(RE_KW.findall(l)):
            motifs.append((kw, "iar"))
        if re.search(r"#\s*pragma\s+(\w+)", l):
            motifs.append(("pragma dans modèle", "iar"))
        for mo, sev in motifs:
            hits.append((idx + 1, "xml_mklepton", mo, sev, extrait(l)))
    return hits


def ext_de(nom):
    base, e = os.path.splitext(nom)
    if e == ".S":
        return ".S"
    return e.lower()


def lire(path):
    try:
        with open(path, "rb") as f:
            data = f.read()
    except OSError:
        return None
    if b"\0" in data[:8192]:
        return None
    return data.decode("latin-1").replace("\r\n", "\n").replace("\r", "\n")


# ---------------------------------------------------------------- parcours
def auditer(trunk, vent):
    lignes = []      # dicts
    asm = []         # (rel, syntaxe, role, ensemble, origine)
    nb_fichiers = collections.Counter()
    for racine, dirs, fichiers in os.walk(trunk, followlinks=True):
        relr = os.path.relpath(racine, trunk)
        dirs[:] = sorted(d for d in dirs if not (relr == "." and d in DIRS_EXCLUS)
                         and d != ".git")
        for nom in sorted(fichiers):
            path = os.path.join(racine, nom)
            rel = os.path.normpath(os.path.join(relr, nom)).replace(os.sep, "/")
            e = ext_de(nom)
            genre = None
            if e in EXT_C:
                genre = "c"
            elif e == ".S" or e in EXT_ASM:
                genre = "asm"
            elif e in EXT_PROJET_IAR:
                genre = "projet"
            elif e == ".xml" and ("mkconf" in nom.lower() or rel.startswith("tools/mklepton")):
                genre = "xml"
            if genre is None:
                continue
            ens, just, src = vent.classer(rel)
            origine = "tiers" if ORIGINE_TIERS.search(rel) else "lepton"
            genere = "oui" if RE_GENERE.search(rel) else ""
            nb_fichiers[(genre, ens)] += 1
            if genere:
                nb_fichiers[("genere", rel)] += 1
            if genre == "projet":
                hits = [(1, "fichier_iar", "fichier " + e, "info", "")]
            else:
                text = lire(path)
                if text is None:
                    continue
                if genre == "c":
                    hits = analyse_c(rel, text, rel.startswith("tools/mklepton/src/"))
                elif genre == "asm":
                    hits, syn, role = analyse_asm(rel, text)
                    asm.append((rel, syn, role, ens, origine))
                else:
                    hits = analyse_xml(rel, text)
            for no, cat, motif, sev, ext in hits:
                lignes.append(dict(fichier=rel, ligne=no, categorie=cat, motif=motif,
                                   severite=sev, ensemble=ens, origine=origine, genere=genere,
                                   ventilation=src, justification=just, extrait=ext))
    return lignes, asm, nb_fichiers


# ---------------------------------------------------------------- sorties
def totaux(lignes):
    t = collections.Counter()
    for r in lignes:
        t[(r["severite"], r["ensemble"])] += 1
    return t


def texte_summary(lignes, vent):
    t = totaux(lignes)
    actif_iar = t[("iar", "actif")]
    lep = sum(1 for r in lignes if r["severite"] == "iar" and r["ensemble"] == "actif"
              and r["origine"] == "lepton")
    out = ["audit_iar — ventilation : " + vent.libelle,
           "TOTAL IAR-ISMES PÉRIMÈTRE ACTIF : %d  (code Lepton : %d, tiers : %d)"
           % (actif_iar, lep, actif_iar - lep)]
    out.append("%-12s %8s %8s %8s %12s %10s" % ("sévérité", "actif", "différé", "gelé",
                                                 "hors_projet", "non_classé"))
    for s in SEVERITES:
        out.append("%-12s %8d %8d %8d %12d %10d" % (s, t[(s, "actif")], t[(s, "differe")],
                                                     t[(s, "gele")], t[(s, "hors_projet")],
                                                     t[(s, "non_classe")]))
    return "\n".join(out)


def ecrire_csv(lignes, chemin):
    champs = ["fichier", "ligne", "categorie", "motif", "severite", "ensemble", "origine",
              "genere", "ventilation", "justification", "extrait"]
    with open(chemin, "w", newline="", encoding="utf-8") as f:
        w = csv.DictWriter(f, fieldnames=champs)
        w.writeheader()
        for r in sorted(lignes, key=lambda r: (r["fichier"], r["ligne"], r["categorie"])):
            w.writerow(r)


def repertoire(rel, prof=8):
    parts = rel.split("/")[:-1]
    return "/".join(parts[:prof]) or "."


def md_table(entetes, rangs):
    rangs = list(rangs)
    num = [bool(rangs) and all(isinstance(r[i], int) for r in rangs)
           for i in range(len(entetes))]
    out = ["| " + " | ".join(entetes) + " |",
           "|" + "|".join("---:" if num[i] else "---" for i in range(len(entetes))) + "|"]
    for r in rangs:
        out.append("| " + " | ".join(str(x) for x in r) + " |")
    return "\n".join(out)


def ecrire_md(lignes, asm, nb_fichiers, vent, trunk, chemin, argv):
    t = totaux(lignes)
    iar = [r for r in lignes if r["severite"] == "iar"]
    actif_iar = [r for r in iar if r["ensemble"] == "actif"]
    lep = sum(1 for r in actif_iar if r["origine"] == "lepton")
    presents = {r["ensemble"] for r in lignes}
    ens_cols = [e for e in ENSEMBLES if e in ("actif", "differe", "gele") or e in presents]
    L = []
    L.append("# Audit des IAR-ismes — étape 1, tâche 3\n")
    L.append("Généré par `tools/migration/audit_iar.py` le %s — ne pas éditer à la main.  "
             % datetime.date.today().isoformat())
    L.append("Commande : `python3 tools/migration/audit_iar.py%s` ; trunk lu : `%s`.  "
             % ((" " + " ".join(argv)) if argv else "", trunk))
    L.append("Détail ligne à ligne : `doc/migration/audit-iar.csv`.\n")
    L.append("**Ventilation : %s.**\n" % vent.libelle)
    L.append("## Métrique\n")
    L.append("> **Total IAR-ismes périmètre actif (sévérité `iar`) : %d** — code Lepton : %d, "
             "code tiers vendored dans l'arbre (CMSIS, HAL ST, embOS IAR, lwIP…) : %d.\n"
             % (len(actif_iar), lep, len(actif_iar) - lep))
    L.append("Métrique décroissante des étapes 3-4 (`--summary` pour la seule relever). "
             "Sévérités : `iar` = à traiter (comptée) ; `autre` = Keil/MSVC, à retirer ; "
             "`a_verifier` = portable ou toléré par GCC (CMSIS, `#pragma pack/weak`, garde "
             "`__GNUC__`, définition de compatibilité) ; `info` = inventaire (projets IAR, asm GNU). "
             "Ensemble `hors_projet` (perimetre.csv) : fichier déclaré par aucun projet ni inclus, "
             "hors métrique.\n")
    L.append("## Totaux sévérité × ensemble (occurrences)\n")
    L.append(md_table(["sévérité"] + ens_cols + ["total"],
                      [[s] + [t[(s, e)] for e in ens_cols] + [sum(t[(s, e)] for e in ens_cols)]
                       for s in SEVERITES]))
    L.append("")
    L.append("## Catégorie × ensemble (sévérité `iar`)\n")
    cats = collections.Counter((r["categorie"], r["ensemble"]) for r in iar)
    cat_noms = sorted({c for c, _ in cats}, key=lambda c: -sum(cats[(c, e)] for e in ens_cols))
    L.append(md_table(["catégorie"] + ens_cols + ["total"],
                      [[c] + [cats[(c, e)] for e in ens_cols] + [sum(cats[(c, e)] for e in ens_cols)]
                       for c in cat_noms]))
    L.append("")
    L.append("## Catégorie × ensemble (toutes sévérités hors `iar`)\n")
    autres = [r for r in lignes if r["severite"] != "iar"]
    cats2 = collections.Counter((r["categorie"], r["severite"], r["ensemble"]) for r in autres)
    cles = sorted({(c, s) for c, s, _ in cats2}, key=lambda k: (k[1], k[0]))
    L.append(md_table(["catégorie", "sévérité"] + ens_cols,
                      [[c, s] + [cats2[(c, s, e)] for e in ens_cols] for c, s in cles]))
    L.append("")
    L.append("## Motifs du périmètre actif (sévérité `iar`, top 40)\n")
    mot = collections.Counter((r["categorie"], r["motif"]) for r in actif_iar)
    L.append(md_table(["catégorie", "motif", "occurrences"],
                      [[c, "`%s`" % m, n] for (c, m), n in mot.most_common(40)]))
    L.append("")
    L.append("## Répertoires du périmètre actif (sévérité `iar`, top 30)\n")
    rep = collections.Counter()
    rep_o = {}
    for r in actif_iar:
        d = repertoire(r["fichier"])
        rep[d] += 1
        rep_o[d] = r["origine"]
    L.append(md_table(["répertoire", "origine", "occurrences"],
                      [["`%s`" % d, rep_o[d], n] for d, n in rep.most_common(30)]))
    L.append("")
    L.append("## Répertoires différé / gelé (sévérité `iar`, top 15 chacun)\n")
    for e in ("differe", "gele"):
        rep = collections.Counter(repertoire(r["fichier"]) for r in iar if r["ensemble"] == e)
        L.append("**%s**\n" % e)
        L.append(md_table(["répertoire", "occurrences"],
                          [["`%s`" % d, n] for d, n in rep.most_common(15)]))
        L.append("")
    L.append("## Fichiers assembleur (%d)\n" % len(asm))
    L.append("Syntaxe déduite des directives (IAR : `MODULE`/`RSEG`/`SECTION x:CODE`/`DC32`/"
             "`PUBWEAK` ; ARMASM : `AREA`/`PRESERVE8`/`DCD` ; GNU : `.section`/`.global`/…) ; "
             "rôle déduit du nom et des symboles.\n")
    syn = collections.Counter((s, e) for _, s, _, e, _ in asm)
    L.append(md_table(["syntaxe"] + ens_cols,
                      [[s] + [syn[(s, e)] for e in ens_cols] for s in sorted({s for s, _ in syn})]))
    L.append("")
    ordre = {e: i for i, e in enumerate(ENSEMBLES)}
    L.append(md_table(["fichier", "syntaxe", "rôle", "ensemble", "origine"],
                      [["`%s`" % r, s, ro, e, o] for r, s, ro, e, o in
                       sorted(asm, key=lambda a: (ordre[a[3]], a[0]))]))
    L.append("")
    # fichiers de projet
    L.append("## Fichiers de projet IAR (inventaire, sévérité `info`)\n")
    proj = collections.Counter((r["motif"], r["ensemble"]) for r in lignes
                               if r["categorie"] == "fichier_iar")
    L.append(md_table(["type"] + ens_cols,
                      [[m] + [proj[(m, e)] for e in ens_cols]
                       for m in sorted({m for m, _ in proj})]))
    L.append("")
    # points notables calculés
    L.append("## Points notables\n")
    def cnt(pred):
        return sum(1 for r in lignes if pred(r))
    L.append("- `__compiler_directive__packed` (abstraction Lepton, `kernel/core/kernelconf.h`) : "
             "vaut `__packed` sous IAR/Keil et **rien sous GCC** — les structures concernées "
             "perdent l'attribut packed : %d usages (catégorie `abstraction_lepton`, dont %d actifs). "
             "Point à traiter à l'étape 4 (`compiler.h`)."
             % (cnt(lambda r: r["motif"] == "__compiler_directive__packed"),
                cnt(lambda r: r["motif"] == "__compiler_directive__packed"
                    and r["ensemble"] == "actif")))
    L.append("- Sélection du compilateur par `__tauon_compiler__` (`kernelconf.h`, valeurs "
             "`__compiler_iar_arm__`, `__compiler_gnuc__`, `__compiler_keil_arm__`…) : "
             "%d gardes `garde_iar` sur `__compiler_iar_*`, %d sur `__ICCARM__`/`__IAR_SYSTEMS_*` "
             "(tous ensembles)."
             % (cnt(lambda r: r["categorie"] == "garde_iar" and "__compiler_iar" in r["motif"]),
                cnt(lambda r: r["categorie"] == "garde_iar" and "__compiler_iar" not in r["motif"])))
    L.append("- `_Pragma` dans des macros (ex. `CORTEXM4_CCM_RAM` : `section`/`location` pour la "
             "CCM du STM32F4) : %d occurrences détectées via `_Pragma(\"…\")`."
             % cnt(lambda r: r["motif"].startswith("_Pragma")))
    L.append("- Modèles émis par mklepton (`tools/mklepton/src`) : %d IAR-ismes dans les chaînes "
             "générées (catégorie `modele_mklepton`, `#pragma memory=constseg` M16C) ; XML "
             "`mkconf*` : %d chemins Windows absolus (`c:/tauon/…`, sévérité `a_verifier`), "
             "%d références IAR." % (
                 cnt(lambda r: r["categorie"] == "modele_mklepton"),
                 cnt(lambda r: r["categorie"] == "xml_mklepton" and "Windows" in r["motif"]),
                 cnt(lambda r: r["categorie"] == "xml_mklepton" and r["severite"] == "iar")))
    gen = sorted(k[1] for k in nb_fichiers if k[0] == "genere")
    L.append("- Fichiers générés par mklepton présents dans l'arbre (colonne `genere`) : %d "
             "fichiers (%s) ; %d occurrences toutes sévérités, dont %d `iar`."
             % (len(gen), ", ".join("`%s`" % g for g in gen) or "aucun",
                cnt(lambda r: r["genere"] == "oui"),
                cnt(lambda r: r["genere"] == "oui" and r["severite"] == "iar")))
    L.append("- Code tiers : %d des %d IAR-ismes actifs sont dans du code vendored (CMSIS/HAL "
             "fournissent déjà des branches GCC ; les copies embOS IAR de `ucore/` sont remplacées "
             "par le port GCC Segger, non traduites)." % (len(actif_iar) - lep, len(actif_iar)))
    L.append("")
    L.append("## Limites et faux positifs connus\n")
    L.append("- Pas d'évaluation du préprocesseur : les blocs `#if 0` et les branches IAR déjà "
             "gardées sont comptés (une branche `#if defined(__ICCARM__)` compte la garde et son "
             "contenu).")
    L.append("- Commentaires, chaînes et doxygen (`@note`, `@brief`) exclus par analyse lexicale ; "
             "`placement_@` = tout `@` restant dans le code C hors directive préprocesseur.")
    L.append("- `__weak`/`__packed`/`__noreturn` sous `cmsis`/`cubemx_hal_driver` classés "
             "`a_verifier` (macros CMSIS/HAL portables) ; ailleurs comptés `iar` même si une "
             "macro de compatibilité les rend portables.")
    L.append("- `__get_*`/`__set_*`/`__DSB`… (`intrinsic_cmsis`) : fournis par CMSIS pour GCC, "
             "`a_verifier` ; `__enable_interrupt`, `__get_interrupt_state`, `__section_begin`… : "
             "IAR seul, `iar`.")
    L.append("- Syntaxe asm déduite par comptage de directives : un fichier IAR très court ou "
             "multi-assembleur peut être mal classé (voir la colonne `motif`).")
    L.append("- Asm en ligne (`asm(\"…\")`, `__asm`) non audité : syntaxe proche entre IAR et GCC, "
             "contraintes d'opérandes à vérifier à la compilation de masse (étape 4).")
    if vent.source_csv is not None:
        n_h = sum(1 for r in lignes if r["ventilation"] == "heuristique")
        L.append("- Ventilation : %d occurrences portent sur des fichiers absents de "
                 "perimetre.csv (XML mkconf, projets IAR, asm `.s79`…) et sont ventilées par "
                 "l'heuristique de chemin (colonne `ventilation`)." % n_h)
    if vent.source_csv is None:
        L.append("- **Ventilation heuristique** : règles par chemin (`REGLES_HEURISTIQUES` du "
                 "script), dont certaines marquées `HYPOTHÈSE À VALIDER` (eCos, NFC, outils). "
                 "Relancer quand `doc/migration/perimetre.csv` existera.")
    L.append("")
    L.append("## Fichiers analysés\n")
    L.append(md_table(["genre"] + ens_cols,
                      [[g] + [nb_fichiers[(g, e)] for e in ens_cols]
                       for g in ("c", "asm", "projet", "xml")]))
    L.append("")
    with open(chemin, "w", encoding="utf-8") as f:
        f.write("\n".join(L))


def main():
    ap = argparse.ArgumentParser(description="Audit des IAR-ismes de l'arbre Lepton.")
    ap.add_argument("--trunk", default=os.environ.get("LEPTON_TRUNK")
                    or os.path.expanduser("~/lepton/trunk"))
    ap.add_argument("--perimetre", choices=("auto", "csv", "heuristique"), default="auto")
    ap.add_argument("--perimetre-file", default=DEFAULT_PERIMETRE)
    ap.add_argument("--out-dir", default=DEFAULT_OUT)
    ap.add_argument("--summary", action="store_true",
                    help="affiche seulement les totaux, n'écrit aucun fichier")
    a = ap.parse_args()
    trunk = os.path.abspath(a.trunk)
    if not os.path.isdir(trunk):
        sys.exit("audit_iar: trunk introuvable : %s" % trunk)
    out = os.path.abspath(a.out_dir)
    if os.path.realpath(out).startswith(os.path.realpath(trunk) + os.sep):
        sys.exit("audit_iar: écriture dans le trunk interdite")
    vent = Ventilateur(a.perimetre, a.perimetre_file)
    lignes, asm, nb = auditer(trunk, vent)
    print(texte_summary(lignes, vent))
    if a.summary:
        return
    argv = [x for x in sys.argv[1:]]
    ecrire_csv(lignes, os.path.join(out, "audit-iar.csv"))
    ecrire_md(lignes, asm, nb, vent, trunk, os.path.join(out, "audit-iar.md"), argv)
    print("écrit : %s, %s" % (os.path.relpath(os.path.join(out, "audit-iar.md"), CLONE_ROOT),
                             os.path.relpath(os.path.join(out, "audit-iar.csv"), CLONE_ROOT)))


if __name__ == "__main__":
    main()
