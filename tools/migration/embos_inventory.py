#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
embos_inventory.py — inventaire embOS pour la migration Lepton (étape 1, tâches 0 et 4).

Rejouable, lecture seule sur le trunk et sur le paquet embOS (licence SFL : aucun code
Segger n'est recopié, seuls des noms de symboles, de types et de champs sont extraits).

Produit :
  --api      : API publique du paquet embOS cible (RTOS.h) — noms uniquement
               (doc/migration/embos-api.txt) ;
  --work     : fichiers intermédiaires (CSV) :
                 usages.csv          identifiant OS_*, fichier, ligne, zone
                 usage-summary.csv   identifiant, occurrences, fichiers, statut cible, statut réf.
                 tcb-fields.csv      accès aux champs de TCB / de trame de pile embOS
                 library-variants.csv décodage des noms de bibliothèques libosT*.a

Usage :
  python3 tools/migration/embos_inventory.py \
      --embos  "$LEPTON_EMBOS_ROOT" \
      --ref    "$LEPTON_TRUNK/sys/root/src/kernel/core/ucore/embOSCXM4_518/inc/RTOS.h" \
      --trunk  "$LEPTON_TRUNK" \
      --api    doc/migration/embos-api.txt \
      --work   "$LEPTON_BUILD/etape-1/kal"
"""
import argparse
import csv
import os
import re
import subprocess
import sys
from collections import defaultdict

ID_RE = re.compile(r'\bOS_[A-Za-z][A-Za-z0-9_]*')

# ---------------------------------------------------------------- en-têtes embOS


def strip_comments(text):
    """Retire les commentaires C/C++ en conservant les sauts de ligne."""
    out = []
    i, n = 0, len(text)
    in_str = None
    while i < n:
        c = text[i]
        if in_str:
            out.append(c)
            if c == '\\' and i + 1 < n:
                out.append(text[i + 1])
                i += 2
                continue
            if c == in_str:
                in_str = None
            i += 1
            continue
        if c in '"\'':
            in_str = c
            out.append(c)
            i += 1
            continue
        if text.startswith('//', i):
            j = text.find('\n', i)
            i = n if j < 0 else j
            continue
        if text.startswith('/*', i):
            j = text.find('*/', i + 2)
            seg = text[i:(n if j < 0 else j + 2)]
            out.append('\n' * seg.count('\n'))
            i = n if j < 0 else j + 2
            continue
        out.append(c)
        i += 1
    return ''.join(out)


def read_text(path):
    with open(path, 'rb') as f:
        return f.read().decode('latin-1').replace('\r\n', '\n').replace('\r', '\n')


def parse_header(path):
    """Extrait fonctions, macros, types, constantes d'énumération et champs de structures."""
    raw = read_text(path)
    ver = None
    m = re.search(r'OS version:\s*V?([0-9][0-9.a-z]*)', raw)
    if m:
        ver = m.group(1)
    m2 = re.search(r'#\s*define\s+OS_VERSION_GENERIC\s+\(?\s*([0-9]+)u?', raw)
    gen = m2.group(1) if m2 else None
    text = strip_comments(raw)
    # Joindre les continuations de ligne.
    text = re.sub(r'\\\n', ' ', text)
    lines = text.split('\n')
    macros = {}
    body = []
    for ln in lines:
        m = re.match(r'\s*#\s*define\s+(OS_\w+)(\()?\s*(.*)$', ln)
        if m:
            name, fn, repl = m.group(1), bool(m.group(2)), m.group(3).strip()
            if fn:
                repl = repl.split(')', 1)[1].strip() if ')' in repl else ''
            macros.setdefault(name, (fn, repl))
            body.append('')
            continue
        if re.match(r'\s*#', ln):
            body.append('')
            continue
        body.append(ln)
    code = '\n'.join(body)
    funcs, types, enums = set(), set(), set()
    # Prototypes : nom précédé d'un type (identifiant ou '*').
    for m in re.finditer(r'(?:[A-Za-z0-9_]\s+|\*\s*)(OS_\w+)\s*\(', code):
        # Exclure typedef de type fonction : traité plus bas.
        start = code.rfind('\n', 0, m.start()) + 1
        stmt_start = max(code.rfind(';', 0, m.start()), code.rfind('}', 0, m.start()),
                         code.rfind('{', 0, m.start()))
        stmt = code[stmt_start + 1:m.end()]
        if re.search(r'\btypedef\b', stmt):
            types.add(m.group(1))
            continue
        if re.search(r'\breturn\b', code[start:m.start()]):
            continue
        funcs.add(m.group(1))
    for m in re.finditer(r'typedef\s+[^;{}]*?\(\s*\*?\s*(OS_\w+)\s*\)\s*\(', code):
        types.add(m.group(1))
    for m in re.finditer(r'typedef\s+[^;{}()]*?\b(OS_\w+)\s*(?:\[[^\]]*\])?\s*;', code):
        types.add(m.group(1))
    for m in re.finditer(r'\}\s*(OS_\w+)\s*;', code):
        types.add(m.group(1))
    for m in re.finditer(r'typedef\s+(?:struct|union|enum)\s+(OS_\w+)\s+(OS_\w+)\s*;', code):
        types.add(m.group(2))
    for m in re.finditer(r'(?:struct|union|enum)\s+(OS_\w+)', code):
        types.add(m.group(1))
    for m in re.finditer(r'enum\s*(?:OS_\w+)?\s*\{([^}]*)\}', code):
        for e in re.finditer(r'\b(OS_\w+)\s*(?:=|,|$)', m.group(1)):
            enums.add(e.group(1))
    # Fonctions internes appelées dans des macros ou inline : on les laisse dans "funcs"
    # uniquement si elles sont déclarées.
    structs = {}
    for m in re.finditer(r'struct\s+(OS_\w+)\s*\{', code):
        structs[m.group(1)] = struct_fields(code, m.end())
    for m in re.finditer(r'typedef\s+(?:struct|union)\s*\{', code):
        fields, end = struct_fields(code, m.end(), return_end=True)
        mm = re.match(r'\s*(OS_\w+)\s*;', code[end:])
        if mm:
            structs[mm.group(1)] = fields
    funcs -= types
    return dict(path=path, version=ver, generic=gen, macros=macros, funcs=funcs,
                types=types, enums=enums, structs=structs, code=code)


def resolve(name, hdr, depth=0):
    """Suit les alias de macros (OS_CreateTimer -> OS_TIMER_Create -> OS_TIMER_Create_DP…)."""
    if hdr is None or depth > 5:
        return name
    if name in hdr['macros']:
        fn, repl = hdr['macros'][name]
        m = re.match(r'\(?\s*(OS_\w+)', repl or '')
        if m and m.group(1) != name and (not fn or re.match(r'\(?\s*OS_\w+\s*\(', repl)):
            return resolve(m.group(1), hdr, depth + 1)
    return name


def prototype(name, hdr):
    if hdr is None or name not in hdr['funcs']:
        return None
    m = re.search(r'(?:^|[;}{])\s*([^;{}#]*?\b' + re.escape(name) + r'\s*\([^;{]*\))\s*;',
                  hdr['code'])
    if not m:
        return None
    p = re.sub(r'\s+', ' ', m.group(1)).strip()
    p = re.sub(r'\b(OS_PRIO|OS_TASK_PRIO)\b', 'PRIO', p)  # renommage de type sans effet ABI
    p = re.sub(r'\s*OS_TEXT_SECTION_ATTRIBUTE\([^)]*\)', '', p)
    # Ne comparer que les types : retirer le nom de chaque paramètre.
    m = re.match(r'(.*?\()(.*)\)$', p)
    if m:
        params = []
        for prm in m.group(2).split(','):
            prm = prm.strip()
            toks = re.findall(r'\w+|\*', prm)
            if len(toks) > 1 and re.match(r'\w+$', toks[-1]) and prm != 'void':
                prm = re.sub(r'\s*\b' + toks[-1] + r'\s*$', '', prm)
            params.append(re.sub(r'\s+', ' ', prm).replace(' *', '*'))
        p = m.group(1) + ', '.join(params) + ')'
    return p


def struct_fields(code, pos, return_end=False):
    depth, i = 1, pos
    while i < len(code) and depth:
        if code[i] == '{':
            depth += 1
        elif code[i] == '}':
            depth -= 1
        i += 1
    body = code[pos:i - 1]
    fields = []
    for stmt in body.split(';'):
        stmt = stmt.strip()
        if not stmt:
            continue
        m = re.search(r'(\w+)\s*(?:\[[^\]]*\])?\s*$', stmt)
        if m:
            fields.append(m.group(1))
    return (fields, i) if return_end else fields


def lib_exports(libdir, lib):
    path = os.path.join(libdir, lib)
    try:
        out = subprocess.run(['nm', '-g', path], capture_output=True, text=True,
                             check=False).stdout
    except FileNotFoundError:
        return set(), set()
    defined, undef = set(), set()
    for ln in out.splitlines():
        p = ln.split()
        if len(p) == 3 and p[1] in 'TDBRCW':
            defined.add(p[2])
        elif len(p) == 2 and p[0] == 'U':
            undef.add(p[1])
    return defined, undef - defined


LIB_RE = re.compile(r'^libosT(6|7|8BL|8ML|81ML)(VH|V)?(B|L)(XR|R|SP|S|DP|DT|D)'
                    r'(_837070)?(_TZ)?(_PACBTI)?\.a$')
ARCH = {'6': 'ARMv6-M (Cortex-M0/M0+/M1)', '7': 'ARMv7-M/ARMv7E-M (Cortex-M3/M4/M7)',
        '8BL': 'ARMv8-M Baseline (Cortex-M23)', '8ML': 'ARMv8-M Mainline (Cortex-M33)',
        '81ML': 'ARMv8.1-M (Cortex-M55/M85)'}
VFP = {None: 'sans FPU', 'V': 'FPU, ABI softfp', 'VH': 'FPU, ABI hard'}
MODE = {'XR': 'Extreme Release', 'R': 'Release', 'S': 'Stack check',
        'SP': 'Stack check + profiling', 'D': 'Debug', 'DP': 'Debug + profiling + stack check',
        'DT': 'Debug + profiling + stack check + trace'}


def decode_libs(libdir):
    rows = []
    for lib in sorted(os.listdir(libdir)):
        m = LIB_RE.match(lib)
        if not m:
            rows.append(dict(lib=lib, arch='?', vfp='?', endian='?', mode='?', errata='',
                             tz='', pacbti=''))
            continue
        a, v, e, md, er, tz, pb = m.groups()
        rows.append(dict(lib=lib, arch=ARCH[a], vfp=VFP[v], endian='big' if e == 'B' else 'little',
                         mode=MODE[md], errata='837070' if er else '', tz='oui' if tz else '',
                         pacbti='oui' if pb else ''))
    return rows

# ---------------------------------------------------------------- usages Lepton


def zone_of(rel):
    """Zone grossière (le classement actif/différé/gelé définitif est celui de la tâche 2)."""
    r = rel.replace('\\', '/')
    frozen_markers = ('/arm7/', '/arm9/', '/at91/', '/gnu32/', '/win32/', '/m16c', 'ports/m16c',
                      'ports/win32', 'ports/gnu/', 'virtual_cpu', 'vc-2010')
    if any(k in r for k in frozen_markers):
        return 'gelé présumé'
    if '/kernel/core/core-segger/' in r:
        return 'core-segger'
    if '/kernel/core/core-freertos/' in r:
        return 'core-freertos'
    if '/kernel/core/core-generic/' in r:
        return 'core-generic'
    if r.endswith('/kernel/core/kal.h') or r.endswith('/kernel/core/kal.c'):
        return 'kal'
    if '/kernel/core/' in r:
        return 'kernel/core commun'
    if '/kernel/dev/arch/cortexm/' in r:
        return 'dev cortexm'
    if '/kernel/dev/' in r:
        return 'dev commun'
    if '/kernel/net/lwip/ports/arm/' in r:
        return 'lwip port arm'
    return 'autre'


KAL_BRANCHES = (  # (motif de la condition de premier niveau, étiquette)
    ('ECOS', 'kal/eCos (gelé présumé)'),
    ('CPU_ARM7', 'kal/eCos ARM7-9 (gelé présumé)'),
    ('USE_KERNEL_STATIC', 'kal/gnu32 statique (mklepton)'),
    ('CPU_WIN32', 'kal/win32 (gelé présumé)'),
    ('M16C', 'kal/m16c (gelé présumé)'),
    ('__KERNEL_UCORE_EMBOS', 'kal/embOS ARM IAR-Keil'),
    ('__KERNEL_UCORE_FREERTOS', 'kal/FreeRTOS'),
)


def kal_branch_map(text):
    """Étiquette de branche de premier niveau (#if/#elif sous la garde d'inclusion) par ligne."""
    labels = {}
    depth = 0
    cur = 'kal/commun'
    lines = text.split('\n')
    i = 0
    while i < len(lines):
        start_i = i
        ln = lines[i]
        cond = ln
        is_cond = re.match(r'\s*#\s*(if|ifdef|ifndef|elif)\b', ln)
        while is_cond and cond.rstrip().endswith('\\') and i + 1 < len(lines):
            i += 1
            cond = cond.rstrip()[:-1] + ' ' + lines[i]
        m = re.match(r'\s*#\s*(if|ifdef|ifndef|elif|else|endif)\b(.*)', cond)
        if m:
            kw, rest = m.groups()
            if kw in ('if', 'ifdef', 'ifndef'):
                depth += 1
                if depth == 2:
                    cur = next((lab for pat, lab in KAL_BRANCHES if pat in rest), 'kal/autre')
            elif kw == 'elif' and depth == 2:
                cur = next((lab for pat, lab in KAL_BRANCHES if pat in rest), 'kal/autre')
            elif kw == 'else' and depth == 2:
                cur = 'kal/défaut (#else)'
            elif kw == 'endif':
                if depth == 2:
                    cur = 'kal/commun'
                depth -= 1
        for k in range(start_i, i + 1):
            labels[k + 1] = cur
        i += 1
    return labels


IFDEF_RE = re.compile(r'^\s*#\s*(?:if|elif|ifdef|ifndef)\b(.*)$')
ISA_TOKENS = re.compile(r'(__tauon_cpu_core__|__tauon_cpu_device__|__tauon_cpu_core_\w+|'
                        r'CPU_(?:ARM7|ARM9|CORTEXM|M16C62|M16C|GNU32|WIN32)\b|__CORE__|'
                        r'__ARM\w*__|__ARMVFP__|__FPU_PRESENT|__FPU_USED|__ARM_ARCH\w*|__thumb2?__|'
                        r'__ARM_FP\b|__VFP_FP__|__SOFTFP__)')
ARCH_DIRS = ('/kernel/dev/arch/', '/kernel/core/arch/', '/kernel/core/ucore/')


def scan_ifdefs(trunk):
    """#if d'ISA/cœur/carte hors des répertoires d'architecture."""
    root = os.path.join(trunk, 'sys', 'root', 'src')
    rows = []
    for dp, dn, fn in os.walk(root, followlinks=True):
        for f in fn:
            if not re.search(r'\.(c|h)$', f):
                continue
            p = os.path.join(dp, f)
            rel = os.path.relpath(p, trunk)
            if any(x in '/' + rel.replace('\\', '/') for x in ARCH_DIRS):
                continue
            try:
                txt = strip_comments(read_text(p))
            except OSError:
                continue
            txt = re.sub(r'\\\n', ' ', txt)
            for i, ln in enumerate(txt.split('\n'), 1):
                m = IFDEF_RE.match(ln)
                if not m:
                    continue
                toks = sorted(set(ISA_TOKENS.findall(m.group(1))))
                if toks:
                    rows.append((rel, i, zone_of('/' + rel), ' '.join(toks)))
    return rows


def scan_lepton(trunk, exclude=('/ucore/',)):
    root = os.path.join(trunk, 'sys', 'root', 'src')
    usages = []
    fields = []
    for dp, dn, fn in os.walk(root, followlinks=True):
        rel_dp = os.path.relpath(dp, trunk).replace('\\', '/') + '/'
        if any(x in '/' + rel_dp for x in exclude):
            continue
        for f in fn:
            if not re.search(r'\.(c|h|s|S|asm)$', f):
                continue
            p = os.path.join(dp, f)
            rel = os.path.relpath(p, trunk)
            try:
                txt = strip_comments(read_text(p))
            except OSError:
                continue
            z0 = zone_of('/' + rel)
            kmap = kal_branch_map(txt) if f == 'kal.h' and z0 == 'kal' else None
            for i, ln in enumerate(txt.split('\n'), 1):
                z = kmap.get(i, z0) if kmap else z0
                for m in ID_RE.finditer(ln):
                    usages.append((m.group(0), rel, i, z))
                for m in re.finditer(r'(?:->\s*tcb\s*->|\btcb\s*->|\.os_task\.|\.tcb\.)\s*(\w+)', ln):
                    fields.append(('TCB', m.group(1), rel, i, z))
                for m in re.finditer(r'OS_REGS\w*\s+OS_STACKPTR\s*\*\s*\)[^;]*?\)\s*->\s*(\w+)', ln):
                    fields.append(('OS_REGS', m.group(1), rel, i, z))
    return usages, fields


def status(name, hdr, exports):
    if hdr is None:
        return ''
    if name in hdr['funcs']:
        return 'fonction' + ('' if exports is None or name in exports or name in hdr['macros']
                             else ' (non exportée par la lib)')
    if name in hdr['macros']:
        fn, repl = hdr['macros'][name]
        tgt = repl if re.fullmatch(r'\(?\s*OS_\w+\s*\)?', repl or '') else ''
        return ('macro' + ('()' if fn else '') + (' -> ' + tgt.strip('() ') if tgt else ''))
    if name in hdr['types']:
        return 'type'
    if name in hdr['enums']:
        return 'constante enum'
    if exports is not None and name in exports:
        return 'symbole lib non déclaré'
    for s in ('OS_TASK_STRUCT', 'OS_REGS_BASE', 'OS_GLOBAL_STRUCT'):
        if name in hdr['structs'].get(s, []):
            return 'champ ' + s
    return 'ABSENT'


def main():
    ap = argparse.ArgumentParser(description=__doc__.split('\n')[1])
    ap.add_argument('--embos', default=os.environ.get('LEPTON_EMBOS_ROOT'))
    ap.add_argument('--ref', help='RTOS.h de référence (version attendue par Lepton)')
    ap.add_argument('--trunk', default=os.environ.get('LEPTON_TRUNK'))
    ap.add_argument('--api', required=True)
    ap.add_argument('--work', required=True)
    ap.add_argument('--lib', default='libosT7VHLDP.a',
                    help='bibliothèque dont les symboles exportés servent de référence')
    a = ap.parse_args()
    if not a.embos or not a.trunk:
        sys.exit('LEPTON_EMBOS_ROOT / LEPTON_TRUNK non définis (source scripts/lepton-env.sh)')
    trunk_real = os.path.realpath(a.trunk)
    for out in (a.api, a.work):
        if os.path.realpath(out).startswith(trunk_real + os.sep):
            sys.exit('refus : écriture dans le trunk (%s)' % out)
    os.makedirs(a.work, exist_ok=True)

    inc = os.path.join(a.embos, 'Start', 'Inc', 'RTOS.h')
    libdir = os.path.join(a.embos, 'Start', 'Lib')
    tgt = parse_header(inc)
    ref = parse_header(a.ref) if a.ref else None
    exports, needs = lib_exports(libdir, a.lib)

    # ---- embos-api.txt (noms uniquement)
    with open(a.api, 'w', encoding='utf-8') as f:
        f.write('# API publique embOS extraite de Start/Inc/RTOS.h (noms uniquement, licence SFL :\n')
        f.write('# aucun code Segger reproduit). Généré par tools/migration/embos_inventory.py.\n')
        f.write('# Paquet : %s\n' % os.path.relpath(inc, os.path.dirname(os.path.dirname(a.embos))))
        f.write('# Version : %s (OS_VERSION_GENERIC %s)\n' % (tgt['version'], tgt['generic']))
        f.write('# Symboles exportés vérifiés sur : %s (%d symboles globaux définis)\n'
                % (a.lib, len(exports)))
        f.write('# Symboles externes requis par la lib (à fournir par l\'application/BSP/libc) :\n')
        f.write('#   %s\n' % ' '.join(sorted(needs)))
        f.write('# Légende fonctions : [L] exportée par la lib ; [M] aussi macro ; [-] déclarée seulement\n\n')
        f.write('## FONCTIONS (%d)\n' % len(tgt['funcs']))
        for n in sorted(tgt['funcs']):
            flag = 'L' if n in exports else ('M' if n in tgt['macros'] else '-')
            f.write('[%s] %s\n' % (flag, n))
        f.write('\n## MACROS (%d) — "->" : alias direct vers un autre identifiant\n' % len(tgt['macros']))
        for n in sorted(tgt['macros']):
            fn, repl = tgt['macros'][n]
            alias = repl.strip('() ') if re.fullmatch(r'\(?\s*OS_\w+\s*\)?', repl or '') else ''
            f.write('%s%s%s\n' % (n, '()' if fn else '', (' -> ' + alias) if alias else ''))
        f.write('\n## TYPES (%d)\n' % len(tgt['types']))
        for n in sorted(tgt['types']):
            f.write(n + '\n')
        f.write('\n## CONSTANTES D\'ÉNUMÉRATION (%d)\n' % len(tgt['enums']))
        for n in sorted(tgt['enums']):
            f.write(n + '\n')
        f.write('\n## CHAMPS DE STRUCTURES (sélection)\n')
        for s in ('OS_TASK_STRUCT', 'OS_REGS_BASE', 'OS_REGS_BASE_FPU', 'OS_GLOBAL_STRUCT'):
            if s in tgt['structs']:
                f.write('%s : %s\n' % (s, ', '.join(tgt['structs'][s])))
        f.write('\n## SYMBOLES EXPORTÉS PAR %s NON DÉCLARÉS DANS RTOS.h (%d)\n'
                % (a.lib, len([x for x in exports if x not in tgt['funcs'] and x not in tgt['macros']])))
        for n in sorted(exports):
            if n not in tgt['funcs'] and n not in tgt['macros']:
                f.write(n + '\n')

    # ---- variantes de bibliothèques
    rows = decode_libs(libdir)
    with open(os.path.join(a.work, 'library-variants.csv'), 'w', newline='', encoding='utf-8') as f:
        w = csv.DictWriter(f, fieldnames=list(rows[0].keys()))
        w.writeheader()
        w.writerows(rows)

    # ---- usages Lepton
    usages, fields = scan_lepton(a.trunk)
    with open(os.path.join(a.work, 'usages.csv'), 'w', newline='', encoding='utf-8') as f:
        w = csv.writer(f)
        w.writerow(['ident', 'fichier', 'ligne', 'zone'])
        w.writerows(usages)
    with open(os.path.join(a.work, 'tcb-fields.csv'), 'w', newline='', encoding='utf-8') as f:
        w = csv.writer(f)
        w.writerow(['objet', 'champ', 'fichier', 'ligne', 'zone',
                    'dans OS_TASK cible', 'dans OS_REGS_BASE cible'])
        tt = set(tgt['structs'].get('OS_TASK_STRUCT', []))
        rb = set(tgt['structs'].get('OS_REGS_BASE', []))
        for o, c, fl, ln, z in fields:
            w.writerow([o, c, fl, ln, z, 'oui' if c in tt else 'non', 'oui' if c in rb else 'non'])
    occ = defaultdict(int)
    files = defaultdict(set)
    zones = defaultdict(lambda: defaultdict(int))
    for n, fl, ln, z in usages:
        occ[n] += 1
        files[n].add(fl)
        zones[n][z] += 1
    with open(os.path.join(a.work, 'usage-summary.csv'), 'w', newline='', encoding='utf-8') as f:
        w = csv.writer(f)
        w.writerow(['ident', 'occurrences', 'fichiers', 'zones', 'statut cible %s' % tgt['version'],
                    'statut réf %s' % (ref['version'] if ref else '-'), 'fonction résolue (cible)',
                    'signature réf/cible'])
        for n in sorted(occ, key=lambda k: (-occ[k], k)):
            zs = ';'.join('%s=%d' % (k, v) for k, v in sorted(zones[n].items()))
            rt = resolve(n, tgt)
            rr = resolve(n, ref) if ref else None
            pt, pr = prototype(rt, tgt), prototype(rr, ref) if ref else None
            sig = ''
            if pt and pr:
                sig = 'identique' if pt.replace(rt, '#') == pr.replace(rr, '#') else \
                    'DIFFÉRENTE: [%s] / [%s]' % (pr, pt)
            elif pt or pr:
                sig = 'déclarée d\'un seul côté'
            w.writerow([n, occ[n], len(files[n]), zs, status(n, tgt, exports),
                        status(n, ref, None) if ref else '', rt if rt != n else '', sig])
    ifd = scan_ifdefs(a.trunk)
    with open(os.path.join(a.work, 'ifdef-hors-arch.csv'), 'w', newline='', encoding='utf-8') as f:
        w = csv.writer(f)
        w.writerow(['fichier', 'ligne', 'zone', 'jetons ISA/cœur'])
        w.writerows(ifd)
    print('#if ISA/cœur hors arch : %d lignes dans %d fichiers'
          % (len(ifd), len(set(r[0] for r in ifd))))
    print('API : %d fonctions, %d macros, %d types, %d enum ; lib %s : %d exports'
          % (len(tgt['funcs']), len(tgt['macros']), len(tgt['types']), len(tgt['enums']),
             a.lib, len(exports)))
    print('Lepton : %d occurrences OS_*, %d identifiants distincts, %d accès champs'
          % (len(usages), len(occ), len(fields)))


if __name__ == '__main__':
    main()
