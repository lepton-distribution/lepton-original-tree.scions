#!/usr/bin/env python3
"""perimetre_complement.py — complète perimetre.csv avec les fichiers créés par la migration
(bilan de l'étape 4).

`perimetre.csv` (build_closure.py, étape 1) classe les sources du trunk tel qu'il était à l'étape 1
d'après les projets IAR. Les fichiers créés ou déplacés depuis (KAL décomposé, démarrage GCC,
pilotes QEMU, noyau statique hôte, bancs de test, copies `legacy/`) n'y figurent pas : les audits
(audit_iar.py, audit_isa_ifdef.py, mass_compile.py) ne les voyaient pas.

Le script, idempotent :
  1. retire les lignes dont le fichier n'existe plus dans le trunk, sauf renommage de casse connu
     (RENOMMAGES : la ligne est conservée sous le nouveau nom) ;
  2. ajoute chaque source du trunk (.c .h .s .S .asm .s79 .cpp, hors `building/`) absente du CSV,
     classée par la première règle de REGLES qui s'applique (aucune règle -> erreur : à classer) ;
  3. réécrit la section « Complément de l'étape 4 » de perimetre.md (totaux par ensemble).
Les lignes existantes ne sont pas reclassées. Lignes de code : cloc --by-file (colonne « code »)
s'il est installé, sinon lignes non vides.

Chemins : relatifs à sys/root/ sous sys/root/, sinon relatifs au trunk (convention de
build_closure.py ; `tests/` et `legacy/` sont relatifs au trunk).

Usage (racine du clone) : python3 tools/migration/perimetre_complement.py [--dry-run]
"""
import argparse
import collections
import csv
import os
import re
import shutil
import subprocess
import sys
import tempfile

HERE = os.path.dirname(os.path.realpath(__file__))
CLONE = os.path.realpath(os.path.join(HERE, "..", ".."))
EXTS = (".c", ".h", ".s", ".S", ".asm", ".s79", ".cpp")
CHAMPS = ["fichier", "ensemble", "justification", "lignes_code", "nb_projets"]

# ancien chemin -> nouveau (renommage sans changement de rôle : la ligne garde son classement)
RENOMMAGES = {
    "src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/inc/legacy/":
        "src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/inc/Legacy/",
}

# (regex sur le chemin, ensemble, justification) : première règle applicable
REGLES = [
    (r"^legacy/", "gelé",
     "étape 4 (D2a/D3a) : copie d'origine des branches gelées, supprimée à l'étape 6"),
    (r"^src/kernel/core/kal/arch/armv6m/kal_arch\.h$", "actif",
     "étape 6 : KAL, axe ISA ARMv6-M (SAMD21 Xplained Pro)"),
    (r"^src/kernel/core/kal/arch/armv6m/", "différé",
     "étape 4 (KAL-2) : réglages d'ISA M0/M0+ (HYPOTHÈSE À VALIDER), cœurs de l'étape 6"),
    (r"^src/kernel/core/kal/backend/freertos/", "actif",
     "étape 7 : KAL FreeRTOS (backend V11.3.0, région atomique, configuration)"),
    (r"^src/kernel/core/core-freertos/", "actif",
     "étape 7 : backend FreeRTOS (core-freertos remis à niveau, démarrage, crochets)"),
    (r"^src/kernel/net/lwip/ports/freertos/", "actif",
     "étape 7 : sys_arch lwIP du backend FreeRTOS"),
    (r"^src/kernel/core/ucore/freeRTOS_11-3-0/((tasks|queue|list|timers|event_groups)\.c$|include/|"
     r"portable/GCC/ARM_CM(3|4F|7/r0p1)/|portable/GCC/ARM_CM0/port(macro\.h|\.c|asm\.[ch])$)", "actif",
     "étape 7 : noyau FreeRTOS V11.3.0 compilé (tiers, 202604 LTS)"),
    (r"^src/kernel/core/ucore/freeRTOS_11-3-0/", "hors-projet",
     "étape 7 : FreeRTOS V11.3.0 non compilé (tiers : MemMang, stream_buffer, croutine, MPU)"),
    (r"^src/kernel/core/kal/", "actif",
     "étape 4 (KAL) : KAL décomposé (dispatcher, arch, backend, contrat)"),
    (r"^src/kernel/core/compiler\.h$", "actif",
     "étape 3 : abstraction compilateur (macros __lepton_*)"),
    (r"^src/kernel/core/arch/cortexm/", "actif",
     "étape 3 : démarrage et sections critiques Cortex-M (GCC)"),
    (r"^src/kernel/core/core-segger/", "actif",
     "étape 3 : backend embOS (démarrage, RTOSInit, verrou des appels système)"),
    (r"^src/kernel/core/(arch/host|core-static|include/libc)/", "actif",
     "étape 2 : noyau statique hôte (mklepton)"),
    (r"^src/kernel/dev/arch/host/", "actif",
     "étape 2 : pilotes du noyau statique hôte"),
    (r"^src/kernel/dev/arch/all/(uart/dev_cmsdk_uart|eth/dev_eth_lan9118)/", "actif",
     "étape 3 : pilote du socle QEMU mps2-an386"),
    (r"^src/kernel/dev/bsp/qemu_mps2_an386/", "actif",
     "étape 3 : BSP du socle QEMU mps2-an386"),
    (r"^sys/user/tauon-basic/src/arch/qemu-mps2-an386/", "actif",
     "étape 3 : configuration de la carte QEMU mps2-an386"),
    (r"^src/kernel/dev/bsp/qemu_mps2/", "actif",
     "étape 6 : BSP commun des machines QEMU MPS2 (an386, an500)"),
    (r"^sys/user/tauon-basic/src/arch/qemu-mps2/", "actif",
     "étape 6 : configuration commune des machines QEMU MPS2"),
    (r"^src/kernel/core/ucore/cmsis-5/CMSIS/Core/Include/core_cm0plus\.h$", "actif",
     "étape 6 : CMSIS-Core 5 du Cortex-M0+ (tiers, copie du paquet embOS)"),
    (r"^src/kernel/core/ucore/cmsis-5/CMSIS/Core/Include/", "actif",
     "étape 6 : CMSIS-Core 5 du Cortex-M7 (tiers, copie du paquet embOS)"),
    (r"^src/kernel/dev/bsp/nucleo_f439zi/", "actif",
     "étape 5 : BSP de la carte de base NUCLEO-F439ZI"),
    (r"^sys/user/tauon-basic/src/arch/nucleo-f439zi/", "actif",
     "étape 5 : configuration de la carte NUCLEO-F439ZI"),
    (r"^src/kernel/dev/arch/cortexm/stm32f7xx/hal_driver/(Inc/|"
     r"Src/stm32f7xx_hal_(gpio|eth|rcc|cortex)\.c$|LEPTON-PROVENANCE)", "actif",
     "étape 6 : HAL STM32F7 liée (tiers, STM32CubeF7 v1.17.4)"),
    (r"^src/kernel/dev/arch/cortexm/stm32f7xx/hal_driver/", "hors-projet",
     "étape 6 : HAL STM32F7 non liée à ce jour (tiers, STM32CubeF7 v1.17.4)"),
    (r"^src/kernel/core/ucore/cmsis-5/Device/ST/STM32F7xx/", "actif",
     "étape 6 : CMSIS Device STM32F7 (tiers, STM32CubeF7 v1.17.4)"),
    (r"^src/kernel/dev/arch/cortexm/stm32f7xx/dev_stm32f7xx/", "actif",
     "étape 6 : pilotes STM32F7 de Lepton"),
    (r"^src/kernel/dev/bsp/stm32f746g_disco/", "actif",
     "étape 6 : BSP de la carte STM32F746G-DISCO"),
    (r"^sys/user/tauon-basic/src/arch/stm32f746g-disco/", "actif",
     "étape 6 : configuration de la carte STM32F746G-DISCO"),
    (r"^src/kernel/core/ucore/cmsis-5/Device/Microchip/SAMD21/", "actif",
     "étape 6 : en-têtes de la SAMD21J18A (tiers, Microchip SAMD21_DFP 3.8.270)"),
    (r"^src/kernel/dev/arch/cortexm/samd21/dev_samd21/", "actif",
     "étape 6 : pilotes SAMD21 de Lepton"),
    (r"^src/kernel/dev/bsp/samd21_xplained_pro/", "actif",
     "étape 6 : BSP de la carte SAMD21 Xplained Pro"),
    (r"^sys/user/tauon-basic/src/arch/samd21-xplained-pro/", "actif",
     "étape 6 : configuration de la carte SAMD21 Xplained Pro"),
    (r"^src/lib/libc/string/strerror\.c$", "actif",
     "étape 3b : frontière libc (strerror, numérotation errno Lepton)"),
    (r"^src/sbin/net/tsterrno\.c$", "actif",
     "étape 4 (kernel/net) : test errno lwIP / Lepton"),
    (r"^tests/(host|kal)/", "actif",
     "étapes 2-3 : bancs de test (ctest host, banc KAL)"),
]
REGLES = [(re.compile(rx), e, j) for rx, e, j in REGLES]

# lignes existantes à reclasser (fichier → ensemble, justification) : fichiers déjà inventoriés
# devenus utilisés par la migration
RECLASSEMENTS = {
    "src/kernel/core/ucore/cmsis-5/Device/ARM/ARMCM7/Include/ARMCM7_DP.h":
        ("actif", "étape 6 : modèle de périphériques Cortex-M7 de mps2-an500 (tiers, CMSIS 5)"),
    "src/kernel/core/ucore/cmsis-5/Device/ARM/ARMCM7/Include/system_ARMCM7.h":
        ("actif", "étape 6 : modèle de périphériques Cortex-M7 de mps2-an500 (tiers, CMSIS 5)"),
    "src/kernel/core/kal/arch/armv6m/kal_arch_conf.h":
        ("actif", "étape 6 : réglages d'ISA ARMv6-M validés sur la SAMD21 Xplained Pro"),
}
# HAL STM32F7 liée par la session réseau de la STM32F746G-DISCO (inventoriée hors-projet avant)
for _src in ("eth", "rcc", "cortex"):
    RECLASSEMENTS["src/kernel/dev/arch/cortexm/stm32f7xx/hal_driver/Src/stm32f7xx_hal_%s.c" % _src] = (
        "actif", "étape 6 : HAL STM32F7 liée (tiers, STM32CubeF7 v1.17.4)")

# NUCLEO-WL55JC1 (étape 6) : fichiers du portage IAR (différés) désormais compilés, et fichiers
# nouveaux de la carte ; session 2 : radio (SubGHz_Phy, utilitaires liés). Pilote cpu0, gabarits,
# reste de la HAL et des utilitaires : différés inchangés. Règles par chemin : reclassement des lignes existantes et ajout des nouvelles.
_WL = "src/kernel/dev/arch/cortexm/stm32wlxx/"
WL55 = [
    (r"^%scubemx_hal_driver/(inc/|src/stm32wlxx_hal(_(cortex|gpio|rcc|rcc_ex|pwr|pwr_ex|dma|"
     r"dma_ex|uart|uart_ex|subghz))?\.c$)" % _WL, "actif",
     "étape 6 : HAL STM32WL liée (tiers, STM32CubeWL, HAL V1.3.0)"),
    (r"^%sdev_stm32wlxx/(dev_stm32wlxx_(uart_x\.[ch]|definitions\.h|hal_tick\.c|"
     r"bsp_radio_if\.[ch]|util_timer\.c)|stm32wlxx_hal_conf\.h)$" % _WL, "actif",
     "étape 6 : pilotes STM32WL de Lepton"),
    # session 2 : pile radio SubGHz_Phy liée (tiers) et sa couche d'adaptation (Lepton)
    (r"^%sradio_subghz_phy/(radio(_def|_ex)?\.h|lr_fhss_v1_base_types\.h|stm32_radio_driver/|"
     r"stm32_radio_core_inc/)" % _WL, "actif",
     "étape 6 : pile radio SubGHz_Phy 1.3.0 liée (tiers, STM32CubeWL)"),
    (r"^%sradio_subghz_phy/stm32_radio_target/" % _WL, "actif",
     "étape 6 : adaptation de la pile radio au pilote /dev/radio (Lepton)"),
    (r"^%sUtilities/(misc/stm32_mem\.[ch]|timer/stm32_timer\.h|trace/adv_trace/stm32_adv_trace\.h)$"
     % _WL, "actif", "étape 6 : utilitaires STM32CubeWL liés (tiers)"),
    (r"^src/bin/radiotst\.c$", "actif", "étape 6 : test radio /dev/radio (NUCLEO-WL55JC1)"),
    (r"^src/kernel/core/ucore/cmsis/Device/st/stm32wlxx/(stm32wl55xx|stm32wlxx|system_stm32wlxx)\.h$",
     "actif", "étape 6 : CMSIS Device STM32WL (tiers, STM32CubeWL)"),
    (r"^src/kernel/dev/bsp/stm32wl55jci_nucleo/(stm32wl55jci_nucleo(_system\.c|\.h)|"
     r"dev_stm32wl55jci_nucleo_peripherals/|dev_stm32wl55jci_nucleo_radio/(?!.*template))", "actif",
     "étape 6 : BSP de la carte NUCLEO-WL55JC1"),
    (r"^sys/user/tauon-basic/src/arch/nucleo-wl55jc1/", "actif",
     "étape 6 : configuration de la carte NUCLEO-WL55JC1"),
]
WL55 = [(re.compile(rx), e, j) for rx, e, j in WL55]
REGLES = WL55 + REGLES

DEBUT_MD = "<!-- perimetre_complement.py : début -->"
FIN_MD = "<!-- perimetre_complement.py : fin -->"


def disp(rel):
    return rel[len("sys/root/"):] if rel.startswith("sys/root/") else rel


def scan(trunk):
    out = set()
    for d, dirs, fs in os.walk(trunk, followlinks=True):
        if os.path.relpath(d, trunk) == ".":
            dirs[:] = [x for x in dirs if x != "building"]
        for f in fs:
            if f.endswith(EXTS):
                out.add(disp(os.path.relpath(os.path.join(d, f), trunk)))
    return out


def renomme(p):
    for old, new in RENOMMAGES.items():
        if p.startswith(old):
            return new + p[len(old):]
    return p


def lignes(trunk, rels):
    def chemin(r):
        return os.path.join(trunk, r if r.startswith(("sys/", "tools/", "tests/", "legacy/"))
                            else "sys/root/" + r)
    loc = {}
    cloc = shutil.which("cloc")
    if cloc and rels:
        with tempfile.TemporaryDirectory() as tmp:
            lst = os.path.join(tmp, "list.txt")
            rep = os.path.join(tmp, "cloc.csv")
            with open(lst, "w") as f:
                f.write("\n".join(chemin(r) for r in rels) + "\n")
            subprocess.run([cloc, "--by-file", "--csv", "--quiet", "--skip-uniqueness",
                            "--follow-links", "--list-file=" + lst, "--report-file=" + rep],
                           check=True, capture_output=True)
            par_chemin = {chemin(r): r for r in rels}
            for row in csv.reader(open(rep, encoding="utf-8", errors="replace")):
                if len(row) >= 5 and row[1] in par_chemin:
                    loc[par_chemin[row[1]]] = int(row[4])
    for r in rels:
        if r not in loc:
            loc[r] = sum(1 for l in open(chemin(r), encoding="latin-1") if l.strip())
    return loc


def main():
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--perimetre", default=os.path.join(CLONE, "doc/migration/perimetre.csv"))
    ap.add_argument("--md", default=os.path.join(CLONE, "doc/migration/perimetre.md"))
    ap.add_argument("--trunk", default=os.environ.get("LEPTON_TRUNK",
                                                      os.path.join(CLONE, "../../../../trunk")))
    ap.add_argument("--dry-run", action="store_true")
    a = ap.parse_args()
    trunk = os.path.realpath(a.trunk)

    rows = list(csv.DictReader(open(a.perimetre, encoding="utf-8")))
    present = scan(trunk)
    garde, retires, renommes, reclasses = [], [], 0, 0
    for row in rows:
        p = renomme(row["fichier"])
        if p != row["fichier"]:
            renommes += 1
            row["fichier"] = p
        if p in RECLASSEMENTS and (row["ensemble"], row["justification"]) != RECLASSEMENTS[p]:
            row["ensemble"], row["justification"] = RECLASSEMENTS[p]
            reclasses += 1
        for rx, e, j in WL55:
            if rx.search(p) and (row["ensemble"], row["justification"]) != (e, j):
                row["ensemble"], row["justification"] = e, j
                reclasses += 1
        (garde if p in present else retires).append(row)
    connus = {r["fichier"] for r in garde}
    nouveaux = sorted(present - connus)

    non_classes, ajout = [], []
    for p in nouveaux:
        regle = next(((e, j) for rx, e, j in REGLES if rx.search(p)), None)
        if regle is None:
            non_classes.append(p)
        else:
            ajout.append((p,) + regle)
    if non_classes:
        sys.exit("perimetre_complement: fichiers sans règle (à classer) :\n  " +
                 "\n  ".join(non_classes))
    loc = lignes(trunk, [p for p, _, _ in ajout])
    for p, e, j in ajout:
        garde.append({"fichier": p, "ensemble": e, "justification": j,
                      "lignes_code": str(loc[p]), "nb_projets": "0"})
    garde.sort(key=lambda r: r["fichier"])

    print("renommés %d, reclassés %d, retirés %d, ajoutés %d"
          % (renommes, reclasses, len(retires), len(ajout)))
    for r in retires:
        print("  retiré  %s (%s)" % (r["fichier"], r["ensemble"]))
    par_ens = collections.Counter(e for _, e, _ in ajout)
    print("  ajouts par ensemble : " + ", ".join("%s %d" % kv for kv in sorted(par_ens.items())))
    if a.dry_run:
        return

    # écriture : même dialecte que build_closure.py (csv.writer par défaut)
    with open(a.perimetre, "w", newline="", encoding="utf-8") as f:
        w = csv.DictWriter(f, fieldnames=CHAMPS)
        w.writeheader()
        w.writerows(garde)

    tot = collections.OrderedDict((e, [0, 0]) for e in ("actif", "différé", "gelé", "hors-projet"))
    for r in garde:
        t = tot.setdefault(r["ensemble"], [0, 0])
        t[0] += 1
        t[1] += int(r["lignes_code"] or 0)
    # ajouts cumulés (toutes exécutions) : lignes portant une justification de REGLES
    justifs = {j for _, _, j in REGLES}
    aj = collections.defaultdict(lambda: [0, 0])
    for r in garde:
        if r["justification"] in justifs:
            aj[r["ensemble"]][0] += 1
            aj[r["ensemble"]][1] += int(r["lignes_code"] or 0)
    md = open(a.md, encoding="utf-8").read()
    # liste cumulée des lignes retirées (exécutions précédentes, lue dans perimetre.md)
    m = re.search(r"^Lignes retirées : (.*)\.$", md, re.M)
    deja = re.findall(r"`([^`]+)`", m.group(1)) if m else []
    tous = deja + [r["fichier"] for r in retires if r["fichier"] not in deja]
    ligne_retires = "Lignes retirées : " + (", ".join("`%s`" % f for f in tous) or "aucune") + "."
    L = [DEBUT_MD, "", "## Complément de l'étape 4 (bilan)", "",
         "Généré par `tools/migration/perimetre_complement.py` (idempotent ; rejouer après "
         "`build_closure.py`). Ajoute les sources créées par la migration (étapes 2 à 4 : KAL "
         "décomposé, démarrage GCC, pilotes et BSP QEMU, noyau statique hôte, bancs `tests/`, "
         "copies `legacy/`), classées par règle de chemin (`REGLES` du script, colonne "
         "`justification`) ; retire les lignes des fichiers disparus ; applique les renommages "
         "(`inc/legacy` → `inc/Legacy`). Les tableaux ci-dessus restent ceux de l'étape 1.", "",
         ligne_retires, "",
         "| Ensemble | Fichiers ajoutés | Lignes ajoutées | Fichiers (total) | Lignes (total) |",
         "|---|---|---|---|---|"]
    for e, (n, l) in tot.items():
        L.append("| %s | %d | %d | %d | %d |" % (e, aj[e][0], aj[e][1], n, l))
    L.append("| **total** | %d | %d | %d | %d |" % (
        sum(v[0] for v in aj.values()), sum(v[1] for v in aj.values()),
        sum(v[0] for v in tot.values()), sum(v[1] for v in tot.values())))
    L += ["", FIN_MD]
    bloc = "\n".join(L) + "\n"
    if DEBUT_MD in md:
        md = md[:md.index(DEBUT_MD)] + bloc + md[md.index(FIN_MD) + len(FIN_MD) + 1:]
    else:
        md = md.rstrip("\n") + "\n\n" + bloc
    open(a.md, "w", encoding="utf-8").write(md)


if __name__ == "__main__":
    main()
