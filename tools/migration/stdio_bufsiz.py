#!/usr/bin/env python3
"""stdio_bufsiz.py — module lib (étape 4) : BUFSIZ par défaut déplacé de stdio.h vers kal/arch/<isa>/kal_arch_conf.h.
Retire de stdio.h les dernières directives d'ISA (CPU_GNU32, CPU_CORTEXM) et la branche morte
__AS386_16__ (ELKS/bcc 16 bits). Une valeur de mkconf (__KERNEL_STDIO_PRINTF_BUFSIZ) reste prioritaire
(kernel_mkconf.h est inclus avant kal_arch_conf.h). Valeurs entre parenthèses, comme dans stdio.h.
Usage : stdio_bufsiz.py <clone>/scion/sys/root/src"""
import re, sys
from pathlib import Path
root = Path(sys.argv[1])  # scion/sys/root/src
stdio = root / "lib/libc/stdio/stdio.h"
raw = stdio.read_bytes().decode("latin-1")
nl = "\r\n" if "\r\n" in raw else "\n"
pat = re.compile(r"#ifndef __KERNEL_STDIO_PRINTF_BUFSIZ\r?\n.*?#else\r?\n(\s*#define BUFSIZ __KERNEL_STDIO_PRINTF_BUFSIZ\r?\n)#endif\r?\n", re.S)
m = pat.search(raw)
assert m and "CPU_GNU32" in m.group(0) and "CPU_CORTEXM" in m.group(0), "bloc BUFSIZ introuvable"
new = ("//taille : __KERNEL_STDIO_PRINTF_BUFSIZ (mkconf de la carte, sinon kal/arch/<isa>/kal_arch_conf.h)" + nl
       + "#define BUFSIZ __KERNEL_STDIO_PRINTF_BUFSIZ" + nl)
stdio.write_bytes((raw[:m.start()] + new + raw[m.end():]).encode("latin-1"))
vals = {"host": 256, "armv7m": 128, "armv6m": 128}
for isa, v in vals.items():
    f = root / f"kernel/core/kal/arch/{isa}/kal_arch_conf.h"
    t = f.read_text(encoding="utf-8")
    assert "__KERNEL_STDIO_PRINTF_BUFSIZ" not in t
    anchor = re.search(r"#define __KERNEL_MAX_SUPER_BLOCK .*\n", t)
    ins = ("#ifndef __KERNEL_STDIO_PRINTF_BUFSIZ //BUFSIZ de stdio (lib/libc/stdio/stdio.h), sauf mkconf\n"
           f"   #define __KERNEL_STDIO_PRINTF_BUFSIZ ({v})\n#endif\n")
    t = t[:anchor.end()] + ins + t[anchor.end():]
    f.write_text(t, encoding="utf-8")
print("stdio_bufsiz.py : stdio.h et", ", ".join(vals), "modifiés")
