#!/usr/bin/env python3
"""endurance_board.py — palier 8 de l'étape 5 : endurance de la carte (lsh + réseau) et piles.

Ouvre la console série, écarte ce que la sonde a tamponné, lance --reset-command (reset par la
sonde), attend l'invite de lsh, puis pendant --duration secondes :
  - toutes les --period secondes, un cycle de commandes lsh (--command, défaut uname -a, ps,
    ls /usr/sbin, cat /usr/etc/.boot, pwd) : retour à l'invite exigé, sortie sans message
    d'erreur ;
  - un ping continu de la carte (--ping-ip, intervalle --ping-interval), compté à partir de la
    première réponse (réseau configuré par le .init après le reset), pertes comptées ;
    une coupure de plus de --max-ping-gap secondes est une faute ;
  - toute réapparition de la bannière de démarrage (« lepton start! ») est une faute
    (redémarrage, chien de garde).
À la fin : nombre de processus de « ps » identique au premier cycle (pas d'accumulation), puis
relevé des piles (commande gdb lepton-stacks) et des registres de faute (CFSR, HFSR nuls) par
OpenOCD + gdb (--openocd-cfg, --gdbinit, --elf). Journal de la console : --log ; rapport
résumé sur la sortie standard ; code de retour 0 si aucune faute.

Exemple :
  endurance_board.py --port /dev/ttyACM0 --ping-ip 192.168.2.5 --duration 10800 \\
      --reset-command "openocd -f debug/openocd-nucleo-f439zi.cfg -c init -c reset -c exit" \\
      --openocd-cfg debug/openocd-nucleo-f439zi.cfg --gdbinit debug/gdbinit-nucleo-f439zi \\
      --elf $LEPTON_BUILD/nucleo-f439zi-embos/lepton.elf --log endurance.log
"""
import argparse
import os
import re
import shlex
import subprocess
import sys
import threading
import time

# tests/ est lu par le trunk (vue de liens) : aucun __pycache__ à y écrire
sys.dont_write_bytecode = True
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from smoke_lsh import Console, SerialLink, PROMPT, ERRORS, run_command  # noqa: E402

BANNER = re.compile(rb"lepton start!")
PS_LINE = re.compile(rb"^\s*\d+\s+\d+\s+\d+\s", re.M)


class Pinger(threading.Thread):
    """Ping continu ; mémorise les compteurs et la plus longue coupure."""

    def __init__(self, ip, interval):
        super().__init__(daemon=True)
        self.ip, self.interval = ip, interval
        self.sent = self.received = 0
        self.last_ok = time.monotonic()
        self.max_gap = 0.0
        self.stop = threading.Event()

    def run(self):
        while not self.stop.is_set():
            t0 = time.monotonic()
            r = subprocess.run(["ping", "-c", "1", "-W", "1", self.ip],
                               stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
            now = time.monotonic()
            self.sent += 1
            if r.returncode == 0:
                self.received += 1
                self.last_ok = now
            self.max_gap = max(self.max_gap, now - self.last_ok)
            self.stop.wait(max(0.0, self.interval - (now - t0)))


def probe(args):
    """Relevé des piles et des registres de faute par OpenOCD + gdb (cœur arrêté le temps du
    relevé, puis relancé par « monitor resume »)."""
    ocd = subprocess.Popen(["openocd", "-f", args.openocd_cfg],
                           stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    try:
        time.sleep(2)
        r = subprocess.run(["gdb-multiarch", "-batch", "-x", args.gdbinit,
                            "-ex", "lepton-stacks",
                            "-ex", "printf \"CFSR=0x%08x HFSR=0x%08x\\n\", "
                                   "*(unsigned int*)0xE000ED28, *(unsigned int*)0xE000ED2C",
                            # l'attachement de gdb arrête le cœur : relance explicite
                            # (après detach seul, le cœur a été vu resté arrêté)
                            "-ex", "monitor resume", "-ex", "detach", args.elf],
                           capture_output=True, text=True, timeout=60)
        return r.stdout
    finally:
        ocd.terminate()
        ocd.wait()


def main():
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--port", required=True)
    ap.add_argument("--baud", type=int, default=115200)
    ap.add_argument("--reset-command", required=True)
    ap.add_argument("--ping-ip", required=True)
    ap.add_argument("--ping-interval", type=float, default=1.0)
    ap.add_argument("--max-ping-gap", type=float, default=10.0)
    ap.add_argument("--duration", type=float, default=3 * 3600)
    ap.add_argument("--period", type=float, default=30.0)
    ap.add_argument("--command", action="append")
    ap.add_argument("--boot-timeout", type=float, default=30.0)
    ap.add_argument("--timeout", type=float, default=10.0)
    ap.add_argument("--openocd-cfg", required=True)
    ap.add_argument("--gdbinit", required=True)
    ap.add_argument("--elf", required=True)
    ap.add_argument("--log", required=True)
    args = ap.parse_args()
    commands = args.command or ["uname -a", "ps", "ls /usr/sbin", "cat /usr/etc/.boot", "pwd"]

    failures = []
    cycles = 0
    ps_count0 = None
    with open(args.log, "wb") as log:
        link = SerialLink(args.port, args.baud)
        con = Console(link, log)
        con.drain()
        subprocess.run(shlex.split(args.reset_command), check=True,
                       stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        if con.read_until(re.compile(rb"any key to continue|lepton#\d+\$ "), args.boot_timeout) is None:
            failures.append("démarrage : pas d'invite")
        else:
            con.send("")
            if con.read_until(PROMPT, args.timeout) is None:
                failures.append("invite lsh absente")
        if not failures:
            deadline = time.monotonic() + args.boot_timeout
            while subprocess.run(["ping", "-c", "1", "-W", "1", args.ping_ip],
                                 stdout=subprocess.DEVNULL).returncode != 0:
                if time.monotonic() > deadline:
                    failures.append("réseau : %s sans réponse après le démarrage" % args.ping_ip)
                    break
        if failures:
            link.kill()
        else:
            print("début : %s" % time.strftime("%Y-%m-%d %H:%M:%S"), flush=True)
            pinger = Pinger(args.ping_ip, args.ping_interval)
            pinger.start()
            start = time.monotonic()
            try:
                while time.monotonic() - start < args.duration and not failures:
                    t0 = time.monotonic()
                    for c in commands:
                        out = run_command(con, c, args.timeout)
                        if out is None:
                            failures.append("cycle %d, %s : pas de retour à l'invite" % (cycles, c))
                            break
                        if BANNER.search(out):
                            failures.append("cycle %d : redémarrage de la carte" % cycles)
                            break
                        if ERRORS.search(out):
                            failures.append("cycle %d, %s : %r" % (cycles, c, out.strip()))
                            break
                        if c == "ps":
                            n = len(PS_LINE.findall(out))
                            if ps_count0 is None:
                                ps_count0 = n
                            elif n != ps_count0:
                                failures.append("cycle %d : ps %d processus (%d au premier cycle)"
                                                % (cycles, n, ps_count0))
                    if pinger.max_gap > args.max_ping_gap:
                        failures.append("ping : coupure de %.1f s" % pinger.max_gap)
                    cycles += 1
                    if cycles % 120 == 0:
                        print("%s : %d cycles, ping %d/%d" % (time.strftime("%H:%M:%S"), cycles,
                              pinger.received, pinger.sent), flush=True)
                    time.sleep(max(0.0, args.period - (time.monotonic() - t0)))
            finally:
                pinger.stop.set()
                pinger.join()
                link.kill()
            elapsed = time.monotonic() - start
            print("fin : %s ; %.0f s, %d cycles de %d commandes, ping %d/%d (perte %.3f %%), "
                  "plus longue coupure %.1f s"
                  % (time.strftime("%Y-%m-%d %H:%M:%S"), elapsed, cycles, len(commands),
                     pinger.received, pinger.sent,
                     100.0 * (pinger.sent - pinger.received) / max(1, pinger.sent),
                     pinger.max_gap), flush=True)
    report = probe(args)
    print(report)
    m = re.search(r"CFSR=0x([0-9a-f]+) HFSR=0x([0-9a-f]+)", report)
    if not m:
        failures.append("relevé sonde : registres de faute non lus")
    elif int(m.group(1), 16) or int(m.group(2), 16):
        failures.append("registres de faute non nuls : CFSR=0x%s HFSR=0x%s" % m.groups())
    for f in failures:
        print("ÉCHEC : " + f)
    print("journal : " + args.log)
    return 1 if failures else 0


if __name__ == "__main__":
    sys.exit(main())
