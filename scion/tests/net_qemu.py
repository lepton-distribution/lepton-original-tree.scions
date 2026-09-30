#!/usr/bin/env python3
"""net_qemu.py — palier réseau du socle QEMU (étape 3b) : ping et session ftpd depuis l'hôte.

Sans privilège (décision 2026-09-30) : le test se relance dans un espace de noms utilisateur et
réseau (unshare --user --map-root-user --net), y crée une interface tap (adresse hôte), démarre
QEMU sur ce tap (LAN9118 de la carte), attend l'invite de lsh (le .init de la carte configure
eth0 et lance ftpd), puis, depuis la pile IP de l'hôte :
  - ping de l'invité (--ping-count réponses exigées) ;
  - session FTP (ftplib, mode passif) : connexion, CWD et LIST d'un répertoire (--ftp-list,
    entrée --ftp-list-expect exigée), téléchargement d'un
    fichier du rootfs comparé octet à octet à sa source (--ftp-file, --ftp-reference).
Échoue explicitement si les espaces de noms utilisateur sont interdits (jamais ignoré).
Code de retour 0 si tout est conforme (CTest, label net).

Exemple :
  net_qemu.py --qemu qemu-system-arm --machine mps2-an386 --kernel lepton.elf \
              --ftp-file /usr/etc/.boot --ftp-reference sys/user/tauon-basic/etc/qemu-mps2-an386/.boot
"""
import argparse
import ftplib
import io
import os
import re
import subprocess
import sys
import tempfile
import time

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from smoke_lsh import Console, PROMPT, run_command  # noqa: E402

TAP = "tap0"


def in_netns(args):
    """Relance ce script dans un espace de noms utilisateur et réseau."""
    unshare = ["unshare", "--user", "--map-root-user", "--net", "--"]
    probe = subprocess.run(unshare + ["true"], capture_output=True, text=True)
    if probe.returncode != 0:
        print("ÉCHEC : espace de noms utilisateur et réseau indisponible : %s"
              % probe.stderr.strip())
        return 1
    return subprocess.run(unshare + [sys.executable, os.path.abspath(__file__), "--in-netns"]
                          + sys.argv[1:]).returncode


def setup_tap(host_ip, prefix):
    for c in (["ip", "link", "set", "lo", "up"],
              ["ip", "tuntap", "add", "dev", TAP, "mode", "tap"],
              ["ip", "addr", "add", "%s/%d" % (host_ip, prefix), "dev", TAP],
              ["ip", "link", "set", TAP, "up"]):
        r = subprocess.run(c, capture_output=True, text=True)
        if r.returncode != 0:
            return "%s : %s" % (" ".join(c), r.stderr.strip())
    return None


def ping(guest_ip, count, deadline_s):
    """Attend que l'invité réponde (lwIP et ARP prêts), puis exige count réponses."""
    end = time.monotonic() + deadline_s
    while time.monotonic() < end:
        r = subprocess.run(["ping", "-c", "1", "-W", "1", guest_ip], capture_output=True, text=True)
        if r.returncode == 0:
            break
    r = subprocess.run(["ping", "-c", str(count), "-W", "2", guest_ip],
                       capture_output=True, text=True)
    m = re.search(r"(\d+) received", r.stdout)
    received = int(m.group(1)) if m else 0
    return received, r.stdout.strip().splitlines()[-2:] if r.stdout else [r.stderr.strip()]


def ftp_session(args):
    with open(args.ftp_reference, "rb") as f:
        reference = f.read()
    ftp = ftplib.FTP()
    ftp.connect(args.guest_ip, 21, timeout=args.timeout)
    welcome = ftp.getwelcome()
    ftp.login(args.ftp_user, args.ftp_password)
    listing = []
    # ftpd de Lepton : LIST sans argument seulement (« LIST with arguments unimplemented »)
    ftp.cwd(args.ftp_list)
    ftp.retrlines("LIST", listing.append)
    data = io.BytesIO()
    ftp.retrbinary("RETR " + args.ftp_file, data.write)
    try:
        ftp.quit()
    except ftplib.all_errors:
        ftp.close()
    return welcome, listing, data.getvalue(), reference


def main():
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--qemu", default="qemu-system-arm")
    ap.add_argument("--machine", required=True)
    ap.add_argument("--kernel", required=True)
    ap.add_argument("--host-ip", default="192.168.100.1")
    ap.add_argument("--guest-ip", default="192.168.100.2")
    ap.add_argument("--prefix", type=int, default=24)
    ap.add_argument("--ping-count", type=int, default=3)
    ap.add_argument("--ftp-user", default="tauon")
    ap.add_argument("--ftp-password", default="tauon")
    ap.add_argument("--ftp-list", default="/usr/sbin", help="répertoire listé (CWD puis LIST)")
    ap.add_argument("--ftp-list-expect", default="lsh", help="entrée attendue dans la liste")
    ap.add_argument("--ftp-file", required=True, help="fichier de l'invité à télécharger")
    ap.add_argument("--ftp-reference", required=True, help="source du fichier (comparaison)")
    ap.add_argument("--boot-timeout", type=float, default=30.0)
    ap.add_argument("--timeout", type=float, default=10.0)
    ap.add_argument("--log", help="journal de la console (défaut : fichier temporaire)")
    ap.add_argument("--in-netns", action="store_true", help=argparse.SUPPRESS)
    args = ap.parse_args()

    if not args.in_netns:
        return in_netns(args)

    failures = []
    err = setup_tap(args.host_ip, args.prefix)
    if err:
        print("ÉCHEC : tap : " + err)
        return 1

    workdir = tempfile.mkdtemp(prefix="net_qemu_")
    log_path = args.log or os.path.join(workdir, "uart0.log")
    cmd = [args.qemu, "-M", args.machine, "-kernel", args.kernel, "-nographic",
           "-monitor", "none", "-serial", "stdio",
           "-nic", "tap,ifname=%s,script=no,downscript=no" % TAP]
    summary = ""
    with open(log_path, "wb") as log:
        proc = subprocess.Popen(cmd, stdin=subprocess.PIPE, stdout=subprocess.PIPE,
                                stderr=subprocess.STDOUT)
        con = Console(proc, log)
        try:
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
                    welcome, listing, data, reference = ftp_session(args)
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
                except (ftplib.all_errors, OSError) as e:
                    failures.append("FTP : %s" % e)
            if not failures:
                out = run_command(con, "ps", args.timeout)
                if out is None or b"ftpd" not in out:
                    failures.append("ps : ftpd absent après la session")
        finally:
            proc.kill()
            proc.wait()
    if failures:
        for f in failures:
            print("ÉCHEC : " + f)
        print("journal : " + log_path)
        return 1
    print("net_qemu : %s ; journal %s" % (summary, log_path))
    return 0


if __name__ == "__main__":
    sys.exit(main())
