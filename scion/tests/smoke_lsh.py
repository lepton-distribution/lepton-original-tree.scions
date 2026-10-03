#!/usr/bin/env python3
"""smoke_lsh.py — test de fumée canonique Lepton : démarrage → lsh → uname -a (étape 3).

Transport QEMU (défaut) : lance QEMU (UART0 sur stdin/stdout du processus, UART1 dans un
fichier). Transport série (étape 5, carte réelle) : ouvre le port de la console (termios, 8N1,
sans contrôle de flux), puis lance --reset-command (reset par la sonde) pour observer le démarrage.
Attend l'invite de lsh, exécute « uname -a » et vérifie la machine attendue ; puis, en option,
des commandes supplémentaires (--command, sortie sans message d'erreur) et l'écriture sur le
second port (--uart1, QEMU : « echo <jeton> > /dev/ttys1 » doit apparaître dans le fichier de
UART1). Code de retour 0 si tout est conforme (CTest, labels smoke et board).

Exemples :
  smoke_lsh.py --qemu qemu-system-arm --machine mps2-an386 --kernel lepton.elf \
               --expect-machine cortexM4-qemu-mps2-an386 --command ls --command ps --uart1
  smoke_lsh.py --transport serial --port /dev/ttyACM0 --expect-machine cortexM4-stm32f4 \
               --reset-command "openocd -f debug/openocd-nucleo-f439zi.cfg -c init -c reset -c exit"
"""
import argparse
import os
import re
import select
import shlex
import shutil
import subprocess
import sys
import tempfile
import time

PROMPT = re.compile(rb"lepton#\d+\$ ")
ERRORS = re.compile(rb"(?i)(not found|error|cannot|unknown command)")


class Console:
    def __init__(self, proc, log):
        self.proc = proc
        self.log = log
        self.buf = b""

    def read_until(self, pattern, timeout):
        deadline = time.monotonic() + timeout
        while True:
            m = pattern.search(self.buf)
            if m:
                out = self.buf[:m.end()]
                self.buf = self.buf[m.end():]
                return out
            left = deadline - time.monotonic()
            if left <= 0 or self.proc.poll() is not None:
                return None
            r, _, _ = select.select([self.proc.stdout], [], [], min(left, 0.2))
            if r:
                data = os.read(self.proc.stdout.fileno(), 4096)
                if data:
                    self.log.write(data)
                    self.log.flush()
                    self.buf += data

    def drain(self, quiet=0.5, limit=5.0):
        """Lit jusqu'à ce que la console se taise (invites en attente, échos) puis vide le tampon."""
        deadline = time.monotonic() + limit
        last = time.monotonic()
        while time.monotonic() < deadline and time.monotonic() - last < quiet:
            r, _, _ = select.select([self.proc.stdout], [], [], 0.1)
            if r:
                data = os.read(self.proc.stdout.fileno(), 4096)
                if data:
                    self.log.write(data)
                    self.log.flush()
                    last = time.monotonic()
        self.buf = b""

    def send(self, line):
        # lsh lit caractère par caractère : envoi lent, fin de ligne CR
        for c in line.encode() + b"\r":
            self.proc.stdin.write(bytes([c]))
            self.proc.stdin.flush()
            time.sleep(0.01)


class SerialLink:
    """Port série présenté comme le processus QEMU attendu par Console (stdin, stdout, poll)."""

    def __init__(self, port, baud):
        import termios
        import tty
        fd = os.open(port, os.O_RDWR | os.O_NOCTTY)
        tty.setraw(fd)
        attrs = termios.tcgetattr(fd)
        attrs[4] = attrs[5] = getattr(termios, "B%d" % baud)
        attrs[2] |= termios.CLOCAL | termios.CREAD
        attrs[2] &= ~getattr(termios, "CRTSCTS", 0)
        termios.tcsetattr(fd, termios.TCSANOW, attrs)
        termios.tcflush(fd, termios.TCIOFLUSH)
        self.stdout = os.fdopen(fd, "rb", buffering=0)
        self.stdin = os.fdopen(os.dup(fd), "wb", buffering=0)

    def poll(self):
        return None

    def kill(self):
        self.stdin.close()
        self.stdout.close()

    def wait(self):
        return 0


def run_command(con, cmd, timeout):
    con.drain()
    con.send(cmd)
    out = con.read_until(PROMPT, timeout)
    if out is None:
        return None
    # retirer l'écho de la commande et l'invite finale
    text = out.split(cmd.encode(), 1)[-1]
    return PROMPT.sub(b"", text)


def main():
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--transport", choices=("qemu", "serial"), default="qemu")
    ap.add_argument("--qemu", default="qemu-system-arm")
    ap.add_argument("--machine", help="machine QEMU (transport qemu)")
    ap.add_argument("--kernel", help="ELF à lancer (transport qemu)")
    ap.add_argument("--port", help="port série de la console (transport serial)")
    ap.add_argument("--baud", type=int, default=115200)
    ap.add_argument("--reset-command",
                    help="commande de reset de la carte, lancée après l'ouverture du port (serial)")
    ap.add_argument("--expect-machine", required=True)
    ap.add_argument("--command", action="append", default=[])
    ap.add_argument("--uart1", action="store_true", help="vérifier le second port (ttys1)")
    ap.add_argument("--boot-timeout", type=float, default=30.0)
    ap.add_argument("--timeout", type=float, default=10.0)
    ap.add_argument("--log", help="journal de la console (défaut : fichier temporaire)")
    args = ap.parse_args()
    if args.transport == "qemu" and not (args.machine and args.kernel):
        ap.error("transport qemu : --machine et --kernel requis")
    if args.transport == "serial" and (not args.port or args.uart1):
        ap.error("transport serial : --port requis, --uart1 non disponible")

    workdir = tempfile.mkdtemp(prefix="smoke_lsh_")
    uart1 = os.path.join(workdir, "uart1.log")
    log_path = args.log or os.path.join(workdir, "uart0.log")
    cmd = [args.qemu, "-M", args.machine, "-kernel", args.kernel, "-nographic",
           "-monitor", "none", "-serial", "stdio", "-serial", "file:" + uart1]
    failures = []
    with open(log_path, "wb") as log:
        if args.transport == "qemu":
            proc = subprocess.Popen(cmd, stdin=subprocess.PIPE, stdout=subprocess.PIPE,
                                    stderr=subprocess.STDOUT)
        else:
            proc = SerialLink(args.port, args.baud)
        con = Console(proc, log)
        if args.transport == "serial" and args.reset_command:
            # la sonde ST-LINK livre après l'ouverture du port ce qu'elle a tamponné avant
            # (invites d'une session précédente) : l'écarter avant le reset, sinon il est pris
            # pour l'invite de démarrage et « uname -a » part pendant l'attente d'initd
            con.drain()
            subprocess.run(shlex.split(args.reset_command), check=True,
                           stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        try:
            # l'invite n'apparaît qu'après une touche (initd : « any key to continue »)
            boot = con.read_until(re.compile(rb"any key to continue|lepton#\d+\$ "), args.boot_timeout)
            if boot is None and args.transport == "serial" and not args.reset_command:
                # carte déjà démarrée, sans reset : réveiller l'invite
                con.send("")
                boot = con.read_until(PROMPT, args.timeout)
            if boot is None:
                failures.append("démarrage : ni invite initd ni invite lsh")
            else:
                con.send("")
                if con.read_until(PROMPT, args.timeout) is None:
                    failures.append("invite lsh absente")
            if not failures:
                out = run_command(con, "uname -a", args.timeout)
                if out is None:
                    failures.append("uname -a : pas de retour à l'invite")
                elif args.expect_machine.encode() not in out:
                    failures.append("uname -a : machine « %s » absente de %r"
                                    % (args.expect_machine, out.strip()))
                for c in args.command:
                    out = run_command(con, c, args.timeout)
                    if out is None:
                        failures.append("%s : pas de retour à l'invite" % c)
                    elif ERRORS.search(out):
                        failures.append("%s : %r" % (c, out.strip()))
                if args.uart1:
                    token = "lepton-uart1-%d" % os.getpid()
                    if run_command(con, "echo %s > /dev/ttys1" % token, args.timeout) is None:
                        failures.append("echo > /dev/ttys1 : pas de retour à l'invite")
                    else:
                        time.sleep(0.5)
                        with open(uart1, "rb") as f:
                            if token.encode() not in f.read():
                                failures.append("UART1 : jeton absent de %s" % uart1)
        finally:
            proc.kill()
            proc.wait()
    if failures:
        for f in failures:
            print("ÉCHEC : " + f)
        print("journal : " + log_path)
        return 1
    # succès : répertoire temporaire supprimé (conservé en cas d'échec, pour le journal)
    shutil.rmtree(workdir, ignore_errors=True)
    print("smoke_lsh : %s, uname -a = %s, %d commande(s)%s ; journal %s"
          % (args.machine if args.transport == "qemu" else args.port, args.expect_machine, len(args.command),
             ", UART1 vérifiée" if args.uart1 else "", args.log or "temporaire supprimé"))
    return 0


if __name__ == "__main__":
    sys.exit(main())
