#!/usr/bin/env python3
"""map_origine.py — octets text/data/bss de chaque lepton.map, par origine (étape 9).

Usage : tools/migration/map_origine.py $LEPTON_BUILD/ci/artefacts
Origines : lepton (bibliothèques du dépôt, FreeRTOS compris), embos, newlib, libgcc/crt.
Somme des sections d'entrée : un peu au-dessus de arm-none-eabi-size (alignements) ;
mesure relative, pour comparer deux hôtes (Debian, macOS) sur le même commit.
"""
import re, sys, pathlib
SEC = re.compile(r'^ (\.[\w.$]+)?\s*(0x[0-9a-f]+)\s+(0x[0-9a-f]+)\s+(\S+)$')
def origin(path):
    n = pathlib.Path(path.split('(')[0]).name
    if n.startswith(('libc_nano', 'libg_nano', 'libc.a', 'libm', 'libnosys')): return 'newlib'
    if n.startswith(('libgcc', 'crt')): return 'libgcc/crt'
    if 'embos' in path.lower() or n.startswith('libos'): return 'embos'
    return 'lepton'
def split(mapfile):
    res = {}; pend = None; out = None
    for line in open(mapfile, encoding='latin-1'):
        if line.startswith('Linker script and memory map'): out = True; continue
        if not out: continue
        m = SEC.match(line.rstrip('\n'))
        name = None
        if m and m.group(1): name = m.group(1)
        elif m and pend: name = pend
        if line.startswith(' .') and not m and len(line.split()) == 1: pend = line.split()[0]; continue
        pend = None
        if not (m and name): continue
        addr, size, src = int(m.group(2), 16), int(m.group(3), 16), m.group(4)
        if addr == 0 or size == 0: continue
        kind = 'text' if name.startswith(('.text', '.rodata', '.ARM')) else 'data' if name.startswith('.data') else 'bss' if name.startswith(('.bss', 'COMMON')) else None
        if kind is None: continue
        o = origin(src); res.setdefault(o, {'text': 0, 'data': 0, 'bss': 0})[kind] += size
    return res
for d in sorted(pathlib.Path(sys.argv[1]).iterdir()):
    mp = d / 'lepton.map'
    if not mp.exists(): continue
    r = split(mp)
    print(d.name, ' '.join(f"{k}:{v['text']}/{v['data']}/{v['bss']}" for k, v in sorted(r.items())))
