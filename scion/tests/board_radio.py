#!/usr/bin/env python3
"""board_radio.py — test radio entre deux cartes (étape 6, NUCLEO-WL55JC1, FSK 868 MHz).

Deux cartes identiques, chacune avec sa console série et sa sonde ; la même image est flashée sur
les deux (--reset-a, --reset-b). Dans l'ordre, sur /dev/radio (pilote du portage IAR) :
  1. radiotst rx (B) / radiotst tx (A), puis l'inverse : n messages, aucune perte admise ;
  2. radiotst pong (B) / radiotst ping (A) : n allers-retours, aucune perte admise ;
  3. fumée lsh (décision 2026-10-05) : « cat /dev/radio & » sur une carte, « echo <jeton> >
     /dev/radio » sur l'autre, dans les deux sens (en dernier : le cat reste en arrière-plan et
     garde /dev/radio ouvert en lecture).
Courte portée (quelques dizaines de mètres) ; période d'émission de radiotst (1 s par défaut) :
rapport cyclique de 1 % de la sous-bande 868,0-868,6 MHz respecté (radiotst.c).
Code de retour 0 si tout est vert.
"""
import argparse
import os
import re
import shlex
import subprocess
import sys
import time

sys.path.insert(0, os.path.dirname(os.path.realpath(__file__)))
from smoke_lsh import Console, SerialLink, PROMPT, run_command  # noqa: E402

BOOT = re.compile(rb"any key to continue|lepton#\d+\$ ")


def boot(con, reset, timeout):
    con.drain()
    subprocess.run(shlex.split(reset), check=True,
                   stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    if con.read_until(BOOT, timeout) is None:
        return False
    con.send("")
    return con.read_until(PROMPT, 10) is not None


def resultat(con, mode, timeout):
    """Dernière ligne « radiotst <mode> : … » puis invite ; texte de la ligne ou None."""
    out = con.read_until(re.compile(rb"radiotst " + mode.encode() + rb" : [^\r\n]*\r?\n"), timeout)
    if out is None:
        return None
    con.read_until(PROMPT, 5)
    return out.decode("latin-1").strip().splitlines()[-1]


def paire(tx, rx, mode_rx, mode_tx, n, periode, nom):
    """Lance mode_rx sur rx (attend « pret »), puis mode_tx sur tx ; vérifie les deux bilans."""
    echecs = []
    rx.drain()
    rx.send("radiotst %s %d" % (mode_rx, n))
    if rx.read_until(re.compile(rb"radiotst " + mode_rx.encode() + rb" : pret"), 10) is None:
        return ["%s : radiotst %s ne démarre pas" % (nom, mode_rx)]
    tx.drain()
    tx.send("radiotst %s %d %d" % (mode_tx, n, periode))
    duree = n * (periode / 1000.0 + 1.5) + 15
    ltx = resultat(tx, mode_tx, duree)
    lrx = resultat(rx, mode_rx, 20)
    print("%s : %s | %s" % (nom, ltx, lrx))
    for ligne, mode in ((ltx, mode_tx), (lrx, mode_rx)):
        if ligne is None:
            echecs.append("%s : pas de bilan radiotst %s" % (nom, mode))
        elif re.search(r"(perdus|perdues)=(?!0\b)\d+", ligne) or \
                not re.search(r"=%d/%d" % (n, n), ligne):
            echecs.append("%s : %s" % (nom, ligne))
    return echecs


def fumee(tx, rx, nom):
    # jeton sans caractère spécial de lsh (« > » : redirection) : « A->B » → « A-vers-B »
    jeton = "lepton-radio-%s-%d" % (re.sub(r"[^A-Za-z0-9-]", "", nom.replace("->", "-vers-")),
                                    os.getpid())
    rx.drain()
    rx.send("cat /dev/radio &")
    rx.read_until(PROMPT, 5)
    time.sleep(1.0)
    if run_command(tx, "echo %s > /dev/radio" % jeton, 10) is None:
        return ["fumée %s : echo sans retour à l'invite" % nom]
    if rx.read_until(re.compile(re.escape(jeton.encode())), 5) is None:
        return ["fumée %s : jeton non reçu" % nom]
    print("fumée %s : jeton reçu" % nom)
    return []


def main():
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--port-a", required=True)
    ap.add_argument("--port-b", required=True)
    ap.add_argument("--reset-a", required=True, help="flash et reset de la carte A")
    ap.add_argument("--reset-b", required=True, help="flash et reset de la carte B")
    ap.add_argument("--frames", type=int, default=20)
    ap.add_argument("--period", type=int, default=1000, help="période d'émission (ms)")
    ap.add_argument("--baud", type=int, default=115200)
    ap.add_argument("--log", default="board_radio.log")
    a = ap.parse_args()
    echecs = []
    with open(a.log, "wb") as log:
        la, lb = SerialLink(a.port_a, a.baud), SerialLink(a.port_b, a.baud)
        ca, cb = Console(la, log), Console(lb, log)
        try:
            for nom, con, reset in (("A", ca, a.reset_a), ("B", cb, a.reset_b)):
                if not boot(con, reset, 30):
                    echecs.append("carte %s : pas d'invite après reset" % nom)
            if not echecs:
                echecs += paire(ca, cb, "rx", "tx", a.frames, a.period, "A->B")
                echecs += paire(cb, ca, "rx", "tx", a.frames, a.period, "B->A")
                echecs += paire(ca, cb, "pong", "ping", a.frames, a.period, "ping A, pong B")
                echecs += fumee(ca, cb, "A->B")
                echecs += fumee(cb, ca, "B->A")
        finally:
            la.kill()
            lb.kill()
    for e in echecs:
        print("ÉCHEC : " + e)
    print("board_radio : %s ; journal %s" % ("vert" if not echecs else "ÉCHEC", a.log))
    return 1 if echecs else 0


if __name__ == "__main__":
    sys.exit(main())
