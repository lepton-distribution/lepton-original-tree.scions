#!/usr/bin/env python3
"""freertos_rebase.py — remise à niveau de core-freertos sur la lignée de core-segger (ETAPE-7,
tâche 2 ; décision utilisateur du 2026-10-06 : reprise de core-freertos).

Constat (kal-freertos-ecarts.md §1) : core-freertos descend d'une lignée plus ancienne du noyau.
L'écart core-segger d'origine (037cd59) → core-freertos mêle :
  - des blocs propres à FreeRTOS (API xTask/xSemaphore/xTimer…, __KERNEL_UCORE_FREERTOS) ;
  - l'en-tête de licence du fichier (auteurs), conservé ;
  - des différences de lignée (fonctions absentes, corrections manquantes), à abandonner au
    profit de core-segger.
Méthode, par fichier :
  1. base = core-segger@037cd59, F = core-freertos (clone), S = core-segger (clone) ;
  2. blocs de diff(base, F) classés : « freertos » (API FreeRTOS, ou code embOS remplacé ou
     retiré), « entete » (gardés) ou « ancien » (non
     gardé) ; classement forcé possible par freertos_rebase.json ({"fichier.c": {"3": "freertos"}}) ;
  3. F' = base + blocs gardés ; résultat = fusion 3 voies (git merge-file) de S et F' sur base :
     les corrections des étapes 3-6 (base → S) et les blocs FreeRTOS (base → F') se composent.
     Conflits laissés en marqueurs, à résoudre à la main (commit sémantique).

Opère sur le clone (scion/…), jamais sur le trunk ; Latin-1, fins de ligne conservées.

Usage : freertos_rebase.py [--clone <racine>] [--report <md>] [--apply] [fichier.c …]
"""
import argparse
import difflib
import json
import os
import re
import subprocess
import sys
import tempfile

CORE = "scion/sys/root/src/kernel/core"
BASE_REV = "037cd59"
HERE = os.path.dirname(os.path.abspath(__file__))
OVERRIDES = os.path.join(HERE, "freertos_rebase.json")

FREERTOS_RE = re.compile(
    r"\b(x|v|ux|pv|e)(Task|Queue|Semaphore|Timer|Port|EventGroup)[A-Za-z]*\b"
    r"|\bport[A-Z_][A-Za-z_]*\b|\bconfig[A-Z_][A-Za-z_]*\b|\bpd(TRUE|FALSE|PASS|FAIL)\b"
    r"|\b(xTaskHandle|xSemaphoreHandle|xTimerHandle|TimerHandle_t|TaskHandle_t|SemaphoreHandle_t"
    r"|TickType_t|BaseType_t|StaticTask_t|freertos_tcb_t)\b"
    r"|__KERNEL_UCORE_FREERTOS|(?i:freertos)")
EMBOS_RE = re.compile(r"\bOS_[A-Za-z]|__KERNEL_UCORE_EMBOS|RTOS\.h")
HEADER_LINES = 30


def git(clone, *args):
    return subprocess.run(["git", "-C", clone, *args], check=True, capture_output=True).stdout


def read(path):
    with open(path, encoding="latin-1", newline="") as fh:
        return fh.read().splitlines(keepends=True)


def hunks(base, other):
    sm = difflib.SequenceMatcher(None, base, other, autojunk=False)
    return [op for op in sm.get_opcodes() if op[0] != "equal"]


def classify(op, base, other, forced):
    tag, i1, i2, j1, j2 = op
    lines = base[i1:i2] + other[j1:j2]
    if forced:
        return forced, "forcé"
    if any(FREERTOS_RE.search(l) for l in lines):
        return "freertos", "mot-clé"
    if any(EMBOS_RE.search(l) for l in base[i1:i2]):
        return "freertos", "remplace embOS"
    if i1 < HEADER_LINES and j1 < HEADER_LINES:
        return "entete", "licence"
    return "ancien", "lignée"


def rebuild(base, other, kept):
    out, pos = [], 0
    for tag, i1, i2, j1, j2 in kept:
        out += base[pos:i1] + other[j1:j2]
        pos = i2
    return out + base[pos:]


def merge3(current, base, other):
    with tempfile.TemporaryDirectory() as d:
        paths = []
        for name, content in (("S", current), ("base", base), ("F", other)):
            p = os.path.join(d, name)
            with open(p, "w", encoding="latin-1", newline="") as fh:
                fh.write("".join(content))
            paths.append(p)
        r = subprocess.run(["git", "merge-file", "-p", "-L", "core-segger", "-L", "base-037cd59",
                            "-L", "core-freertos", *paths], capture_output=True)
        if r.returncode < 0 or r.returncode > 127:
            sys.exit("git merge-file : échec")
        return r.stdout.decode("latin-1"), r.returncode


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("--clone", default=os.path.realpath(os.path.join(HERE, "..", "..")))
    ap.add_argument("--report")
    ap.add_argument("--apply", action="store_true")
    ap.add_argument("files", nargs="*")
    a = ap.parse_args()
    forced_all = json.load(open(OVERRIDES)) if os.path.exists(OVERRIDES) else {}
    seg = os.path.join(a.clone, CORE, "core-segger")
    frt = os.path.join(a.clone, CORE, "core-freertos")
    files = a.files or sorted(f for f in os.listdir(frt) if f.endswith(".c"))
    rep = ["# Remise à niveau de core-freertos (freertos_rebase.py)", "",
           f"Base : core-segger@{BASE_REV}. Classes : freertos et entete gardés, ancien abandonné.", "",
           "| Fichier | Bloc | Lignes base | Classe | Raison | Extrait |", "|---|---|---|---|---|---|"]
    total = {}
    for f in files:
        S = read(os.path.join(seg, f))
        F = read(os.path.join(frt, f))
        B = git(a.clone, "show", f"{BASE_REV}:{CORE}/core-segger/{f}").decode("latin-1").splitlines(keepends=True)
        forced = forced_all.get(f, {})
        kept = []
        for n, op in enumerate(hunks(B, F)):
            cls, why = classify(op, B, F, forced.get(str(n)))
            total[cls] = total.get(cls, 0) + 1
            if cls != "ancien":
                kept.append(op)
            tag, i1, i2, j1, j2 = op
            ex = next((l.strip() for l in (F[j1:j2] or B[i1:i2]) if l.strip()), "")[:60]
            ex = ex.replace("|", "\\|").replace("`", "'")
            rep.append(f"| {f} | {n} | {i1 + 1}-{i2} | {cls} | {why} | `{ex}` |")
        merged, conflicts = merge3(S, B, rebuild(B, F, kept))
        rep.append(f"| {f} | — | — | **fusion** | {conflicts} conflit(s) | |")
        print(f"{f}: {len(hunks(B, F))} blocs, {conflicts} conflit(s)")
        if a.apply:
            with open(os.path.join(frt, f), "w", encoding="latin-1", newline="") as fh:
                fh.write(merged)
    rep += ["", "Totaux : " + ", ".join(f"{k} {v}" for k, v in sorted(total.items()))]
    print(rep[-1])
    if a.report:
        with open(a.report, "w", encoding="utf-8") as fh:
            fh.write("\n".join(rep) + "\n")


if __name__ == "__main__":
    main()
