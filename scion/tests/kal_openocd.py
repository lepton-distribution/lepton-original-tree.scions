#!/usr/bin/env python3
"""kal_openocd.py — banc KAL sur carte réelle, semihosting par OpenOCD (étape 5).

Pendant QEMU de -semihosting-config : OpenOCD sert la ligne de commande (SYS_GET_CMDLINE), la
sortie (SYS_WRITE0) et se termine avec le code de sortie de l'application (SYS_EXIT_EXTENDED).
  --program ELF : écrit kal_bench en flash (vérifié), sans le lancer ;
  --test T      : reset, semihosting actif, lance « kal_bench T » ; code de retour = celui du test.
Sans débogueur, l'instruction BKPT du semihosting met le cœur en faute : kal_bench ne tourne
que sous ce lanceur. Code de retour non nul si OpenOCD ne rend pas la main (--timeout).

Exemple :
  kal_openocd.py --openocd openocd --cfg debug/openocd-nucleo-f439zi.cfg --test=T1
"""
import argparse
import subprocess
import sys


def main():
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--openocd", default="openocd")
    ap.add_argument("--cfg", required=True, help="configuration OpenOCD de la carte")
    ap.add_argument("--program", help="ELF à écrire en flash")
    ap.add_argument("--test", help="argument de kal_bench (T1…T8, T1F…, IRQ, HARNESS_FAIL)")
    ap.add_argument("--timeout", type=float, default=45.0)
    args = ap.parse_args()
    if bool(args.program) == bool(args.test):
        ap.error("--program ou --test, exclusivement")

    cmd = [args.openocd, "-f", args.cfg,
           "-c", "gdb_port disabled", "-c", "tcl_port disabled", "-c", "telnet_port disabled"]
    if args.program:
        cmd += ["-c", "program %s verify exit" % args.program]
    else:
        cmd += ["-c", "init", "-c", "reset halt", "-c", "arm semihosting enable",
                "-c", "arm semihosting_cmdline kal_bench %s" % args.test, "-c", "resume"]
    try:
        proc = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                              timeout=args.timeout)
    except subprocess.TimeoutExpired as e:
        sys.stdout.write((e.output or b"").decode(errors="replace"))
        print("ÉCHEC : OpenOCD n'a pas rendu la main en %.0f s (pas de SYS_EXIT)" % args.timeout)
        return 1
    out = proc.stdout.decode(errors="replace")
    # la sortie du banc (lignes « kal_bench : … ») suit les messages d'OpenOCD
    sys.stdout.write("\n".join(l for l in out.splitlines()
                               if not l.startswith(("Info :", "Unable to match"))) + "\n")
    return proc.returncode


if __name__ == "__main__":
    sys.exit(main())
