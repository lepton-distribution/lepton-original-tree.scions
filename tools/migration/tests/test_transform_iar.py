"""Tests de tools/migration/transform_iar.py (règles sur fixtures). Lancer :
python3 -m unittest discover -s tools/migration/tests
"""
import os
import sys
import unittest

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), ".."))
import transform_iar as t  # noqa: E402


def garde(src):
    new, auto, resid = t.rule_garde_iar_arm(src, "test.c")
    return new, len(auto), len(resid)


class GardeIarArm(unittest.TestCase):
    def test_premiere_branche_iar_retiree(self):
        src = ("#if (__tauon_compiler__==__compiler_iar_arm__)\nA\n"
               "#elif (__tauon_compiler__==__compiler_gnuc__)\nB\n#endif\n")
        new, a, r = garde(src)
        self.assertEqual(new, "#if (__tauon_compiler__==__compiler_gnuc__)\nB\n#endif\n")
        self.assertEqual((a, r), (1, 0))

    def test_branche_iar_du_milieu(self):
        src = "#if defined(X)\nA\n#elif defined(__ICCARM__)\nB\n#else\nC\n#endif\n"
        new, a, r = garde(src)
        self.assertEqual(new, "#if defined(X)\nA\n#else\nC\n#endif\n")

    def test_garde_toujours_vraie_levee(self):
        src = "x\n#if (__tauon_compiler__!=__compiler_iar_arm__)\nA\n#else\nB\n#endif\ny\n"
        new, a, r = garde(src)
        self.assertEqual(new, "x\nA\ny\n")

    def test_toujours_vraie_apres_branche_inconnue(self):
        src = "#if defined(X)\nA\n#elif !defined(__ICCARM__)\nB\n#elif defined(Y)\nC\n#endif\n"
        new, a, r = garde(src)
        self.assertEqual(new, "#if defined(X)\nA\n#else\nB\n#endif\n")

    def test_ifdef_et_else_pris(self):
        src = "   #ifdef __ICCARM__\nA\n   #else\nB\n   #endif\n"
        new, a, r = garde(src)
        self.assertEqual(new, "B\n")

    def test_comparaison_macro_iar(self):
        src = "#if __IAR_SYSTEMS_ICC__> 1\nA\n#else\nB\n#endif\n"
        self.assertEqual(garde(src)[0], "B\n")

    def test_condition_simplifiee(self):
        src = "#if defined(__IAR_SYSTEMS_ICC__) || defined(__ARMCC_VERSION)\nA\n#endif\n"
        new, a, r = garde(src)
        self.assertEqual(new, "#if defined(__ARMCC_VERSION)\nA\n#endif\n")

    def test_et_avec_iar_faux(self):
        src = ("#if defined(X)\nA\n#elif ( (__tauon_compiler__==__compiler_iar_arm__) \\\n"
               "      && (__tauon_cpu_core__ == 3))\nB\n#endif\n")
        self.assertEqual(garde(src)[0], "#if defined(X)\nA\n#endif\n")

    def test_m16c_residuel_inchange(self):
        src = "#if (__tauon_compiler__==__compiler_iar_m16c__)\nA\n#endif\n"
        new, a, r = garde(src)
        self.assertEqual((new, a, r), (src, 0, 1))

    def test_pragma_residuel_inchange(self):
        src = ("#if (__tauon_compiler__==__compiler_iar_arm__)\n"
               "#define S _Pragma(\"location=\\\"X\\\"\")\n#endif\n")
        new, a, r = garde(src)
        self.assertEqual((new, a, r), (src, 0, 1))

    def test_imbrication(self):
        src = ("#if defined(X)\n#if defined(__ICCARM__)\nA\n#else\nB\n#endif\n"
               "#elif defined(__IAR_SYSTEMS_ICC__)\nC\n#endif\n")
        self.assertEqual(garde(src)[0], "#if defined(X)\nB\n#endif\n")

    def test_tout_retire(self):
        src = "a\n#if defined(__ICCARM__)\nA\n#elif defined(__IAR_SYSTEMS_ICC__)\nB\n#endif\nb\n"
        self.assertEqual(garde(src)[0], "a\nb\n")

    def test_sans_iar_identique(self):
        src = "#ifndef H\n#define H\n#if A && (B || !C)\nx\n#elif D\ny\n#endif\n#endif\n"
        self.assertEqual(garde(src), (src, 0, 0))


class IntrinsicsCmsis(unittest.TestCase):
    def test_table(self):
        src = ("  __disable_interrupt();\n  s = __get_interrupt_state();\n"
               "  __set_interrupt_state(s); __no_operation();\n  __enable_interrupt ();\n")
        new, auto, resid = t.rule_intrinsics_cmsis(src, "test.c")
        self.assertEqual(new, "  __disable_irq();\n  s = __get_PRIMASK();\n"
                              "  __set_PRIMASK(s); __NOP();\n  __enable_irq ();\n")
        self.assertEqual((len(auto), len(resid)), (5, 0))

    def test_residuels(self):
        src = "p = __sfe(\"CSTACK\");\n#include <intrinsics.h>\n"
        new, auto, resid = t.rule_intrinsics_cmsis(src, "test.c")
        self.assertEqual((new, len(auto), len(resid)), (src, 0, 1))


class GardeAlignement(unittest.TestCase):
    def test_data_alignment_double_par_gcc(self):
        src = ("#if defined ( __ICCARM__ )\n  #pragma data_alignment=4\n#endif\n"
               "__ALIGN_BEGIN static int t[4] __ALIGN_END;\n")
        new, a, r = garde(src)
        self.assertEqual((new, a, r), ("__ALIGN_BEGIN static int t[4] __ALIGN_END;\n", 1, 0))

    def test_data_alignment_sans_gcc_residuel(self):
        src = "#if defined ( __ICCARM__ )\n  #pragma data_alignment=4\n#endif\nstatic int t[4];\n"
        new, a, r = garde(src)
        self.assertEqual((new, a, r), (src, 0, 1))


class GardeIarGelee(unittest.TestCase):
    def test_m16c_retire(self):
        src = ("#if (__tauon_compiler__==__compiler_iar_m16c__)\nA\n"
               "#elif defined(__IAR_SYSTEMS_ICC)\nB\n#else\nC\n#endif\n")
        new, auto, resid = t.rule_garde_iar_gelee(src, "test.c")
        self.assertEqual((new, len(resid)), ("C\n", 0))

    def test_arm_non_touche(self):
        src = "#if defined(__ICCARM__)\nA\n#endif\n"
        self.assertEqual(t.rule_garde_iar_gelee(src, "test.c"), (src, [], []))

    def test_pragma_m16c_retire(self):
        src = "#if (__tauon_compiler__==__compiler_iar_m16c__)\n#pragma memory=far\n#endif\nx\n"
        self.assertEqual(t.rule_garde_iar_gelee(src, "test.c")[0], "x\n")


class MotCleIar(unittest.TestCase):
    def test_packed_struct_et_include(self):
        src = ('#include "a.h"\ntypedef __packed union {\n  __packed struct { int a:8; } b;\n} u;\n'
               "/* __packed */ char *s = \"__packed\";\n")
        new, auto, resid = t.rule_mot_cle_iar(src, "test.c")
        self.assertEqual(new, '#include "a.h"\n#include "kernel/core/compiler.h"\n'
                              "typedef union __lepton_packed {\n"
                              "  struct __lepton_packed { int a:8; } b;\n} u;\n"
                              "/* __packed */ char *s = \"__packed\";\n")
        self.assertEqual(len(resid), 0)

    def test_include_apres_garde_entete(self):
        src = "/* x */\n#ifndef X_H\n#define X_H\n__no_init int v;\n#endif\n"
        new, auto, resid = t.rule_mot_cle_iar(src, "x.h")
        self.assertEqual(new, "/* x */\n#ifndef X_H\n#define X_H\n"
                              '#include "kernel/core/compiler.h"\n__lepton_no_init int v;\n#endif\n')

    def test_residuels(self):
        src = ('#include "kernel/core/compiler.h"\n#define P __packed\n__packed int *p;\n'
               "__root const int k = 1;\n")
        new, auto, resid = t.rule_mot_cle_iar(src, "test.c")
        self.assertEqual(new, '#include "kernel/core/compiler.h"\n#define P __packed\n'
                              "__packed int *p;\n__lepton_used const int k = 1;\n")
        self.assertEqual((len(auto), len(resid)), (1, 2))

    def test_sous_garde_iar_ignore(self):
        src = "#if defined(__ICCARM__)\n__packed struct s;\n#endif\n"
        self.assertEqual(t.rule_mot_cle_iar(src, "t.c"), (src, [], []))


class PragmaIar(unittest.TestCase):
    def test_retires_et_residuels(self):
        src = ("#pragma optimize=none\n#pragma diag_suppress=Pe177\nint a;\n"
               "#pragma location=\"X\"\nint b;\n#pragma pack(1)\n")
        new, auto, resid = t.rule_pragma_iar(src, "t.c")
        self.assertEqual(new, "int a;\n#pragma location=\"X\"\nint b;\n#pragma pack(1)\n")
        self.assertEqual((len(auto), len(resid)), (2, 1))

    def test_location_operateur(self):
        src = '#include "a.h"\n#define R _Pragma("location = \\"ZZZ_INCONNUE\\"")\n'
        new, auto, resid = t.rule_pragma_iar(src, "t.c")
        self.assertEqual(new, '#include "a.h"\n#include "kernel/core/compiler.h"\n'
                              '#define R __lepton_section("ZZZ_INCONNUE")\n')
        self.assertEqual(len(resid), 1)          # section absente des scripts .ld


class Signalements(unittest.TestCase):
    def test_header_et_symbole(self):
        src = "#include <yvals.h>\n#include <stdio.h>\nx = __iar_dlmalloc(4); // __iar_x\n"
        self.assertEqual(len(t.rule_header_iar(src, "t.c")[2]), 1)
        self.assertEqual(len(t.rule_symbole_iar(src, "t.c")[2]), 1)
        self.assertEqual(t.rule_symbole_iar(src, "t.c")[0], src)

    def test_tiers(self):
        self.assertTrue(t.est_tiers("sys/root/src/kernel/fs/fatfs/core/diskio.c"))
        self.assertFalse(t.est_tiers("sys/root/src/kernel/fs/ufs/ufscore.c"))


if __name__ == "__main__":
    unittest.main()
