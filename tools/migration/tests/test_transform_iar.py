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
        self.assertEqual((new, len(auto), len(resid)), (src, 0, 2))


if __name__ == "__main__":
    unittest.main()
