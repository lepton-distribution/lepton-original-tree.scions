#!/usr/bin/env python3
"""occupation.py — occupation mémoire (flash, RAM) par bibliothèque, lue dans la MAP de GNU ld (BLD-4).

Usage : occupation.py <fichier.map> [--out <rapport.txt>]

Chaque section d'entrée retenue (partie « Linker script and memory map ») est attribuée à son
origine : archive libX.a → X ; objet d'une cible CMake (CMakeFiles/<cible>.dir/…) → <cible> ;
autre objet → son nom de fichier. Région : attribut « w » de « Memory Configuration » → RAM, sinon
flash. Une section initialisée en RAM avec adresse de chargement (.data) compte en RAM et en flash ;
.bss, COMMON, .noinit, .heap, .stack en RAM seulement. Sections de débogage ignorées.
Chaînes fusionnées (.str*, même adresse, taille d'avant fusion) : chaque octet n'est compté qu'une
fois, attribué à la première section d'entrée qui le couvre (tri par adresse, puis taille).
Totaux comparables à « size » (text + data = flash, data + bss = RAM), au remplissage
d'alignement (*fill*) près, non attribué.
Code de retour : 0 si succès, 1 si fichier illisible ou format inattendu.
"""
import argparse
import os
import re
import sys

REGION = re.compile(r"^(\S+)\s+0x([0-9a-fA-F]+)\s+0x([0-9a-fA-F]+)(?:\s+(\S+))?\s*$")
SORTIE = re.compile(r"^(\.\S+|COMMON)(?:\s+0x([0-9a-fA-F]+)\s+0x([0-9a-fA-F]+)(\s+load address\s+0x[0-9a-fA-F]+)?)?\s*$")
ENTREE = re.compile(r"^ (\S+)\s+0x([0-9a-fA-F]+)\s+0x([0-9a-fA-F]+)\s+(\S.*)$")
ENTREE_NOM = re.compile(r"^ (\S+)\s*$")
ENTREE_SUITE = re.compile(r"^\s+0x([0-9a-fA-F]+)\s+0x([0-9a-fA-F]+)\s+(\S.*)$")
IGNOREES = (".debug", ".comment", ".ARM.attributes", ".stab", ".note", ".gnu.attributes")
SANS_CHARGE = ("bss", "COMMON", "noinit", "heap", "stack")


def origine(chemin):
    m = re.search(r"([^/\\]+)\.a\(", chemin)
    if m:
        nom = m.group(1)
        return nom[3:] if nom.startswith("lib") else nom
    m = re.search(r"CMakeFiles/([^/]+)\.dir/", chemin)
    if m:
        return m.group(1)
    return os.path.basename(chemin)


def lire_map(lignes):
    regions = []   # (origine, longueur, ram)
    i = 0
    while i < len(lignes) and not lignes[i].startswith("Memory Configuration"):
        i += 1
    i += 1
    while i < len(lignes) and not lignes[i].startswith("Linker script and memory map"):
        m = REGION.match(lignes[i])
        if m and m.group(1) not in ("Name", "*default*"):
            regions.append((int(m.group(2), 16), int(m.group(3), 16), "w" in (m.group(4) or "")))
        i += 1
    if not regions or i >= len(lignes):
        raise ValueError("sections « Memory Configuration » ou « Linker script and memory map » absentes")

    def en_ram(adresse):
        for debut, longueur, ram in regions:
            if debut <= adresse < debut + longueur:
                return ram
        return None

    entrees = []   # (sortie, charge, nom, adresse, taille, origine)
    sortie, charge, attente = None, False, None
    for ligne in lignes[i + 1:]:
        if ligne.startswith("OUTPUT(") or ligne.startswith("LOAD linker stubs"):
            continue
        m = SORTIE.match(ligne)
        if m and not ligne.startswith(" "):
            sortie, charge, attente = m.group(1), bool(m.group(4)), None
            continue
        if sortie is None or sortie.startswith(IGNOREES):
            continue
        m = ENTREE.match(ligne)
        if m:
            nom, adresse, taille, fichier = m.group(1), int(m.group(2), 16), int(m.group(3), 16), m.group(4)
        else:
            m = ENTREE_NOM.match(ligne)
            if m and not ligne.strip().startswith(("*", "0x")):
                attente = m.group(1)
                continue
            m = ENTREE_SUITE.match(ligne)
            if not (m and attente):
                attente = None
                continue
            nom, adresse, taille, fichier = attente, int(m.group(1), 16), int(m.group(2), 16), m.group(3)
        attente = None
        if taille == 0 or nom == "*fill*" or fichier.startswith("load address"):
            continue
        entrees.append((sortie, charge, nom, adresse, taille, origine(fichier.strip())))

    flash, ram = {}, {}
    fin = {}   # fin de la zone déjà comptée, par section de sortie
    for sortie, charge, nom, adresse, taille, o in sorted(entrees, key=lambda e: (e[0], e[3], -e[4])):
        zone = en_ram(adresse)
        if zone is None:
            continue
        effectif = adresse + taille - max(adresse, fin.get(sortie, 0))
        fin[sortie] = max(fin.get(sortie, 0), adresse + taille)
        if effectif <= 0:
            continue
        if zone:
            ram[o] = ram.get(o, 0) + effectif
            if charge and not any(s in nom or s in sortie for s in SANS_CHARGE):
                flash[o] = flash.get(o, 0) + effectif
        else:
            flash[o] = flash.get(o, 0) + effectif
    return flash, ram


def rapport(chemin_map, flash, ram):
    noms = sorted(set(flash) | set(ram), key=lambda n: (-flash.get(n, 0), -ram.get(n, 0), n))
    largeur = max([len(n) for n in noms] + [len("bibliothèque")])
    lignes = [f"Occupation mémoire par bibliothèque — {os.path.basename(chemin_map)}",
              "", f"{'bibliothèque':<{largeur}}  {'flash':>9}  {'RAM':>9}"]
    for n in noms:
        lignes.append(f"{n:<{largeur}}  {flash.get(n, 0):>9}  {ram.get(n, 0):>9}")
    lignes.append(f"{'total':<{largeur}}  {sum(flash.values()):>9}  {sum(ram.values()):>9}")
    return "\n".join(lignes) + "\n"


def main():
    p = argparse.ArgumentParser(description="Occupation mémoire par bibliothèque (MAP GNU ld).")
    p.add_argument("map")
    p.add_argument("--out")
    a = p.parse_args()
    try:
        with open(a.map, encoding="utf-8", errors="replace") as f:
            flash, ram = lire_map(f.read().splitlines())
    except (OSError, ValueError) as e:
        print(f"occupation: {a.map} : {e}", file=sys.stderr)
        return 1
    texte = rapport(a.map, flash, ram)
    if a.out:
        with open(a.out, "w", encoding="utf-8") as f:
            f.write(texte)
    else:
        sys.stdout.write(texte)
    return 0


if __name__ == "__main__":
    sys.exit(main())
