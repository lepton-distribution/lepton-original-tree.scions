#!/usr/bin/env python3
"""mkconf_relpaths.py — chemins des mkconf rendus relatifs à la racine du trunk (étape 2).

Décision 2026-09-30 (étape 0) : pas de « tauon » dans $HOME ; les mkconf utilisent des chemins
relatifs à la racine de l'arbre, résolus par « mklepton -s <trunk> » (et -o pour les sorties).

Règle : dans les valeurs d'attributs XML, le préfixe d'un ancien poste
  c:/tauon/, C:\\tauon\\, $(HOME)/tauon/, /home/<utilisateur>/tauon/
est supprimé et les « \\ » restants de la valeur deviennent « / ». Rien d'autre n'est modifié
(octets, fins de ligne et encodage conservés). Les mkconf des cibles gelées (code-gele.md) ne sont
pas touchés. Écrit dans le clone (scion/…), jamais dans le trunk.

Usage (racine du clone) : python3 tools/migration/mkconf_relpaths.py [--dry-run]
"""
import os
import re
import sys

CLONE = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
SCION = os.path.join(CLONE, "scion")

# mkconf des cibles gelées : ARM7/ARM9, simulation gnu32/win32, K60 (code-gele.md, 2026-09-30).
FROZEN = {
    "sys/user/tauon-basic/etc/mkconf_tauon_basic_at91sam9261-ek.xml": "ARM9 AT91SAM9261",
    "sys/user/tauon-basic/etc/mkconf_base.xml": "cibles gnu32 et ARM9 seulement",
    "sys/user/tauon-basic/etc/mkconf_complet.xml": "cibles gnu32 et ARM9 seulement",
    "sys/user/tauon_sampleapp/etc/mkconf_tauon_sampleapp_gnu.xml": "simulation gnu32",
    "sys/user/tauon_sampleapp/etc/mkconf_tauon_sampleapp_gnu_simple.xml": "simulation gnu32",
    "sys/user/tauon_sampleapp/etc/mkconf_tauon_sampleapp_gnu_k60.xml": "Freescale K60",
    "tools/mklepton/mkconf.xml": "cibles gnu32, ARM7 et ARM9 seulement",
    "tools/mklepton/mkconf_9260.xml": "ARM9 AT91SAM9260",
    "tools/mklepton/prj/vc/mkconf.xml": "projet Visual C (simulation Windows)",
}

PREFIX = re.compile(rb'(?i)(?:c:[\\/]+tauon|\$\(home\)/tauon|/home/[a-z0-9_.-]+/tauon)[\\/]+')
ATTR = re.compile(rb'="([^"]*)"')


def rewrite_value(m):
    val = m.group(1)
    if not PREFIX.search(val):
        return m.group(0)
    val = PREFIX.sub(b"", val).replace(b"\\", b"/")
    return b'="' + val + b'"'


def main():
    dry = "--dry-run" in sys.argv
    total = 0
    for root, dirs, files in os.walk(SCION):
        dirs.sort()
        for name in sorted(files):
            if not (name.lower().startswith("mkconf") and name.lower().endswith(".xml")):
                continue
            path = os.path.join(root, name)
            rel = os.path.relpath(path, SCION)
            if rel in FROZEN:
                print("gelé      %s (%s)" % (rel, FROZEN[rel]))
                continue
            data = open(path, "rb").read()
            new, n = ATTR.subn(rewrite_value, data)
            changed = sum(1 for a, b in zip(ATTR.findall(data), ATTR.findall(new)) if a != b)
            left = len(PREFIX.findall(new))
            total += changed
            print("%s %s : %d attribut(s) réécrit(s)%s" % ("simulé   " if dry else "réécrit  ", rel, changed,
                                                          ", %d préfixe(s) restant(s)" % left if left else ""))
            if changed and not dry:
                open(path, "wb").write(new)
    print("total : %d attribut(s)" % total)


if __name__ == "__main__":
    main()
