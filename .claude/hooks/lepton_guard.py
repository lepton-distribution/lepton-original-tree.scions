#!/usr/bin/env python3
"""Garde-fous PreToolUse du chantier Lepton.

1. Opérations git distantes (push, et création de PR par gh) : Claude Code doit DEMANDER
   l'accord de l'utilisateur à chaque fois (décision 2026-09-30 : git local uniquement).
2. Refus de toute écriture dans le trunk scion.

Le trunk est une vue de liens symboliques ; y écrire modifie un fichier versionné du clone
à l'insu de git (écriture à travers un lien) ou crée un fichier régulier qui bloque
« scion graft ». Toute écriture se fait dans le clone, puis « scion graft ».
Code de sortie 2 = action refusée, le message de stderr est rendu à Claude Code.
"""
import json
import os
import re
import sys


def find_rootstock(start):
    d = os.path.realpath(start)
    while True:
        if os.path.isfile(os.path.join(d, ".scion.rootstock.signature")):
            return d
        parent = os.path.dirname(d)
        if parent == d:
            return None
        d = parent


def trunks(rootstock):
    found = []
    for name in os.listdir(rootstock):
        p = os.path.join(rootstock, name)
        if os.path.isfile(os.path.join(p, ".scion.grafted.list")):
            found.append(os.path.realpath(p))
    return found


def inside(path, roots):
    # Le chemin n'est PAS résolu : un fichier du trunk est un lien vers le clone ;
    # seul son répertoire parent (réel) est normalisé.
    parent = os.path.realpath(os.path.dirname(os.path.abspath(path)))
    return any(parent == r or parent.startswith(r + os.sep) for r in roots)


# « git [options globales] push » ; « push » ailleurs (message de commit) ne déclenche rien.
REMOTE = re.compile(r"\bgit((\s+-[Cc]\s+\S+)|(\s+--[\w-]+(=\S+)?))*\s+push\b"
                    r"|\bgh\s+(pr\s+create|repo\s+sync)\b")


def ask(reason):
    """Force une demande d'accord à l'utilisateur (décision « ask » du hook PreToolUse)."""
    print(json.dumps({"hookSpecificOutput": {
        "hookEventName": "PreToolUse",
        "permissionDecision": "ask",
        "permissionDecisionReason": reason}}))
    return 0


def main():
    data = json.load(sys.stdin)
    tool = data.get("tool_name", "")
    params = data.get("tool_input", {}) or {}
    if tool == "Bash" and REMOTE.search(params.get("command", "")):
        return ask("Opération git distante : le chantier Lepton est en git local uniquement. "
                   "Accord explicite de l'utilisateur requis (branche et commits concernés).")
    project = os.environ.get("CLAUDE_PROJECT_DIR", data.get("cwd", os.getcwd()))
    rootstock = find_rootstock(project)
    if rootstock is None:          # amorçage (étape 0) : pas encore de rootstock
        return 0
    roots = trunks(rootstock)
    if not roots:
        return 0

    msg = ("Écriture refusée dans le trunk scion ({}). Éditer le fichier correspondant dans le "
           "clone (depots/lepton/original/master/scion/…), puis lancer « scion graft ».")

    if tool in ("Write", "Edit", "MultiEdit", "NotebookEdit"):
        path = params.get("file_path") or params.get("notebook_path") or ""
        if path and inside(path, roots):
            print(msg.format(path), file=sys.stderr)
            return 2
    elif tool == "Bash":
        cmd = params.get("command", "")
        mentions = any(r in cmd or os.path.basename(r) + "/" in cmd for r in roots)
        inplace = re.search(r"\bsed\s+(-[a-zA-Z]*i|--in-place)\b", cmd) \
            and "--follow-symlinks" not in cmd
        inplace = inplace or re.search(r"\bspatch\b.*--in-place", cmd)
        if inplace and mentions:
            print(msg.format("commande : " + cmd[:120]), file=sys.stderr)
            return 2
        # Redirections (> et >>, hors >&) dont la cible est dans le trunk.
        for target in re.findall(r"(?<![<>&0-9])>{1,2}(?!&)\s*([^\s;&|<>]+)", cmd):
            target = os.path.expandvars(os.path.expanduser(target))
            if not os.path.isabs(target):
                target = os.path.join(data.get("cwd", os.getcwd()), target)
            if inside(target, roots):
                print(msg.format("redirection vers " + target), file=sys.stderr)
                return 2
    return 0


if __name__ == "__main__":
    sys.exit(main())
