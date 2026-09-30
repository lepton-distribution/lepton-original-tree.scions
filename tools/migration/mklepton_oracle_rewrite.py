#!/usr/bin/env python3
"""Réécriture d'un mkconf*.xml pour mklepton_oracle.sh (aucune écriture hors --out-xml/--report).

- dest_path des éléments de SORTIE (<mklepton>, <arch>, <boot>, <mount>) -> --dest
  (les dest_path de <binaries>, <file>, <directory> sont des chemins DANS l'image : inchangés) ;
- $(HOME) -> --home ;
- chemins sources (src_file, src_path) de l'ancien poste (c:/tauon, ~/tauon, /home/x/tauon,
  $(HOME)/tauon, /opt/lepton) -> --trunk (lecture seule).
Modes : --list-targets FICHIER ; --list-outputs FICHIER.
"""
import argparse, re, sys, xml.etree.ElementTree as ET

OUTPUT_TAGS = {"mklepton", "arch", "boot", "mount"}
SRC_PREFIX = re.compile(r"^(?:\$\(HOME\)/tauon|~/tauon|[A-Za-z]:[\\/]tauon|/home/[^/]+/tauon|/opt/lepton)(?=[\\/])")


def list_targets(path):
    seen = []
    for t in ET.parse(path).getroot().iter("target"):
        n = t.get("name")
        if n and n not in seen:
            seen.append(n)
    print("\n".join(seen))


def list_outputs(path):
    for e in ET.parse(path).getroot().iter():
        if e.tag in OUTPUT_TAGS and e.get("dest_path"):
            print(e.get("dest_path"))


def rewrite(a):
    tree = ET.parse(a.inp)
    rep = []
    for e in tree.getroot().iter():
        for k, v in list(e.attrib.items()):
            nv = v
            if e.tag in OUTPUT_TAGS and k == "dest_path":
                nv = a.dest
            elif k in ("src_file", "src_path"):
                nv = SRC_PREFIX.sub(a.trunk, v).replace("\\", "/")
            if "$(HOME)" in nv:
                nv = nv.replace("$(HOME)", a.home)
            if nv != v:
                e.set(k, nv)
                rep.append(f"{e.tag}@{k}: {v} -> {nv}")
    tree.write(a.out_xml, encoding="utf-8", xml_declaration=True)
    with open(a.report, "w") as f:
        f.write("\n".join(rep) + "\n")


def main():
    p = argparse.ArgumentParser()
    p.add_argument("--list-targets")
    p.add_argument("--list-outputs")
    p.add_argument("--in", dest="inp")
    p.add_argument("--out-xml")
    p.add_argument("--home")
    p.add_argument("--dest")
    p.add_argument("--trunk")
    p.add_argument("--report")
    a = p.parse_args()
    if a.list_targets:
        list_targets(a.list_targets)
    elif a.list_outputs:
        list_outputs(a.list_outputs)
    else:
        if not all([a.inp, a.out_xml, a.home, a.dest, a.trunk, a.report]):
            sys.exit("arguments manquants")
        rewrite(a)


if __name__ == "__main__":
    main()
