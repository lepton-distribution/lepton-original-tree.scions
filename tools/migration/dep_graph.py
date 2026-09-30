#!/usr/bin/env python3
"""Graphe des dépendances entre composants Lepton (ETAPE-1, tâche 6).

Méthode (rejouable, Python 3 stdlib + gcc/nm hôtes) :
  1. recense les fichiers C des composants (lecture par le trunk, liens suivis) ;
  2. compile chaque fichier séparément avec le gcc hôte (`-m32 -ffreestanding -nostdinc`,
     sans édition de liens) en simulant la configuration IAR STM32F4 / embOS
     (projets `tauon_8.40.ewp` et `dev_stm32f4xx_8.40.ewp`) au moyen d'en-têtes de stub
     générés dans le répertoire intermédiaire (aucune écriture dans les sources) ;
  3. extrait les symboles globaux définis / non résolus (`nm -P`) ;
     pour les fichiers qui ne compilent pas : extraction textuelle APPROXIMATIVE
     (sortie `gcc -E` si le préprocesseur passe, sinon texte brut) ;
  4. construit le graphe orienté pondéré entre composants (poids = nombre de symboles
     distincts), les composantes fortement connexes (Tarjan) et les symboles non résolus ;
  5. écrit `doc/migration/dependances.md` et `doc/migration/dependances.dot`.

Usage :
  source scripts/lepton-env.sh
  python3 tools/migration/dep_graph.py [--trunk T] [--work W] [--out-dir D] [--jobs N]
Défauts : T=$LEPTON_TRUNK ou ~/lepton/trunk ; W=$LEPTON_BUILD/etape-1/depgraph ;
D=<clone>/doc/migration.
"""
import argparse
import collections
import concurrent.futures
import json
import os
import re
import subprocess
import sys

HOME = os.path.expanduser("~")
CLONE = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

# --------------------------------------------------------------------------------------
# Composants : (préfixe relatif à sys/root/src, nom du composant, nature)
# Ordre significatif : premier préfixe correspondant.
# --------------------------------------------------------------------------------------
EXCLUDED = [
    # (préfixe, raison)
    ("kernel/core/ucore/", "micro-noyau vendored (embOS/FreeRTOS/CMSIS) : hors graphe"),
    ("kernel/core/core-freertos/", "backend FreeRTOS (étape 7) : exclu, définitions concurrentes de core-segger"),
    ("kernel/core/arch/", "sorties mklepton win32 (gelé)"),
    ("kernel/dev/arch/arm7/", "gelé (ARM7)"),
    ("kernel/dev/arch/arm9/", "gelé (ARM9)"),
    ("kernel/dev/arch/at91/", "gelé (bibliothèques Atmel ARM7/ARM9)"),
    ("kernel/dev/arch/win32/", "gelé (simulation Windows)"),
    ("kernel/dev/arch/gnu32/", "gelé (simulation Linux)"),
    ("kernel/net/lwip/ports/gnu/", "port lwIP synthétique (gelé)"),
    ("kernel/net/lwip/ports/m16c/", "port lwIP M16C (gelé)"),
    ("kernel/net/lwip/ports/win32/", "port lwIP win32 (gelé)"),
    ("kernel/net/lwip/prj/", "projet scons lwIP"),
    ("kernel/net/lwip/netif/ppp/polarssl/", "compris dans netif/ppp (doublon polarssl)"),
]
BSP_STM32F4 = ("discovery_f4/", "discovery_f4-baseboard-modem/", "olimex_p407/", "stm32f469i-eval/")

COMPONENTS = [
    ("kernel/core/kal.c", "kal", "noyau"),
    ("kernel/core/core-segger/", "kernel/core/core-segger", "noyau"),
    ("kernel/core/core-generic/", "kernel/core/core-generic", "noyau"),
    ("kernel/core/net/", "kernel/core/net", "noyau"),
    ("kernel/core/usb/", "kernel/core/usb", "noyau"),
    ("kernel/core/", "kernel/core", "noyau"),
    ("kernel/dev/dev_", "kernel/dev (logiciel)", "pilote logiciel"),
    ("kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/", "kernel/dev/stm32f4xx-hal", "matériel vendored ST"),
    ("kernel/dev/arch/cortexm/stm32f4xx/driverlib/", "kernel/dev/stm32f4xx-hal", "matériel vendored ST"),
    ("kernel/dev/arch/cortexm/stm32f4xx/", "kernel/dev/stm32f4xx", "pilote matériel"),
    ("kernel/dev/arch/cmsis/", "kernel/dev/cmsis", "pilote matériel"),
    ("kernel/dev/arch/all/", "kernel/dev/all", "pilote matériel (circuits externes)"),
    ("kernel/dev/bsp/", "kernel/dev/bsp-stm32f4", "BSP"),
    ("kernel/fs/vfs/", "kernel/fs/vfs", "fs"),
    ("kernel/fs/", None, "fs"),  # kernel/fs/<x>
    ("kernel/net/lwip/", "kernel/net/lwip", "réseau"),
    ("kernel/net/uip2.5/", "kernel/net/uip2.5", "réseau"),
    ("kernel/net/uip/", "kernel/net/uip", "réseau"),
    ("kernel/usb/", "kernel/usb", "usb vendored ST"),
    ("lib/libc/", "lib/libc", "lib"),
    ("lib/pthread/", "lib/pthread", "lib"),
    ("lib/", None, "lib"),  # lib/<x>
    ("sbin/", "sbin", "commande"),
    ("bin/", "bin", "commande"),
]


# Composants mutuellement exclusifs (une seule pile IP par image) : même valeur = même groupe.
ALTERNATIVES = {"kernel/net/lwip": "ip", "kernel/net/uip": "ip", "kernel/net/uip2.5": "ip"}


def classify(rel):
    """Renvoie (composant | None, raison d'exclusion | None)."""
    for pre, why in EXCLUDED:
        if rel.startswith(pre):
            return None, why
    if rel.startswith("kernel/dev/arch/cortexm/") and not rel.startswith("kernel/dev/arch/cortexm/stm32f4xx/"):
        return None, "matériel Cortex-M hors STM32F4 (différé)"
    if rel.startswith("kernel/dev/arch/") and not rel.startswith(("kernel/dev/arch/cortexm/", "kernel/dev/arch/cmsis/", "kernel/dev/arch/all/")):
        return None, "matériel hors périmètre"
    if rel.startswith("kernel/dev/bsp/") and not rel[len("kernel/dev/bsp/"):].startswith(BSP_STM32F4):
        return None, "BSP hors STM32F4 (différé)"
    for pre, name, _ in COMPONENTS:
        if rel.startswith(pre):
            if name is None:
                parts = rel.split("/")
                name = "/".join(parts[:3]) if pre == "kernel/fs/" else "/".join(parts[:2])
            return name, None
    return None, "hors composants"


# --------------------------------------------------------------------------------------
# Configuration de compilation simulée
# --------------------------------------------------------------------------------------
INCLUDE_DIRS = [  # relatifs à sys/root/src (projets IAR tauon_8.40 / dev_stm32f4xx_8.40)
    "",
    "kernel/core/ucore/embOSCXM4_518/inc",
    "kernel/core/ucore/cmsis/CMSIS/Include",
    "kernel/net/lwip",
    "kernel/net/lwip/include",
    "kernel/net/lwip/ports/arm",
    "kernel/net/lwip/ports/arm/include",
    "kernel/net/lwip/include/ipv4",
    "kernel/fs/yaffs/core",
    "kernel/fs/yaffs/core/direct",
    "kernel/net/uip2.5",
    "kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/inc",
    "kernel/dev/arch/cortexm/stm32f4xx/cubemx_hal_driver/inc/legacy",
]

DEFINES = [
    # Simulation du compilateur IAR EWARM (kal.h n'a pas de branche GCC + embOS).
    "-U__GNUC__", "-U__GNUC_MINOR__", "-U__GNUC_PATCHLEVEL__",
    "-U__i386__", "-U__i386", "-Ui386", "-U__x86_64__", "-U__linux__", "-U__linux", "-Ulinux", "-U__unix__", "-Uunix",
    "-D__IAR_SYSTEMS_ICC__=8", "-D__ICCARM__=1", "-D__VER__=8040000",
    "-D__ARM6M__=12", "-D__ARM7M__=13", "-D__ARM7EM__=14", "-D__CORE__=__ARM7EM__",
    "-D__ARMVFP__=1", "-D__LITTLE_ENDIAN__=1",
    # Projets IAR (CCDefines)
    "-DOS_SUPPORT_CLEANUP_ON_TERMINATE", "-DOS_LIBMODE_SP", "-DNDEBUG", "-Dewarm",
    "-DUSE_HAL_DRIVER",
    # Cible (HYPOTHÈSE À VALIDER : pas d'en-tête STM32F439 dans cubemx, F429 retenu) ; en -D car
    # la HAL et la pile USB ST n'incluent pas kernelconf.h.
    "-DSTM32F429xx",
]

# Fichiers compilés avec la pile uIP au lieu de lwIP (variante de configuration).
UIP_VARIANT = ("kernel/core/net/uip_core/", "kernel/net/uip/", "kernel/net/uip2.5/", "kernel/dev/arch/all/ppp/")

STUB_MKCONF = r"""/* STUB dep_graph.py : remplace kernel/core/arch/cortexm/kernel_mkconf.h (généré par mklepton).
 * HYPOTHÈSE À VALIDER : valeurs reprises de mkconf_tauon_basic_stm32f4_lwip.xml et
 * user_kernel_mkconf.h (st-stm32f407-discovery) ; STM32F429xx faute d'en-tête F439 dans cubemx. */
#ifndef _KERNEL_MKCONF_H
#define _KERNEL_MKCONF_H
#define CPU_CORTEXM
#define __KERNEL_UCORE_EMBOS
#define STM32F429xx
#define HSE_VALUE ((uint32_t)8000000)
#define __tauon_cpu_device__ __tauon_cpu_device_cortexM4_stm32f4__
#define __tauon_kernel_profile__ __tauon_kernel_profile_classic__
#define __file_system_profile__ __file_system_profile_full__
#define __KERNEL_PRINTK
#define __KERNEL_TRACE_PRINTK
#define __KERNEL_DEV_TTY "/dev/ttys3"
#define __KERNEL_CPU_FREQ 168000000L
#define __KERNEL_HEAP_SIZE 10000
#define __KERNEL_PTHREAD_MAX 11
#define __KERNEL_PROCESS_MAX 8
#define MAX_OPEN_FILE 32
#define OPEN_MAX 16
#define __KERNEL_ENV_PATH {"/usr","/usr/sbin","/usr/bin","/usr/bin/net"}
#define __KERNEL_NET_IPSTACK
#if defined(DEPGRAPH_UIP)
#define USE_UIP
#else
#define USE_LWIP
#endif
#define USE_IF_ETHERNET
#endif
"""

STUB_IAR = r"""/* STUB dep_graph.py : neutralise les extensions IAR pour gcc hôte (-include). */
#ifndef DEPGRAPH_IAR_COMPAT_H
#define DEPGRAPH_IAR_COMPAT_H
/* HYPOTHÈSE À VALIDER : types stdint visibles partout (etypes.h les omet sous IAR C99 et
   ~18 fichiers les utilisent sans inclure stdint.h ; le build IAR les voit donc par la DLib). */
#include <stdint.h>
#define __no_init
#define __root
#define __ramfunc
#define __weak __attribute__((weak))
#define __packed __attribute__((packed))
#define __irq
#define __fiq
#define __swi
#define __arm
#define __thumb
#define __interwork
#define __nested
#define __task
#define __monitor
#define __intrinsic
#define __noreturn __attribute__((noreturn))
#define __stackless
#define __interrupt
#define __INLINE inline
#define __STATIC_INLINE static inline
#endif
"""

INTRINSICS = r"""/* STUB dep_graph.py : <intrinsics.h> IAR, fonctions neutres. */
#ifndef DEPGRAPH_INTRINSICS_H
#define DEPGRAPH_INTRINSICS_H
typedef unsigned long __istate_t;
static inline void __disable_interrupt(void) {}
static inline void __enable_interrupt(void) {}
static inline void __disable_irq(void) {}
static inline void __enable_irq(void) {}
static inline void __disable_fiq(void) {}
static inline void __enable_fiq(void) {}
static inline __istate_t __get_interrupt_state(void) { return 0; }
static inline void __set_interrupt_state(__istate_t s) { (void)s; }
static inline unsigned long __get_BASEPRI(void) { return 0; }
static inline void __set_BASEPRI(unsigned long v) { (void)v; }
static inline unsigned long __get_PRIMASK(void) { return 0; }
static inline void __set_PRIMASK(unsigned long v) { (void)v; }
static inline unsigned long __get_FAULTMASK(void) { return 0; }
static inline void __set_FAULTMASK(unsigned long v) { (void)v; }
static inline unsigned long __get_CONTROL(void) { return 0; }
static inline void __set_CONTROL(unsigned long v) { (void)v; }
static inline unsigned long __get_PSP(void) { return 0; }
static inline void __set_PSP(unsigned long v) { (void)v; }
static inline unsigned long __get_MSP(void) { return 0; }
static inline void __set_MSP(unsigned long v) { (void)v; }
static inline unsigned long __get_FPSCR(void) { return 0; }
static inline void __set_FPSCR(unsigned long v) { (void)v; }
static inline unsigned long __get_SP(void) { return 0; }
static inline unsigned long __get_LR(void) { return 0; }
static inline unsigned long __get_PC(void) { return 0; }
static inline unsigned long __get_CPSR(void) { return 0; }
static inline void __set_CPSR(unsigned long v) { (void)v; }
static inline unsigned long __REV(unsigned long v) { return v; }
static inline unsigned long __REV16(unsigned long v) { return v; }
static inline long __REVSH(long v) { return v; }
static inline unsigned long __RBIT(unsigned long v) { return v; }
static inline unsigned char __CLZ(unsigned long v) { return (unsigned char)v; }
static inline unsigned long __LDREX(unsigned long *p) { return *p; }
static inline unsigned long __STREX(unsigned long v, unsigned long *p) { *p = v; return 0; }
static inline void __CLREX(void) {}
static inline void __no_operation(void) {}
static inline void __NOP(void) {}
static inline void __WFI(void) {}
static inline void __WFE(void) {}
static inline void __SEV(void) {}
static inline void __ISB(void) {}
static inline void __DSB(void) {}
static inline void __DMB(void) {}
static inline void __BKPT(int v) { (void)v; }
static inline unsigned long __SSAT(long v, unsigned s) { (void)s; return v; }
static inline unsigned long __USAT(long v, unsigned s) { (void)s; return v; }
#endif
"""

# En-têtes système (DLib IAR) minimaux : types et prototypes ANSI.
SYSINC = {
    "intrinsics.h": INTRINSICS,
    "cmsis_iar.h": '#include <intrinsics.h>\n',
    "string.h": r"""#ifndef DG_STRING_H
#define DG_STRING_H
#include <stddef.h>
#include <stdint.h>
void *memcpy(void *, const void *, size_t); void *memmove(void *, const void *, size_t);
void *memset(void *, int, size_t); int memcmp(const void *, const void *, size_t);
void *memchr(const void *, int, size_t); size_t strlen(const char *);
char *strcpy(char *, const char *); char *strncpy(char *, const char *, size_t);
char *strcat(char *, const char *); char *strncat(char *, const char *, size_t);
int strcmp(const char *, const char *); int strncmp(const char *, const char *, size_t);
char *strchr(const char *, int); char *strrchr(const char *, int); char *strstr(const char *, const char *);
char *strtok(char *, const char *); size_t strspn(const char *, const char *);
size_t strcspn(const char *, const char *); char *strpbrk(const char *, const char *);
char *strerror(int); int strcoll(const char *, const char *);
#endif
""",
    "stdlib.h": r"""#ifndef DG_STDLIB_H
#define DG_STDLIB_H
#include <stddef.h>
#include <stdint.h>
#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1
#define RAND_MAX 32767
typedef struct { int quot, rem; } div_t;
typedef struct { long quot, rem; } ldiv_t;
int atoi(const char *); long atol(const char *); double atof(const char *);
long strtol(const char *, char **, int); unsigned long strtoul(const char *, char **, int);
double strtod(const char *, char **); int abs(int); long labs(long);
div_t div(int, int); ldiv_t ldiv(long, long); int rand(void); void srand(unsigned);
void qsort(void *, size_t, size_t, int (*)(const void *, const void *));
void *bsearch(const void *, const void *, size_t, size_t, int (*)(const void *, const void *));
#endif
""",
    "stdio.h": r"""#ifndef DG_STDIO_H
#define DG_STDIO_H
#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>
#ifndef EOF
#define EOF (-1)
#endif
int sprintf(char *, const char *, ...); int snprintf(char *, size_t, const char *, ...);
int vsprintf(char *, const char *, va_list); int vsnprintf(char *, size_t, const char *, va_list);
int sscanf(const char *, const char *, ...);
#endif
""",
    "ctype.h": r"""#ifndef DG_CTYPE_H
#define DG_CTYPE_H
int isalnum(int); int isalpha(int); int iscntrl(int); int isdigit(int); int isgraph(int);
int islower(int); int isprint(int); int ispunct(int); int isspace(int); int isupper(int);
int isxdigit(int); int tolower(int); int toupper(int);
#endif
""",
    "limits.h": r"""#ifndef DG_LIMITS_H
#define DG_LIMITS_H
#define CHAR_BIT 8
#define SCHAR_MIN (-128)
#define SCHAR_MAX 127
#define UCHAR_MAX 255
#define CHAR_MIN 0
#define CHAR_MAX 255
#define SHRT_MIN (-32768)
#define SHRT_MAX 32767
#define USHRT_MAX 65535
#define INT_MIN (-2147483647-1)
#define INT_MAX 2147483647
#define UINT_MAX 4294967295U
#define LONG_MIN (-2147483647L-1)
#define LONG_MAX 2147483647L
#define ULONG_MAX 4294967295UL
#endif
""",
    "assert.h": "#ifndef assert\n#define assert(x) ((void)0)\n#endif\n",
    "math.h": r"""#ifndef DG_MATH_H
#define DG_MATH_H
double sqrt(double); double pow(double, double); double floor(double); double ceil(double);
double fabs(double); double log(double); double log10(double); double exp(double);
double sin(double); double cos(double); double tan(double); double atan(double); double atan2(double, double);
double modf(double, double *); double fmod(double, double); double frexp(double, int *); double ldexp(double, int);
#endif
""",
    "setjmp.h": "#ifndef DG_SETJMP_H\n#define DG_SETJMP_H\ntypedef int jmp_buf[16];\nint setjmp(jmp_buf); void longjmp(jmp_buf, int);\n#endif\n",
    "errno.h": "#ifndef DG_ERRNO_H\n#define DG_ERRNO_H\n#include \"kernel/core/errno.h\"\n#endif\n",
    "yfuns.h": "/* STUB DLib <yfuns.h> */\n",
    "inttypes.h": "#ifndef DG_INTTYPES_H\n#define DG_INTTYPES_H\n#include <stdint.h>\n#define PRId32 \"ld\"\n#define PRIu32 \"lu\"\n#define PRIx32 \"lx\"\n#define PRIu16 \"u\"\n#define PRId16 \"d\"\n#define PRIu8 \"u\"\n#define PRIx8 \"x\"\n#define PRIx16 \"x\"\n#endif\n",
    "DLib_Threads.h": "/* STUB DLib <DLib_Threads.h> */\nvoid __iar_Initlocks(void);\n",
    "icclbutl.h": "/* STUB DLib <icclbutl.h> */\n",
}

CAUSES = [
    (re.compile(r"Assembler messages|Error: (no such instruction|bad register|invalid instruction|unknown pseudo-op|junk|bad expression|operand)|unknown register name|impossible constraint|invalid 'asm'|asm operand|register specified for"), "asm/registres ARM inline"),
    (re.compile(r"fatal error: (\S+): No such file"), "en-tête introuvable"),
    (re.compile(r"stray '@'|before '@'|expected .* '@'"), "placement IAR `@`"),
    (re.compile(r"#error"), "#error de configuration"),
    (re.compile(r"static declaration of .* follows non-static"), "`static` après déclaration non static (toléré par IAR)"),
    (re.compile(r"conflicting types|redefinition of|redeclaration of|conflicting type qualifiers"), "conflit de déclarations"),
    (re.compile(r"unknown type name|storage size of .* isn't known|has incomplete type|invalid use of undefined type|dereferencing pointer to incomplete"), "type inconnu/incomplet"),
    (re.compile(r"undeclared"), "identificateur non déclaré (config/macro)"),
    (re.compile(r"expected|stray"), "syntaxe (extension IAR ou config)"),
]


def sh(cmd, cwd=None):
    env = dict(os.environ, LC_ALL="C", LANG="C")
    p = subprocess.run(cmd, cwd=cwd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, env=env)
    return p.returncode, p.stdout.decode("latin-1"), p.stderr.decode("latin-1")


def write(path, text):
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, "w", encoding="utf-8") as f:
        f.write(text)


def make_stubs(work):
    stubs = os.path.join(work, "stubs")
    write(os.path.join(stubs, "inc/kernel/core/arch/cortexm/kernel_mkconf.h"), STUB_MKCONF)
    write(os.path.join(stubs, "iar_compat.h"), STUB_IAR)
    for name, text in SYSINC.items():
        write(os.path.join(stubs, "sysinc", name), text)
    return stubs


def classify_error(log):
    for line in log.splitlines():
        if "error" in line.lower() or "Assembler messages" in line:
            for rx, label in CAUSES:
                m = rx.search(line)
                if m:
                    if label == "en-tête introuvable":
                        return label + " : " + m.group(1)
                    return label
    for rx, label in CAUSES:
        if rx.search(log):
            return label
    return "autre"


# --------------------------------------------------------------------------------------
# Extraction textuelle approximative
# --------------------------------------------------------------------------------------
KEYWORDS = set("""if else while for do switch case default return sizeof break continue goto typedef
struct union enum static extern const volatile register auto inline signed unsigned int char short long
float double void defined __attribute__ __asm__ asm _Pragma __typeof__ typeof __extension__ __builtin_va_start
__builtin_va_end __builtin_va_arg __builtin_offsetof __builtin_va_copy""".split())
RX_COMMENT = re.compile(r"/\*.*?\*/|//[^\n]*", re.S)
RX_STRING = re.compile(r'"(?:\\.|[^"\\\n])*"|\'(?:\\.|[^\'\\\n])*\'')
RX_IDENT = re.compile(r"\b([A-Za-z_]\w*)\b")
RX_CALL = re.compile(r"\b([A-Za-z_]\w*)\s*\(")
RX_FUNCHEAD = re.compile(r"\b([A-Za-z_]\w*)\s*\((?:[^()]|\([^()]*\))*\)\s*(?:__attribute__\s*\(\(.*?\)\)\s*)?$", re.S)


def main_file_text(pre, src):
    """Garde les lignes de `gcc -E` provenant du fichier principal."""
    out, keep = [], True
    base = os.path.basename(src)
    for line in pre.splitlines():
        if line.startswith("# "):
            m = re.match(r'# \d+ "([^"]*)"', line)
            if m:
                keep = os.path.basename(m.group(1)) == base
            continue
        if keep:
            out.append(line)
    return "\n".join(out)


def text_symbols(text):
    """(définis globaux, références-appels, identificateurs des corps) — approximatif."""
    text = RX_COMMENT.sub(" ", text)
    text = RX_STRING.sub('""', text)
    text = re.sub(r"^\s*#.*$", " ", text, flags=re.M)
    defined, local, calls, idents = set(), set(), set(), set()
    depth, start, i, n = 0, 0, 0, len(text)
    body_start, cur_func = None, None
    while i < n:
        c = text[i]
        if c == "{":
            if depth == 0:
                head = text[start:i]
                m = RX_FUNCHEAD.search(head)
                hs = head.strip()
                if m and not re.match(r"(typedef|struct|union|enum)\b", hs) and "=" not in head.split("(")[0]:
                    name = m.group(1)
                    if name not in KEYWORDS:
                        cur_func = name
                        if re.search(r"\bstatic\b", head):
                            local.add(name)
                        else:
                            defined.add(name)
                        body_start = i
            depth += 1
        elif c == "}":
            depth -= 1
            if depth <= 0:
                depth = 0
                if body_start is not None:
                    body = text[body_start:i]
                    calls.update(x for x in RX_CALL.findall(body) if x not in KEYWORDS)
                    idents.update(RX_IDENT.findall(body))
                    body_start, cur_func = None, None
                start = i + 1
        elif c == ";" and depth == 0:
            stmt = text[start:i].strip()
            start = i + 1
            if stmt and not re.match(r"(typedef|extern|static)\b", stmt) and "(" not in stmt.split("=")[0]:
                lhs = stmt.split("=")[0].split("[")[0]
                ids = RX_IDENT.findall(lhs)
                if len(ids) >= 2 and ids[-1] not in KEYWORDS:
                    defined.add(ids[-1])
        i += 1
    calls -= local
    calls -= defined
    idents -= local
    idents -= defined
    return defined, calls, idents


# --------------------------------------------------------------------------------------
# Compilation
# --------------------------------------------------------------------------------------
def build_cmd(src_root, stubs, rel, out, gccinc, extra=()):
    cmd = ["gcc", "-m32", "-ffreestanding", "-nostdinc", "-fno-builtin", "-fno-pic", "-fno-pie",
           "-fpermissive", "-std=gnu99", "-w", "-O0", "-fno-common",
           "-isystem", os.path.join(stubs, "sysinc"), "-isystem", gccinc,
           "-include", os.path.join(stubs, "iar_compat.h")]
    cmd += DEFINES
    if rel.startswith(UIP_VARIANT):
        cmd.append("-DDEPGRAPH_UIP")
    if rel.startswith(("kernel/net/uip/", "kernel/core/net/uip_core/")):
        cmd.append("-I" + os.path.join(src_root, "kernel/net/uip/core"))  # contiki 3.0 (USE_UIP_VER 3000)
    for d in INCLUDE_DIRS:
        cmd.append("-I" + os.path.join(src_root, d))
    cmd.append("-I" + os.path.join(stubs, "inc"))
    # en-têtes introuvables résolus par recherche (casse Windows, include paths d'autres projets)
    cmd.append("-I" + os.path.join(stubs, "resolve"))
    # répertoire du fichier (inclusions relatives) : implicite pour "..." chez gcc
    cmd += list(extra)
    cmd += ["-c", os.path.join(src_root, rel), "-o", out]
    return cmd


def process(args):
    src_root, work, stubs, rel, gccinc = args
    obj = os.path.join(work, "obj", rel + ".o")
    log = os.path.join(work, "logs", rel + ".log")
    os.makedirs(os.path.dirname(obj), exist_ok=True)
    os.makedirs(os.path.dirname(log), exist_ok=True)
    rc, out, err = sh(build_cmd(src_root, stubs, rel, obj, gccinc))
    res = {"rel": rel, "ok": rc == 0}
    if rc == 0:
        rc2, nmout, _ = sh(["nm", "-P", obj])
        d, u = set(), set()
        for line in nmout.splitlines():
            parts = line.split()
            if len(parts) < 2:
                continue
            sym, typ = parts[0], parts[1]
            if typ in ("U", "w", "v"):
                u.add(sym)
            elif typ in ("T", "D", "B", "R", "C", "G", "S", "W", "V", "u", "i"):
                d.add(sym)
        u -= {"_GLOBAL_OFFSET_TABLE_"}
        d = {s for s in d if not s.startswith("__x86.")}
        res.update(defined=sorted(d), undef=sorted(u), idents=[], method="nm")
        if os.path.exists(log):
            os.remove(log)
    else:
        write(log, " ".join(build_cmd(src_root, stubs, rel, obj, gccinc)) + "\n\n" + err)
        res["cause"] = classify_error(err)
        m = RX_MISSING.search(err)
        if m:
            res["missing"] = (os.path.abspath(m.group(1)), m.group(2))
        # Repli : préprocesseur puis texte brut.
        cmd = build_cmd(src_root, stubs, rel, "-", gccinc)
        cmd[cmd.index("-c")] = "-E"
        rc3, pre, _ = sh(cmd)
        src_path = os.path.join(src_root, rel)
        if rc3 == 0 and pre.strip():
            text, method = main_file_text(pre, src_path), "texte (gcc -E)"
        else:
            with open(src_path, encoding="latin-1") as f:
                text, method = f.read(), "texte brut"
        d, calls, idents = text_symbols(text)
        res.update(defined=sorted(d), undef=sorted(calls), idents=sorted(idents), method=method)
    return res


def collect(src_root):
    files, excluded = [], collections.Counter()
    for dirpath, dirnames, filenames in os.walk(src_root, followlinks=True):
        dirnames.sort()
        for fn in sorted(filenames):
            if not fn.endswith(".c"):
                continue
            rel = os.path.relpath(os.path.join(dirpath, fn), src_root).replace(os.sep, "/")
            comp, why = classify(rel)
            if comp is None:
                excluded[why] += 1
            else:
                files.append((rel, comp))
    return files, excluded


# --------------------------------------------------------------------------------------
# Graphe
# --------------------------------------------------------------------------------------
def tarjan(nodes, adj):
    index, low, onstack, stack, sccs = {}, {}, set(), [], []
    counter = [0]
    sys.setrecursionlimit(10000)

    def strong(v):
        index[v] = low[v] = counter[0]
        counter[0] += 1
        stack.append(v)
        onstack.add(v)
        for w in adj.get(v, ()):
            if w not in index:
                strong(w)
                low[v] = min(low[v], low[w])
            elif w in onstack:
                low[v] = min(low[v], index[w])
        if low[v] == index[v]:
            comp = []
            while True:
                w = stack.pop()
                onstack.discard(w)
                comp.append(w)
                if w == v:
                    break
            sccs.append(sorted(comp))

    for v in sorted(nodes):
        if v not in index:
            strong(v)
    return sccs


LIBC_NAMES = set("""memcpy memmove memset memcmp memchr strlen strcpy strncpy strcat strncat strcmp strncmp
strchr strrchr strstr strtok strspn strcspn strpbrk strerror strcoll strdup strcasecmp strncasecmp
atoi atol atof strtol strtoul strtod abs labs div ldiv rand srand qsort bsearch malloc calloc realloc free
exit abort atexit getenv sprintf snprintf vsprintf vsnprintf sscanf printf fprintf vprintf vfprintf
isalnum isalpha iscntrl isdigit isgraph islower isprint ispunct isspace isupper isxdigit tolower toupper
sqrt pow floor ceil fabs log log10 exp sin cos tan atan atan2 modf fmod frexp ldexp setjmp longjmp
""".split())
INTRINSIC_NAMES = set("""__disable_interrupt __enable_interrupt __get_BASEPRI __set_BASEPRI __get_PRIMASK
__set_PRIMASK __no_operation __get_interrupt_state __set_interrupt_state __iar_Initlocks""".split())


def sym_category(s):
    if s in LIBC_NAMES:
        return "libc C standard (DLib/newlib)"
    if s.startswith("OS_") or s.startswith("_OS_"):
        return "embOS"
    if s.startswith(("HAL_", "LL_", "ETH_", "RCC_", "GPIO_", "USART_", "SPI_", "I2C_", "SDIO_", "NVIC_", "SysTick", "USB_", "USBD_", "SystemCoreClock", "SystemInit")):
        return "HAL/CMSIS ST"
    if s.startswith(("__aeabi_", "__iar_", "__divdi3", "__udivdi3", "__moddi3", "__umoddi3")) or s in INTRINSIC_NAMES:
        return "intrinsèque/runtime compilateur"
    if s.endswith("_map") or s in ("pdev_lst", "dev_lst", "bin_lst", "boot_dev_lst", "_boot_dev_lst", "boot_handler") or s.startswith(("_dskimg", "dskimg")):
        return "généré par mklepton"
    return "autre (application/BSP/symbole manquant)"


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--trunk", default=os.environ.get("LEPTON_TRUNK", os.path.join(HOME, "lepton/trunk")))
    ap.add_argument("--work", default=os.path.join(os.environ.get("LEPTON_BUILD", os.path.join(HOME, "lepton/build")), "etape-1/depgraph"))
    ap.add_argument("--out-dir", default=os.path.join(CLONE, "doc/migration"))
    ap.add_argument("--jobs", type=int, default=os.cpu_count() or 2)
    ap.add_argument("--reuse", action="store_true", help="réutiliser results.json (pas de recompilation)")
    a = ap.parse_args()

    src_root = os.path.join(a.trunk, "sys/root/src")
    work = os.path.abspath(a.work)
    for p in (work, a.out_dir):
        if os.path.realpath(p).startswith(os.path.realpath(a.trunk)):
            sys.exit("refus : écriture dans le trunk (%s)" % p)
    os.makedirs(work, exist_ok=True)
    results_path = os.path.join(work, "results.json")

    files, excluded = collect(src_root)
    comp_of = dict(files)
    if a.reuse and os.path.exists(results_path):
        with open(results_path) as f:
            results = json.load(f)
    else:
        stubs = make_stubs(work)
        gccinc = sh(["gcc", "-m32", "-print-file-name=include"])[1].strip()
        resolve_dir = os.path.join(stubs, "resolve")
        if os.path.isdir(resolve_dir):
            import shutil
            shutil.rmtree(resolve_dir)
        os.makedirs(resolve_dir)
        index = header_index(src_root)
        resolved = {}
        todo = [rel for rel, _ in files]
        byrel = {}
        for it in range(6):
            jobs = [(src_root, work, stubs, rel, gccinc) for rel in todo]
            with concurrent.futures.ThreadPoolExecutor(a.jobs) as ex:
                for r in ex.map(process, jobs):
                    byrel[r["rel"]] = r
            print("  passe %d : %d fichiers, %d en échec" % (it + 1, len(todo),
                  sum(1 for r in byrel.values() if not r["ok"])), file=sys.stderr)
            new = add_resolutions(byrel, index, resolve_dir, resolved, src_root)
            if not new:
                break
            todo = [rel for rel, r in byrel.items() if not r["ok"]]
        results = [byrel[rel] for rel, _ in files]
        with open(os.path.join(work, "resolved-headers.tsv"), "w") as f:
            for name, (target, why) in sorted(resolved.items()):
                f.write("%s\t%s\t%s\n" % (name, os.path.relpath(target, src_root) if target else "-", why))
        with open(results_path, "w") as f:
            json.dump(results, f)
    report(results, comp_of, excluded, a.out_dir, work)


def header_index(src_root):
    idx = collections.defaultdict(list)  # nom de base en minuscules -> chemins
    for dirpath, dirnames, filenames in os.walk(src_root, followlinks=True):
        for fn in filenames:
            if fn.lower().endswith((".h", ".hpp")):
                idx[fn.lower()].append(os.path.join(dirpath, fn))
    return idx


RX_MISSING = re.compile(r"^(\S+?):\d+:\d+: fatal error: (\S+): No such file", re.M)


def add_resolutions(byrel, index, resolve_dir, resolved, src_root):
    """Crée des en-têtes de renvoi pour les inclusions introuvables. Renvoie le nombre ajouté."""
    added = 0
    for r in byrel.values():
        if r["ok"] or not r.get("missing"):
            continue
        requester, name = r["missing"]
        if name in resolved:
            continue
        norm = name.replace("\\", "/")  # séparateurs Windows dans #include
        cands = [p for p in index.get(os.path.basename(norm).lower(), [])
                 if p.replace(os.sep, "/").lower().endswith("/" + norm.lower())]
        if not cands:
            resolved[name] = (None, "introuvable dans l'arbre")
            continue
        # priorité aux include paths du projet, puis au plus proche du demandeur
        incs = [os.path.join(src_root, d) for d in INCLUDE_DIRS if d]
        cands.sort(key=lambda p: (not any(p.startswith(i + os.sep) for i in incs),
                                  -len(os.path.commonpath([p, requester])), p))
        target = cands[0]
        exact = target.endswith(norm)
        why = ("séparateur `\\` (Windows)" if norm != name else
               "casse différente (Windows)" if not exact else "hors include path du projet")
        if len(cands) > 1:
            why += " ; %d candidats, le plus proche retenu" % len(cands)
        write(os.path.join(resolve_dir, name), '#include "%s"\n' % target)
        resolved[name] = (target, why)
        added += 1
    return added


def report(results, comp_of, excluded, out_dir, work):
    comps = sorted(set(comp_of.values()))
    stats = {c: collections.Counter() for c in comps}
    causes = collections.Counter()
    causes_by_comp = collections.defaultdict(collections.Counter)
    empty = collections.Counter()
    defs = collections.defaultdict(set)          # symbole -> composants définisseurs
    defs_nm = collections.defaultdict(set)
    for r in results:
        c = comp_of[r["rel"]]
        stats[c]["total"] += 1
        if r["ok"]:
            stats[c]["ok"] += 1
            if not r["defined"] and not r["undef"]:
                empty[c] += 1
        else:
            causes[r["cause"]] += 1
            causes_by_comp[c][r["cause"]] += 1
        for s in r["defined"]:
            if not r["ok"] and len(s) <= 3:
                continue  # extraction textuelle : noms courts trop ambigus (data, p, id…)
            defs[s].add(c)
            if r["ok"]:
                defs_nm[s].add(c)

    # Arêtes : edges[(a,b)] = {symbole: 'nm'|'texte'}
    edges = collections.defaultdict(dict)
    unresolved = collections.defaultdict(set)  # symbole -> composants demandeurs
    unresolved_text = collections.defaultdict(set)
    multi = {}
    for r in results:
        a = comp_of[r["rel"]]
        kind = "nm" if r["ok"] else "texte"
        refs = set(r["undef"])
        if not r["ok"]:
            # identificateurs des corps (variables globales) si définis ailleurs
            refs |= {x for x in r["idents"] if x in defs_nm and len(x) > 3}
        for s in refs:
            owners = defs.get(s)
            if owners:
                # piles IP alternatives : jamais de résolution d'une pile vers une autre
                alt = ALTERNATIVES.get(a)
                owners = {o for o in owners if ALTERNATIVES.get(o) is None or ALTERNATIVES.get(o) != alt or o == a}
                if kind == "texte" and a not in ("bin", "sbin"):
                    # extraction textuelle : variables locales homonymes des globales des commandes
                    owners = {o for o in owners if o not in ("bin", "sbin")}
                if not owners:
                    continue
                if len(owners) > 1 and a not in owners and a not in ALTERNATIVES:
                    alt_owners = {o for o in owners if o in ALTERNATIVES}
                    if alt_owners and len(alt_owners) < len(owners):
                        owners -= alt_owners
            if not owners:
                if r["ok"]:
                    unresolved[s].add(a)
                else:
                    unresolved_text[s].add(a)
                continue
            if a in owners:
                continue
            if len(owners) > 1:
                multi[s] = owners
            for b in owners:
                prev = edges[(a, b)].get(s)
                edges[(a, b)][s] = "nm" if (prev == "nm" or kind == "nm") else "texte"

    adj = collections.defaultdict(set)
    for (x, y) in edges:
        adj[x].add(y)
    sccs = [s for s in tarjan(comps, adj) if len(s) > 1]

    # ---------------------------------------------------------------- Markdown
    L = []
    w = L.append
    tot = sum(s["total"] for s in stats.values())
    ok = sum(s["ok"] for s in stats.values())
    w("# Graphe des dépendances entre composants (ETAPE-1, tâche 6)")
    w("")
    w("Généré par `tools/migration/dep_graph.py` — ne pas éditer à la main. Graphe : `dependances.dot`")
    w("(`dot -Tsvg dependances.dot -o dependances.svg`). Intermédiaires (objets, journaux d'erreurs, stubs,")
    w("`results.json`) : `$LEPTON_BUILD/etape-1/depgraph/`.")
    w("")
    w("## 1. Méthode")
    w("")
    w("- Chaque fichier `.c` des composants est compilé **isolément** par le gcc hôte "
      "(`gcc -m32 -ffreestanding -nostdinc -fno-builtin -fpermissive -w -c`, sans édition de liens), puis "
      "`nm -P` donne les symboles globaux définis (T/D/B/R/C/W/V) et non résolus (U).")
    w("- `-m32` : le multilib gcc est présent mais **pas la libc 32 bits** (`libc6-dev-i386` absent) ; "
      "d'où `-ffreestanding -nostdinc` + en-têtes système minimaux générés (`stubs/sysinc` : string.h, "
      "stdlib.h, stdio.h, ctype.h, limits.h, math.h, setjmp.h, intrinsics.h…, simulant la DLib IAR).")
    w("- **Configuration simulée : celle du build IAR de référence**, pas une configuration GCC : "
      "`kal.h` n'a aucune branche GCC + embOS (seulement IAR/Keil), donc `-U__GNUC__ -D__ICCARM__ "
      "-D__IAR_SYSTEMS_ICC__=8 -D__CORE__=__ARM7EM__` ; extensions IAR neutralisées par `-include "
      "stubs/iar_compat.h` (`__no_init`, `__root`, `__ramfunc`, `__packed`, `__weak`…) ; defines des projets "
      "`tauon_8.40.ewp` / `dev_stm32f4xx_8.40.ewp` (`OS_LIBMODE_SP`, `OS_SUPPORT_CLEANUP_ON_TERMINATE`, "
      "`NDEBUG`) ; include paths de ces projets (embOS `embOSCXM4_518/inc`, CMSIS, lwIP, yaffs, uip2.5, "
      "cubemx HAL).")
    w("- `kernel/core/arch/cortexm/kernel_mkconf.h` (sortie mklepton, absente de l'arbre) est remplacé par un "
      "stub : STM32F4 / Cortex-M4, embOS, profil noyau *classic*, profil fs *full* (tous les fs référencés par "
      "le VFS), `__KERNEL_NET_IPSTACK` + `USE_LWIP` + `USE_IF_ETHERNET`, `STM32F429xx`. "
      "**HYPOTHÈSE À VALIDER** : valeurs reprises de `mkconf_tauon_basic_stm32f4_lwip.xml` et du "
      "`user_kernel_mkconf.h` discovery F4 ; pas d'en-tête STM32F439 dans cubemx, F429 retenu.")
    w("- Variante uIP : les fichiers de `kernel/core/net/uip_core`, `kernel/net/uip*`, `kernel/dev/arch/all/ppp` "
      "sont compilés avec `USE_UIP` au lieu de `USE_LWIP`.")
    w("- Fichiers non compilables : cause dominante relevée (première erreur), puis **extraction textuelle "
      "APPROXIMATIVE** : sur la sortie `gcc -E` (lignes du fichier principal) si le préprocesseur passe, sinon "
      "sur le texte brut ; définitions = fonctions non `static` et variables globales simples ; références = "
      "appels `f(` dans les corps + identificateurs des corps définis ailleurs. Les arêtes qui ne reposent "
      "que sur cette extraction sont signalées (colonne *texte*).")
    w("- Arête A → B : A référence (U) un symbole défini dans B et non dans A ; poids = nombre de symboles "
      "distincts. Un symbole défini dans plusieurs composants crée une arête vers chacun (voir §6).")
    w("- Aucune modification de source ; aucune écriture dans le trunk ni dans `scion/`.")
    w("")
    w("### Périmètre")
    w("")
    w("| Composant | Contenu |")
    w("|---|---|")
    desc = {
        "kal": "`kernel/core/kal.c` (l'API KAL est surtout dans `kal.h`, macros/inline)",
        "kernel/core": "`kernel/core/*.c` hors `kal.c`",
        "kernel/core/core-segger": "backend embOS",
        "kernel/core/core-generic": "`kernel_pthread_tsd.c`",
        "kernel/core/net": "couche socket noyau (`lwip_core`, `uip_core`, `modem_core`)",
        "kernel/core/usb": "cœur USB STM32 (`stm32_usb_core`)",
        "kernel/dev (logiciel)": "`kernel/dev/dev_*` (null, proc, cpufs, head, tty, mem, fb, part, loadavg)",
        "kernel/dev/stm32f4xx": "pilotes Lepton `dev/arch/cortexm/stm32f4xx` (hors HAL)",
        "kernel/dev/stm32f4xx-hal": "HAL ST vendored (`cubemx_hal_driver`, `driverlib`)",
        "kernel/dev/cmsis": "`dev/arch/cmsis` (cpu, ITM) — commun Cortex-M",
        "kernel/dev/all": "`dev/arch/all` : circuits externes (eth, flash, i2c, lcd, modem, ppp, sd…)",
        "kernel/dev/bsp-stm32f4": "BSP discovery_f4, discovery_f4-baseboard-modem, olimex_p407, stm32f469i-eval",
        "kernel/usb": "pile USB device ST (`kernel/usb/stm32f4-usb-core`)",
    }
    for c in comps:
        w("| `%s` | %s |" % (c, desc.get(c, "`%s`" % c)))
    w("")
    w("Exclus (fichiers `.c`) :")
    w("")
    for why, n in excluded.most_common():
        w("- %s : %d" % (why, n))
    w("")

    w("## 2. Taux de compilation par composant")
    w("")
    w("Total : **%d / %d** fichiers compilés (%.0f %%). *Vides* : objets compilés sans aucun symbole "
      "global (fichier exclu par la configuration, ex. pile réseau non retenue)." % (ok, tot, 100.0 * ok / max(tot, 1)))
    w("")
    w("| Composant | Fichiers | Compilés | Taux | Vides | Causes d'échec dominantes |")
    w("|---|---:|---:|---:|---:|---|")
    for c in comps:
        s = stats[c]
        cc = ", ".join("%s (%d)" % (k, v) for k, v in causes_by_comp[c].most_common(3))
        w("| `%s` | %d | %d | %.0f %% | %d | %s |" % (c, s["total"], s["ok"], 100.0 * s["ok"] / max(s["total"], 1), empty[c], cc or "—"))
    w("")
    w("Causes d'échec (toutes) :")
    w("")
    for k, v in causes.most_common(25):
        w("- %s : %d" % (k, v))
    w("")
    w("Fichiers en échec : liste et journal par fichier dans `$LEPTON_BUILD/etape-1/depgraph/logs/`.")
    w("")

    # Matrice
    w("## 3. Matrice des dépendances (symboles)")
    w("")
    w("Ligne = composant qui référence, colonne = composant qui définit. Abréviations :")
    w("")
    abbr = {c: "C%d" % i for i, c in enumerate(comps, 1)}
    w(", ".join("%s=`%s`" % (abbr[c], c) for c in comps))
    w("")
    w("| | " + " | ".join(abbr[c] for c in comps) + " |")
    w("|---|" + "---:|" * len(comps))
    for x in comps:
        row = []
        for y in comps:
            e = edges.get((x, y))
            row.append(str(len(e)) if e else "")
        w("| %s | %s |" % (abbr[x], " | ".join(row)))
    w("")
    w("### Arêtes principales (poids ≥ 5)")
    w("")
    w("| De | Vers | Symboles | dont *texte* seul | Exemples |")
    w("|---|---|---:|---:|---|")
    for (x, y), syms in sorted(edges.items(), key=lambda kv: -len(kv[1])):
        if len(syms) < 5:
            continue
        nt = sum(1 for v in syms.values() if v == "texte")
        ex = ", ".join("`%s`" % s for s in sorted(syms)[:6])
        w("| `%s` | `%s` | %d | %d | %s |" % (x, y, len(syms), nt, ex))
    w("")

    # Cycles
    w("## 4. Cycles (composantes fortement connexes)")
    w("")
    if not sccs:
        w("Aucun cycle.")
    for i, scc in enumerate(sorted(sccs, key=len, reverse=True), 1):
        inner = {(x, y): s for (x, y), s in edges.items() if x in scc and y in scc}
        dens = sum(len(s) for s in inner.values())
        w("### CFC %d : %d composants, %d arêtes internes, %d symboles en jeu" % (i, len(scc), len(inner), dens))
        w("")
        w(", ".join("`%s`" % c for c in scc))
        w("")
        # paires réciproques
        w("Paires réciproques (A→B / B→A) :")
        w("")
        w("| A | B | A→B | B→A | Exemples A→B | Exemples B→A |")
        w("|---|---|---:|---:|---|---|")
        seen = set()
        pairs = []
        for (x, y) in inner:
            if (y, x) in inner and (y, x) not in seen:
                seen.add((x, y))
                pairs.append((x, y))
        for x, y in sorted(pairs, key=lambda p: -min(len(inner[p]), len(inner[(p[1], p[0])]))):
            ab, ba = inner[(x, y)], inner[(y, x)]
            w("| `%s` | `%s` | %d | %d | %s | %s |" % (x, y, len(ab), len(ba),
              ", ".join("`%s`" % s for s in sorted(ab)[:4]), ", ".join("`%s`" % s for s in sorted(ba)[:4])))
        w("")
        w("Arêtes internes de faible poids (≤ 3 symboles : candidats à couper) :")
        w("")
        weak = sorted(((x, y), s) for (x, y), s in inner.items() if len(s) <= 3)
        for (x, y), s in weak:
            w("- `%s` → `%s` : %s" % (x, y, ", ".join("`%s`%s" % (k, "*" if v == "texte" else "") for k, v in sorted(s.items()))))
        if not weak:
            w("- (aucune)")
        w("")

    # Non résolus
    w("## 5. Symboles non résolus dans tout le périmètre")
    w("")
    w("Référencés (nm, fichiers compilés) mais définis dans aucun composant : fournis par la libc, embOS, "
      "la HAL/CMSIS, le runtime compilateur, les sorties mklepton ou l'application.")
    w("")
    bycat = collections.defaultdict(list)
    for s, cs in unresolved.items():
        bycat[sym_category(s)].append((s, cs))
    w("| Catégorie | Nombre | Exemples (composants demandeurs) |")
    w("|---|---:|---|")
    for cat in sorted(bycat, key=lambda k: -len(bycat[k])):
        lst = sorted(bycat[cat], key=lambda t: (-len(t[1]), t[0]))
        ex = "; ".join("`%s` (%s)" % (s, ", ".join(sorted(cs))) for s, cs in lst[:10])
        w("| %s | %d | %s |" % (cat, len(lst), ex))
    w("")
    w("Liste complète : `$LEPTON_BUILD/etape-1/depgraph/unresolved.txt`. Appels non résolus issus de "
      "l'extraction textuelle (non comptés ici, bruit de macros) : %d." % len(unresolved_text))
    w("")

    # multi-définis
    w("## 6. Symboles définis dans plusieurs composants")
    w("")
    md = collections.Counter(tuple(sorted(v)) for v in multi.values())
    if not md:
        w("Aucun parmi les symboles référencés.")
    else:
        w("| Composants | Symboles référencés | Exemples |")
        w("|---|---:|---|")
        for owners, n in md.most_common(15):
            ex = [s for s, o in multi.items() if tuple(sorted(o)) == owners][:5]
            w("| %s | %d | %s |" % (", ".join("`%s`" % o for o in owners), n, ", ".join("`%s`" % s for s in sorted(ex))))
    w("")

    # Proposition
    w("## 7. Proposition de découpage en bibliothèques statiques (étape 2)")
    w("")
    # Variantes : robustesse des cycles
    def sccs_of(pred):
        ad = collections.defaultdict(set)
        for (x, y), syms in edges.items():
            if any(pred(x, y, k, v) for k, v in syms.items()):
                ad[x].add(y)
        return [c for c in tarjan(comps, ad) if len(c) > 1]
    nm_only = sccs_of(lambda x, y, k, v: v == "nm")
    cut_ops = sccs_of(lambda x, y, k, v: v == "nm" and not (x == "kernel/fs/vfs" and k.endswith("_op")))
    w("### Robustesse des cycles")
    w("")
    w("- Arêtes *nm* seules (sans extraction textuelle) : " + ("; ".join(
        "{" + ", ".join("`%s`" % c for c in scc) + "}" for scc in nm_only) or "aucun cycle") + ".")
    w("- *nm* seules **et** sans les références du VFS aux tables d'opérations des fs (`*_op`, "
      "à générer par mklepton ou à enregistrer à l'initialisation) : " + ("; ".join(
        "{" + ", ".join("`%s`" % c for c in scc) + "}" for scc in cut_ops) or "aucun cycle") + ".")
    w("")
    w(proposal(comps, edges, sccs))
    w("")
    w(ANALYSE)
    w("")
    w("## 8. Limites")
    w("")
    w("- Configuration unique (STM32F4, embOS, lwIP, profil fs *full*) : le code sous `#if` d'autres "
      "configurations n'apparaît pas ; les objets *vides* le signalent. uIP compilé en variante séparée.")
    w("- Hôte x86 32 bits : asm inline ARM, intrinsèques non stubées et placements `@` empêchent la "
      "compilation ; ces fichiers ne contribuent que par extraction textuelle (approximative : macros non "
      "développées si le préprocesseur échoue, pointeurs de fonction et tables non vus).")
    w("- Symboles référencés via des tables générées (`dev_lst`, `bin_lst`… sorties mklepton) : invisibles ; "
      "les pilotes et commandes apparaissent donc « non référencés » par le noyau alors qu'ils le sont au link final.")
    w("- Les fonctions `static inline` des en-têtes (dont `kal.h`) sont compilées dans chaque utilisateur : la "
      "dépendance vers `kal` (API en macros/inline) est sous-estimée ; elle se traduit ici en références "
      "directes vers embOS (`OS_*`) et `core-segger`.")
    w("- En-têtes système simulés (DLib) : un conflit de déclarations peut provenir du stub et non du code.")
    write(os.path.join(out_dir, "dependances.md"), "\n".join(L) + "\n")

    with open(os.path.join(work, "unresolved.txt"), "w") as f:
        for s in sorted(unresolved):
            f.write("%s\t%s\t%s\n" % (s, sym_category(s), ",".join(sorted(unresolved[s]))))
    with open(os.path.join(work, "edges.tsv"), "w") as f:
        for (x, y), syms in sorted(edges.items()):
            for s, k in sorted(syms.items()):
                f.write("%s\t%s\t%s\t%s\n" % (x, y, s, k))
    with open(os.path.join(work, "failed.tsv"), "w") as f:
        for r in sorted(results, key=lambda r: r["rel"]):
            if not r["ok"]:
                f.write("%s\t%s\t%s\n" % (r["rel"], r["cause"], r["method"]))
    dot(comps, edges, sccs, stats, os.path.join(out_dir, "dependances.dot"))


ANALYSE = """### Analyse et proposition (rédigée sur la passe du 2026-09-30 ; à relire si les chiffres changent)

**Nœud du problème : un seul gros cycle « noyau + libc + réseau ».** Il ne tient pas à l'extraction
textuelle (il subsiste avec les arêtes *nm* seules). Ses mécanismes, du plus dense au plus ténu :

1. `kernel/core` ⇄ `kernel/core/core-segger` (≈ 27 / 21 symboles) et `core-segger` ⇄ `kernel/fs/vfs`
   (≈ 46 / 10 : les `_syscall_*` sont définis dans `vfskernel.c`, le VFS lit fd/errno/propriétaire dans le
   backend). Densité forte dans les deux sens : **indissociable** sans refactorisation.
2. `kernel/fs/vfs` → tables `*_op` de chaque fs (1–2 symboles) et fs → `ofile_lst`, `_vfs_*`. Cycle
   **artificiel** : la table des fs montés est codée en dur dans le VFS (profil fs). Coupure : table des fs
   générée (comme `dev_lst`/`bin_lst` par mklepton) → `fat`, `kofs`, `rootfs`, `ufs` deviennent acycliques.
3. `lib/libc` ⇄ noyau : libc → `kernel/core` / `core-segger` / `kernel/core/net` (≈ 10 + 8 + 12 : appels
   système, `kernel_io_*`, `kernel_net_core_*`) ; retour noyau → libc **7 symboles seulement** : `__sprintf`,
   `__sscanf`, `__printf`, `__stdio_init`, `ldiv`, `__lepton_libc_isascii`, `libc_lib_entrypoint`.
   Contrainte « `lib` hors de `kernel/` » : ce retour impose un groupe de liens transversal tant qu'il existe
   (coupure possible à l'étape 4 : formatage noyau via `kernel_printk`, point d'entrée libc enregistré).
4. `kernel/core/net` ⇄ piles IP : callbacks `uip_sock_*_callback`, `uip_hostaddr` définis dans
   `kernel/core/net` et appelés par uIP (3–4 symboles) ; lwIP ne revient que vers `kernel/core`
   (`_sys_malloc`) et `core-segger` (pthreads), et entre dans le cycle via `lib/libc` → `lwip_htonl`.
5. `kernel/dev/all` n'y entre que par une arête textuelle (`trap_lion_flag`) : acyclique en *nm*.

**Découpage proposé pour l'étape 2** (une bibliothèque STATIC par composant, cibles CMake) :

| Groupe de liens | Bibliothèques | Raison |
|---|---|---|
| G1 « noyau » (`LINK_GROUP:RESCAN`) | `k_core`, `k_core_segger` (ou `k_core_freertos` à l'étape 7), `k_core_generic`, `k_fs_vfs`, `k_core_net`, pile IP retenue (`k_net_lwip` **ou** `k_net_uip`), `lib_libc` | cycles 1, 3, 4 ; `core-segger` reste une bibliothèque séparée (axe micro-noyau interchangeable) |
| G1 tant que la table des fs n'est pas générée | `k_fs_fat`, `k_fs_kofs`, `k_fs_rootfs`, `k_fs_ufs` | cycle 2 ; sortent du groupe dès la génération de la table |
| fusion recommandée | `k_core_usb` + `k_usb` (≈ 11 / 9) | cycle dense entre glue Lepton et pile USB ST, toujours liés ensemble |
| acycliques (ordre de lien simple) | `sbin`, `bin` → `lib_pthread`, `lib_librt`, `lib_lib_nxpnfc` → G1 → `k_dev_sw`, `k_dev_bsp_stm32f4` → `k_dev_stm32f4xx`, `k_dev_all`, `k_dev_cmsis`, `k_fs_fatfs`, `k_fs_yaffs` → `k_dev_stm32f4xx_hal` (vendored) | pas de retour |
| pas de bibliothèque | `kal` | `kal.c` n'a de code que sous `CPU_WIN32` (objet vide sur cible) : le KAL est `kal.h` (macros/inline) + le backend |

Notes : les piles lwIP / uIP (contiki 3.0) / uIP 2.5 sont **mutuellement exclusives** (≈ 20 symboles
`uip_*` définis dans les deux uIP, `tcpip_init`/`tcpip_input` dans les trois) : une seule par image,
choix par option CMake. Les quatre BSP STM32F4 définissent aussi des symboles concurrents : une
bibliothèque BSP par carte. `bin`/`sbin` définissent des globales non `static` génériques (`byte`,
`offset`, `prompt`…) : collisions possibles dans une image statique unique (à surveiller à l'étape 3).

**Constats utiles hors graphe** (issus de la compilation) : `kal.h` n'a pas de branche GCC + embOS
(uniquement IAR/Keil ; et la condition de la branche embOS se termine par `|| (__tauon_cpu_core__ ==
cortexM7)` hors parenthèses, vraie pour tout compilateur sur M7) ; 14 fichiers redéclarent `static` une
fonction déjà déclarée non `static` dans leur en-tête (accepté par IAR, erreur GCC) ; inclusions
dépendantes de Windows (casse `RTOS.H`, `Legacy/`, séparateurs `\\`), voir
`$LEPTON_BUILD/etape-1/depgraph/resolved-headers.tsv`.
"""


def proposal(comps, edges, sccs):
    lines = []
    w = lines.append
    in_scc = {c: i for i, s in enumerate(sccs) for c in s}
    w("Contraintes : une bibliothèque par composant ; `bin`, `sbin`, `lib/*` hors de `kernel/`. "
      "Les cycles ci-dessus imposent, pour chaque CFC, soit une **fusion**, soit une édition de liens "
      "en groupe (`--start-group … --end-group`, en CMake `LINK_GROUP:RESCAN` ≥ 3.24 ou "
      "`target_link_libraries` circulaire entre bibliothèques STATIC, que CMake répète automatiquement).")
    w("")
    w("| Bibliothèque CMake proposée | Composants | Dépend de (hors cycle) | Contrainte |")
    w("|---|---|---|---|")
    def libname(c):
        n = c.replace(" (logiciel)", "_sw").replace("kernel/", "k_").replace("/", "_").replace("-", "_").replace(".", "")
        return "lepton_" + n
    for c in comps:
        outs = sorted({y for (x, y) in edges if x == c and in_scc.get(y, -1) != in_scc.get(c, -2) and y != c})
        cons = "CFC %d (groupe)" % (in_scc[c] + 1) if c in in_scc else "acyclique"
        w("| `%s` | `%s` | %s | %s |" % (libname(c), c, ", ".join("`%s`" % libname(y) for y in outs) or "—", cons))
    return "\n".join(lines)


def dot(comps, edges, sccs, stats, path):
    colors = ["#fde0dd", "#e0ecf4", "#e5f5e0", "#fff7bc", "#efedf5"]
    in_scc = {c: i for i, s in enumerate(sccs) for c in s}
    group = lambda c: ("commandes" if c in ("bin", "sbin") else "lib" if c.startswith("lib/") else
                       "fs" if c.startswith("kernel/fs") else "net" if c.startswith("kernel/net") or c == "kernel/core/net" else
                       "dev" if c.startswith("kernel/dev") or c == "kernel/usb" or c == "kernel/core/usb" else "core")
    L = ["digraph lepton_components {",
         '  graph [rankdir=LR, splines=true, overlap=false, nodesep=0.35, ranksep=1.1, fontname="Helvetica", label="Lepton : dépendances entre composants (poids = symboles ; rouge = arête dans un cycle ; pointillé = arête issue de l\'extraction textuelle seule ; arêtes < 2 symboles omises)", labelloc=t];',
         '  node [shape=box, style="rounded,filled", fontname="Helvetica", fontsize=11];',
         '  edge [fontname="Helvetica", fontsize=9, color="#555555"];']
    groups = collections.defaultdict(list)
    for c in comps:
        groups[group(c)].append(c)
    for g, cs in sorted(groups.items()):
        L.append('  subgraph "cluster_%s" { label="%s"; style=dashed; color="#999999";' % (g, g))
        for c in cs:
            s = stats[c]
            col = colors[in_scc[c] % len(colors)] if c in in_scc else "#ffffff"
            L.append('    "%s" [label="%s\\n%d/%d .c", fillcolor="%s"%s];' % (
                c, c, s["ok"], s["total"], col, ', penwidth=2, color="#b30000"' if c in in_scc else ""))
        L.append("  }")
    for (x, y), syms in sorted(edges.items()):
        n = len(syms)
        if n < 2:
            continue
        textonly = all(v == "texte" for v in syms.values())
        cyc = x in in_scc and in_scc.get(y) == in_scc[x]
        pw = min(1 + n / 15.0, 6)
        attrs = ['label="%d"' % n, "penwidth=%.1f" % pw]
        if cyc:
            attrs.append('color="#b30000"')
        if textonly:
            attrs.append("style=dashed")
        L.append('  "%s" -> "%s" [%s];' % (x, y, ", ".join(attrs)))
    L.append("}")
    write(path, "\n".join(L) + "\n")


if __name__ == "__main__":
    main()
