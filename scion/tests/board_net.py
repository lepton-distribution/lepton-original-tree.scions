#!/usr/bin/env python3
"""board_net.py — palier 7 de l'étape 5 : réseau de la carte réelle (ping, ftpd, errno).

Pendant de net_qemu.py (étape 3b) pour une carte reliée à l'hôte par un câble Ethernet : pas de
tap ni d'espace de noms, l'hôte joint la carte par son interface (--host-ip, adresse de l'hôte
sur ce lien ; --guest-ip, adresse configurée par le .init de la carte). Ouvre la console série,
écarte ce que la sonde a tamponné, lance --reset-command (flash et reset par la sonde), attend
l'invite de lsh (le .init configure eth0 et lance ftpd), puis, depuis l'hôte :
  - ping de la carte (--ping-count réponses exigées) ;
  - session FTP (connexion, CWD et LIST de --ftp-list, téléchargement de --ftp-file comparé
    octet à octet à --ftp-reference) ;
  - errno d'un connect() refusé vu par net/tsterrno (port fermé de l'hôte) : numérotation Lepton ;
  - ftpd toujours présent dans ps.
Fonctions de ping, FTP et errno partagées avec net_qemu.py. Code de retour 0 si tout est
conforme (CTest, labels board et net).

Exemple :
  board_net.py --port /dev/ttyACM0 --host-ip 192.168.2.20 --guest-ip 192.168.2.5 \\
      --reset-command "openocd -f debug/openocd-nucleo-f439zi.cfg -c 'program lepton.elf verify reset exit'" \\
      --ftp-file /usr/etc/.boot --ftp-reference sys/user/tauon-basic/etc/nucleo-f439zi/.boot \\
      --errno-header sys/root/src/kernel/core/errno.h
"""
import argparse
import ftplib
import os
import re
import shlex
import shutil
import subprocess
import sys
import tempfile

# tests/ est lu par le trunk (vue de liens) : aucun __pycache__ à y écrire
sys.dont_write_bytecode = True
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from smoke_lsh import Console, SerialLink, PROMPT, run_command  # noqa: E402
from net_qemu import ping, ftp_session, errno_check  # noqa: E402


def main():
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--port", required=True, help="port série de la console de la carte")
    ap.add_argument("--baud", type=int, default=115200)
    ap.add_argument("--reset-command", required=True,
                    help="flash et reset par la sonde, lancé après l'ouverture du port")
    ap.add_argument("--host-ip", required=True, help="adresse de l'hôte sur le lien de la carte")
    ap.add_argument("--guest-ip", required=True, help="adresse de la carte (.init)")
    ap.add_argument("--ping-count", type=int, default=3)
    ap.add_argument("--ftp-user", default="tauon")
    ap.add_argument("--ftp-password", default="tauon")
    ap.add_argument("--ftp-list", default="/usr/sbin", help="répertoire listé (CWD puis LIST)")
    ap.add_argument("--ftp-list-expect", default="lsh", help="entrée attendue dans la liste")
    ap.add_argument("--ftp-file", required=True, help="fichier de la carte à télécharger")
    ap.add_argument("--ftp-reference", required=True, help="source du fichier (comparaison)")
    ap.add_argument("--errno-header", required=True, help="kernel/core/errno.h (valeurs Lepton)")
    ap.add_argument("--refused-port", type=int, default=9, help="port TCP fermé de l'hôte")
    ap.add_argument("--boot-timeout", type=float, default=30.0)
    ap.add_argument("--timeout", type=float, default=10.0)
    ap.add_argument("--log", help="journal de la console (défaut : fichier temporaire)")
    ap.add_argument("--ftp-debug", type=int, default=1,
                    help="trace du protocole FTP (ftplib, mot de passe masqué) ; 0 : aucune")
    args = ap.parse_args()

    workdir = None if args.log else tempfile.mkdtemp(prefix="board_net_")
    log_path = args.log or os.path.join(workdir, "console.log")
    failures = []
    summary = ""
    with open(log_path, "wb") as log:
        link = SerialLink(args.port, args.baud)
        con = Console(link, log)
        try:
            # invites tamponnées par la sonde avant l'ouverture du port : écartées (smoke_lsh)
            con.drain()
            subprocess.run(shlex.split(args.reset_command), check=True,
                           stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
            boot = con.read_until(re.compile(rb"any key to continue|lepton#\d+\$ "), args.boot_timeout)
            if boot is None:
                failures.append("démarrage : ni invite initd ni invite lsh")
            else:
                con.send("")
                if con.read_until(PROMPT, args.timeout) is None:
                    failures.append("invite lsh absente")
            if not failures:
                received, tail = ping(args.guest_ip, args.ping_count, args.boot_timeout)
                if received != args.ping_count:
                    failures.append("ping %s : %d/%d réponses (%s)"
                                    % (args.guest_ip, received, args.ping_count, " | ".join(tail)))
            if not failures:
                try:
                    welcome, listing, data, reference = ftp_session(args, args.ftp_debug)
                    if not any(l.split()[-1:] == [args.ftp_list_expect] for l in listing):
                        failures.append("FTP LIST %s : « %s » absent de %r"
                                        % (args.ftp_list, args.ftp_list_expect, listing))
                    if data != reference:
                        failures.append("FTP RETR %s : %d octets, différent de %s (%d octets)"
                                        % (args.ftp_file, len(data), args.ftp_reference,
                                           len(reference)))
                    summary = ("ping %d/%d, FTP « %s », LIST %d entrée(s), RETR %s %d octets identique"
                               % (received, args.ping_count, welcome.strip(), len(listing),
                                  args.ftp_file, len(data)))
                except ftplib.all_errors as e:  # tuple incluant OSError
                    failures.append("FTP : %s" % e)
            if not failures:
                echec, resume = errno_check(con, args)
                if echec:
                    failures.append(echec)
                else:
                    summary += ", " + resume
            if not failures:
                out = run_command(con, "ps", args.timeout)
                if out is None or b"ftpd" not in out:
                    failures.append("ps : ftpd absent après la session")
        finally:
            link.kill()
    if failures:
        for f in failures:
            print("ÉCHEC : " + f)
        print("journal : " + log_path)
        return 1
    # succès : répertoire temporaire supprimé (conservé en cas d'échec, pour le journal)
    if workdir:
        shutil.rmtree(workdir, ignore_errors=True)
    print("board_net : %s ; journal %s" % (summary, args.log or "temporaire supprimé"))
    return 0


if __name__ == "__main__":
    sys.exit(main())
