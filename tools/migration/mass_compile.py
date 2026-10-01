#!/usr/bin/env python3
"""mass_compile.py — compilation de masse du périmètre actif (ETAPE-4 tâche 4).

Compile (« -c ») chaque fichier C du périmètre actif (`doc/migration/perimetre.csv`, ensemble
« actif ») et produit le tableau de bord : fichiers OK / total, par module, et histogramme des
erreurs (la première erreur de chaque fichier, normalisée : l'erreur la plus fréquente désigne la
prochaine règle de transformation).

Commande de chaque fichier :
  1. sa propre entrée dans le compile_commands.json du preset qui le compile (QEMU, puis hôte) ;
  2. sinon, un gabarit : l'entrée du preset QEMU la plus proche dans l'arborescence (plus long
     préfixe commun), dont le fichier source et la sortie sont remplacés, corrigée par le profil
     du répertoire (PROFILS : famille STM32F4, outils hôte…).
Le gabarit ne vaut pas configuration de carte : il mesure la compilabilité GCC, pas le produit.
Les objets sont écrits sous $LEPTON_BUILD/mass-compile/ (hors trunk et hors clone).

Prérequis : presets `qemu-mps2-an386-embos` et `host` configurés et construits (sources générées
par mklepton : generated/board).

Exemples :
  mass_compile.py                                    # tout le périmètre actif
  mass_compile.py --only sys/root/src/kernel/fs      # un module
  mass_compile.py --report doc/migration/mass-compile.md --csv doc/migration/mass-compile.csv
"""
import argparse
import collections
import concurrent.futures
import csv
import json
import os
import re
import shlex
import subprocess
import sys

HERE = os.path.dirname(os.path.realpath(__file__))
CLONE = os.path.realpath(os.path.join(HERE, "..", ".."))

# modules de l'étape 4 (tableau « Modules de l'étape 4 » de MIGRATION-STATUS.md) : préfixe → nom
MODULES = [
    ("sys/root/src/kernel/core/", "kernel/core"),
    ("sys/root/src/kernel/dev/", "kernel/dev"),
    ("sys/root/src/kernel/fs/", "kernel/fs"),
    ("sys/root/src/kernel/net/", "kernel/net"),
    ("sys/root/src/lib/", "lib"),
    ("sys/root/src/sbin/", "sbin"),
    ("sys/root/src/bin/", "bin"),
    ("sys/user/tauon-basic/", "tauon-basic"),
    ("tools/mklepton/", "tools/mklepton"),
]

# profils de compilation par préfixe (le plus long l'emporte) :
#   base : preset dont on prend le gabarit ; retire : options retirées du gabarit (regex) ;
#   ajoute : options ajoutées (chemins relatifs au trunk préfixés par « @/ ») ;
#   mkconf : mkconf de l'application de la carte (XML relatif au trunk) ; mass_compile le fait
#            générer par le mklepton hôte (cible cortexm_lepton) et son kernel_mkconf.h remplace
#            celui de QEMU (generated/board) : les pilotes STM32F4 reçoivent leur carte (UART_NB,
#            _GPIO_DEFAULT_SPEED…) par user_kernel_mkconf.h, comme sous IAR.
# HYPOTHÈSE À VALIDER : définitions STM32F4 relevées dans dev_stm32f4xx_8.40.ewp et
# tauon-basic_stm32f4* (STM32F429xx, USE_STDPERIPH_DRIVER) ; la configuration exacte de la
# NUCLEO-F439ZI est l'objet de l'étape 5.
# Base de la NUCLEO-F439ZI (étape 1) : application tauon-basic Olimex STM32-P407.
MKCONF_P407 = "sys/user/tauon-basic/etc/mkconf_tauon_basic_lwip_stm32f4-olimex-p407.xml"
STM32F4 = {
    "base": "qemu-mps2-an386-embos",
    "mkconf": MKCONF_P407,
    "retire": [r"-D__tauon_cpu_device__=\S+", r"-I\S*/bsp/qemu_mps2_an386",
               r"-I\S*/generated/board"],
    "ajoute": ["-D__tauon_cpu_device__=__tauon_cpu_device_cortexM4_stm32f4__",
               "-DUSE_STDPERIPH_DRIVER", "-DSTM32F429xx",
               "-I@/sys/root/src/kernel/core/ucore/cmsis/Device/st/stm32f4xx",
               "-I@/sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/inc",
               "-I@/sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/inc/Legacy",
               "-I@/sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/driverlib",
               "-I@/sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/dev_stm32f4xx"],
}
UIP = {"base": "qemu-mps2-an386-embos", "retire": [],
       "ajoute": ["-I@/sys/root/src/kernel/net/uip/core"]}
PROFILS = {
    "": {"base": "qemu-mps2-an386-embos", "retire": [], "ajoute": []},
    "sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/": STM32F4,
    # HAL CubeMX (tiers) : sans carte ; le mkconf de carte apporte la SPL (driverlib), dont les
    # types entrent en conflit avec ceux du HAL (RCC_PLLI2SInitTypeDef…)
    "sys/root/src/kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/": dict(
        STM32F4, mkconf=None, retire=STM32F4["retire"][:-1]),
    "sys/root/src/kernel/dev/bsp/discovery_f4/": dict(
        STM32F4, mkconf="sys/user/tauon-basic/etc/mkconf_tauon_basic_stm32f4-discovery.xml"),
    "sys/root/src/kernel/dev/bsp/olimex_p407/": STM32F4,
    "sys/root/src/kernel/dev/bsp/stm32f469i-eval/": dict(
        STM32F4, mkconf="sys/user/tauon-basic/etc/mkconf_tauon_basic_stm32f469i-eval.xml",
        ajoute=[o.replace("STM32F429xx", "STM32F469xx") for o in STM32F4["ajoute"]]),
    # pile uIP (contiki) : chemins d'inclusion de uip/core (projets IAR : uip/core et uip2.5)
    "sys/root/src/kernel/core/net/uip_core/": UIP,
    "sys/root/src/kernel/dev/arch/all/ppp/dev_ppp_uip/": UIP,
    "tools/": {"base": "host", "retire": [], "ajoute": []},
    "sys/root/src/kernel/core/arch/host/": {"base": "host", "retire": [], "ajoute": []},
}


def env(name, default):
    return os.path.realpath(os.environ.get(name) or default)


def actifs(perimetre):
    out = []
    for row in csv.DictReader(open(perimetre, encoding="utf-8")):
        if row["ensemble"] != "actif" or not row["fichier"].endswith(".c"):
            continue
        f = row["fichier"]
        out.append(f if f.startswith(("sys/", "tools/")) else "sys/root/" + f)
    return sorted(out)


def module_de(rel):
    for pre, nom in MODULES:
        if rel.startswith(pre):
            return nom
    return "autre"


def profil_de(rel):
    best = ""
    for pre in PROFILS:
        if rel.startswith(pre) and len(pre) > len(best):
            best = pre
    return PROFILS[best]


def charger_db(build, preset, trunk):
    path = os.path.join(build, preset, "compile_commands.json")
    if not os.path.exists(path):
        raise SystemExit("compile_commands.json absent : %s (configurer le preset %s)" % (path, preset))
    db = {}
    for ent in json.load(open(path)):
        if not ent["file"].startswith(trunk + "/") or not ent["file"].endswith(".c"):
            continue
        rel = ent["file"][len(trunk) + 1:]
        db[rel] = ent
    return db


def args_de(ent):
    if "arguments" in ent:
        return list(ent["arguments"])
    return shlex.split(ent["command"])


def sans_sortie(args):
    """Retire -o <x>, -c et le fichier source ; rend la liste d'options."""
    out, skip = [], False
    for a in args:
        if skip:
            skip = False
            continue
        if a == "-o":
            skip = True
            continue
        if a == "-c" or a.endswith((".c", ".S", ".s")):
            continue
        out.append(a)
    return out


def plus_proche(rel, db):
    def commun(a, b):
        pa, pb = a.split("/"), b.split("/")
        n = 0
        while n < min(len(pa), len(pb)) and pa[n] == pb[n]:
            n += 1
        return n
    return max(sorted(db), key=lambda k: commun(rel, k))


def generer_mkconf(xml, trunk, build):
    """kernel_mkconf.h de l'application xml (mklepton hôte, cible cortexm_lepton) ; rend le
    répertoire de sortie. Régénéré si le XML est plus récent."""
    out = os.path.join(build, "mass-compile", "mkconf",
                       os.path.splitext(os.path.basename(xml))[0])
    gen = os.path.join(out, "kernel_mkconf.h")
    if os.path.exists(gen) and os.path.getmtime(gen) >= os.path.getmtime(os.path.join(trunk, xml)):
        return out
    tool = os.path.join(build, "host", "mklepton")
    if not os.path.exists(tool):
        raise SystemExit("mklepton hôte absent : %s (cmake --build --preset host)" % tool)
    os.makedirs(out, exist_ok=True)
    r = subprocess.run([tool, "-s", trunk, "-o", out, "-t", "cortexm_lepton", xml], cwd=out,
                       capture_output=True, text=True, errors="replace",
                       env=dict(os.environ, SOURCE_DATE_EPOCH="0"))
    if r.returncode != 0 or not os.path.exists(gen):
        raise SystemExit("mklepton %s : échec\n%s" % (xml, r.stderr[-2000:]))
    return out


def commande(rel, dbs, trunk, objdir, mkconfs=None):
    prof = profil_de(rel)
    src = os.path.join(trunk, rel)
    obj = os.path.join(objdir, rel + ".o")
    if rel in dbs["qemu-mps2-an386-embos"] and prof is PROFILS[""]:
        ent, origine = dbs["qemu-mps2-an386-embos"][rel], "preset qemu"
    elif rel in dbs["host"] and prof["base"] == "host":
        ent, origine = dbs["host"][rel], "preset host"
    else:
        db = dbs[prof["base"]]
        ent, origine = db[plus_proche(rel, db)], "gabarit " + prof["base"]
    opts = sans_sortie(args_de(ent))
    for motif in prof["retire"]:
        opts = [o for o in opts if not re.fullmatch(motif, o)]
    opts += [o.replace("@/", trunk + "/") for o in prof["ajoute"]]
    if prof.get("mkconf"):
        # la puce (STM32F407xx…) est définie par le user_kernel_mkconf.h de l'application
        opts = [o for o in opts if not re.fullmatch(r"-DSTM32F4\w+", o)]
        opts.append("-I" + mkconfs[prof["mkconf"]])
    return opts + ["-o", obj, "-c", src], ent["directory"], origine


NORMS = [(re.compile(r"‘[^’]*’|'[^']*'|\"[^\"]*\""), "'X'"),
         (re.compile(r"\b\d+\b"), "N")]


def normaliser(msg):
    for rx, rep in NORMS:
        msg = rx.sub(rep, msg)
    return msg.strip()


RE_ERR = re.compile(r"^(\S+?):(\d+):(?:\d+:)? (?:fatal )?error: (.*)$")


def compiler(job):
    rel, cmd, cwd, origine = job
    os.makedirs(os.path.dirname(cmd[cmd.index("-o") + 1]), exist_ok=True)
    r = subprocess.run(cmd, cwd=cwd, capture_output=True, text=True, errors="replace")
    errs = []
    for line in r.stderr.splitlines():
        m = RE_ERR.match(line)
        if m:
            errs.append((m.group(1), int(m.group(2)), m.group(3)))
    nwarn = sum(1 for l in r.stderr.splitlines() if ": warning: " in l)
    return rel, r.returncode == 0, errs, nwarn, origine


def iar_par_module(audit_csv):
    c = collections.Counter()
    if not audit_csv or not os.path.exists(audit_csv):
        return None
    for row in csv.DictReader(open(audit_csv, encoding="utf-8")):
        if row["ensemble"] == "actif" and row["severite"] == "iar":
            rel = row["fichier"]
            c[(module_de(rel), row["origine"])] += 1
    return c


def main():
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0],
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--perimetre", default=os.path.join(CLONE, "doc/migration/perimetre.csv"))
    ap.add_argument("--only", action="append", default=[],
                    help="préfixe de chemin relatif au trunk (répétable)")
    ap.add_argument("--jobs", "-j", type=int, default=os.cpu_count() or 4)
    ap.add_argument("--report", help="rapport Markdown")
    ap.add_argument("--csv", help="résultat par fichier (CSV)")
    ap.add_argument("--audit-csv", help="audit-iar.csv : colonne IAR-ismes par module")
    ap.add_argument("--quiet", "-q", action="store_true")
    ap.add_argument("--db-out", help="écrit les commandes au format compile_commands.json (pour "
                                     "transform_iar.py --cpp-snapshot/--cpp-compare) sans compiler")
    a = ap.parse_args()

    trunk = env("LEPTON_TRUNK", os.path.join(CLONE, "../../../../trunk"))
    build = env("LEPTON_BUILD", os.path.join(trunk, "../build"))
    objdir = os.path.join(build, "mass-compile")
    dbs = {p: charger_db(build, p, trunk) for p in ("qemu-mps2-an386-embos", "host")}

    files = actifs(a.perimetre)
    if a.only:
        files = [f for f in files if f.startswith(tuple(a.only))]
    mkconfs = {}
    for rel in files:
        xml = profil_de(rel).get("mkconf")
        if xml and xml not in mkconfs:
            mkconfs[xml] = generer_mkconf(xml, trunk, build)
    jobs = []
    for rel in files:
        cmd, cwd, origine = commande(rel, dbs, trunk, objdir, mkconfs)
        jobs.append((rel, cmd, cwd, origine))
    if a.db_out:
        json.dump([{"directory": cwd, "file": cmd[-1], "command": shlex.join(cmd)}
                   for rel, cmd, cwd, origine in jobs], open(a.db_out, "w"), indent=1)
        print("mass_compile : %d commande(s) écrite(s) dans %s" % (len(jobs), a.db_out))
        return 0
    with concurrent.futures.ThreadPoolExecutor(a.jobs) as ex:
        res = sorted(ex.map(compiler, jobs))

    ok = sum(1 for r in res if r[1])
    par_mod = collections.defaultdict(lambda: [0, 0])
    histo = collections.Counter()
    exemples = {}
    for rel, good, errs, nwarn, origine in res:
        m = par_mod[module_de(rel)]
        m[1] += 1
        m[0] += good
        if not good:
            cle = normaliser(errs[0][2]) if errs else "(échec sans message d'erreur)"
            histo[cle] += 1
            exemples.setdefault(cle, rel)
    print("mass_compile : %d/%d fichiers OK (%.1f %%)" % (ok, len(res), 100.0 * ok / max(1, len(res))))
    if not a.quiet:
        for nom in [n for _, n in MODULES] + ["autre"]:
            if nom in par_mod:
                print("  %-16s %4d/%-4d" % (nom, *par_mod[nom]))
        for cle, n in histo.most_common(15):
            print("  %4d  %s" % (n, cle))

    if a.csv:
        with open(a.csv, "w", newline="", encoding="utf-8") as f:
            w = csv.writer(f)
            w.writerow(["fichier", "module", "statut", "commande", "avertissements",
                        "premiere_erreur"])
            for rel, good, errs, nwarn, origine in res:
                first = ""
                if errs:
                    first = "%s:%d: %s" % (errs[0][0].replace(trunk + "/", ""), errs[0][1], errs[0][2])
                w.writerow([rel, module_de(rel), "OK" if good else "ÉCHEC", origine, nwarn, first])
    if a.report:
        iar = iar_par_module(a.audit_csv)
        L = ["# Compilation de masse du périmètre actif — tableau de bord", "",
             "Généré par `tools/migration/mass_compile.sh` — ne pas éditer à la main.  ",
             "Commande : `%s`" % " ".join(["mass_compile.sh"] + sys.argv[1:]), "",
             "Résultat : **%d/%d fichiers C OK (%.1f %%)** ; `arm-none-eabi-gcc -c` (outils hôte : "
             "`cc -m32`)." % (ok, len(res), 100.0 * ok / max(1, len(res))), "",
             "Commande de chaque fichier : entrée du preset qui le compile, sinon gabarit du preset "
             "QEMU le plus proche corrigé par profil (voir l'en-tête du script). Détail par fichier : "
             "`mass-compile.csv`.", "",
             "## Par module", ""]
        if iar is not None:
            L += ["| Module | OK / total | IAR-ismes (Lepton) | IAR-ismes (tiers) |", "|---|---|---:|---:|"]
        else:
            L += ["| Module | OK / total |", "|---|---|"]
        for nom in [n for _, n in MODULES] + ["autre"]:
            if nom not in par_mod:
                continue
            o, t = par_mod[nom]
            if iar is not None:
                L.append("| %s | %d / %d | %d | %d |" % (nom, o, t, iar[(nom, "lepton")], iar[(nom, "tiers")]))
            else:
                L.append("| %s | %d / %d |" % (nom, o, t))
        L += ["", "## Histogramme des erreurs (première erreur de chaque fichier)", "",
              "| Fichiers | Erreur normalisée | Exemple |", "|---:|---|---|"]
        for cle, n in histo.most_common():
            L.append("| %d | `%s` | `%s` |" % (n, cle.replace("|", "\\|").replace("`", "'"), exemples[cle]))
        L += ["", "## Fichiers en échec", "", "| Fichier | Première erreur |", "|---|---|"]
        for rel, good, errs, nwarn, origine in res:
            if not good:
                e = ("%s:%d: %s" % (errs[0][0].replace(trunk + "/", ""), errs[0][1], errs[0][2])) if errs else ""
                L.append("| `%s` | `%s` |" % (rel, e.replace("|", "\\|").replace("`", "'")))
        open(a.report, "w", encoding="utf-8").write("\n".join(L) + "\n")
    return 0 if ok == len(res) else 1


if __name__ == "__main__":
    sys.exit(main())
