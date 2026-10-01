"""Tests de tools/migration/kal_split.py : retrait de bras sur fixtures, et rejeu complet de la
décomposition sur les copies d'origine (scion/legacy/) comparé à l'arbre versionné. Lancer :
python3 -m unittest discover -s tools/migration/tests
"""
import contextlib
import io
import os
import shutil
import sys
import tempfile
import unittest

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, ".."))
import kal_split as k  # noqa: E402

CLONE = os.path.abspath(os.path.join(HERE, "..", "..", ".."))


def lines(src):
    return src.splitlines(keepends=True)


def remove(src, pred):
    with contextlib.redirect_stdout(io.StringIO()):
        out, n = k.remove_all(lines(src), pred)
    return "".join(out), n


class RetraitDeBras(unittest.TestCase):
    def test_premier_bras_elif_devient_if(self):
        src = "#if defined(A)\na\n#elif defined(B)\nb\n#else\nc\n#endif\n"
        new, n = remove(src, lambda c, g, i: c == "defined(A)")
        self.assertEqual(new, "#if defined(B)\nb\n#else\nc\n#endif\n")
        self.assertEqual(n, 1)

    def test_bras_du_milieu(self):
        src = "#if defined(A)\na\n#elif defined(B)\nb\n#else\nc\n#endif\n"
        new, _ = remove(src, lambda c, g, i: c == "defined(B)")
        self.assertEqual(new, "#if defined(A)\na\n#else\nc\n#endif\n")

    def test_premier_bras_puis_else_inconditionnel(self):
        src = "x\n#ifdef D\nd\n#else\ne\n#endif\ny\n"
        new, _ = remove(src, lambda c, g, i: i == 0 and c == "D")
        self.assertEqual(new, "x\ne\ny\n")

    def test_groupe_a_un_bras_retire_avec_continuation(self):
        src = "a\n#if (X == 1)\\\n  || (X == 2)\nz\n#endif\nb\n"
        new, _ = remove(src, lambda c, g, i: "X == 2" in c)
        self.assertEqual(new, "a\nb\n")


class Rejeu(unittest.TestCase):
    """Rejoue gel → dispatcher sur legacy/ ; compare aux fichiers non retouchés depuis."""

    # fichiers dont le contenu versionné est exactement la sortie du script ; arch/armv7m et
    # backend/embos ont reçu ensuite des commits sémantiques (primitives d'arch)
    IDENTIQUES = ["kal.h", "kal/contrat.h", "kal/backend/freertos/kal_backend.h",
                  "kal/backend/static/kal_backend.h", "kal/arch/host/kal_arch.h"]

    def test_rejeu_complet(self):
        legacy = os.path.join(CLONE, "scion", "legacy", k.CORE)
        if not os.path.exists(os.path.join(legacy, "kal.h")):
            self.skipTest("copies d'origine absentes")
        with tempfile.TemporaryDirectory() as tmp:
            core = os.path.join(tmp, "scion", k.CORE)
            os.makedirs(core)
            for f in ("kal.h", "kal.c"):
                shutil.copy2(os.path.join(legacy, f), core)
            with contextlib.redirect_stdout(io.StringIO()):
                k.step_gel(tmp, False)
                k.step_anciens(tmp, False)
                for step in ("extraire-contrat", "extraire-freertos", "extraire-static",
                             "extraire-embos", "dispatcher"):
                    k.step_extract(step, tmp, False)
            self.assertFalse(os.path.exists(os.path.join(core, "kal.c")))
            for rel in self.IDENTIQUES:
                with self.subTest(rel=rel):
                    got = open(os.path.join(core, rel), encoding="utf-8").read()
                    ref = open(os.path.join(CLONE, "scion", k.CORE, rel), encoding="utf-8").read()
                    self.assertEqual(got, ref)
            for rel in ("kal/arch/armv7m/kal_arch.h", "kal/backend/embos/kal_backend.h"):
                self.assertTrue(os.path.exists(os.path.join(core, rel)))


if __name__ == "__main__":
    unittest.main()
