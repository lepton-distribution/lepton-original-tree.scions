"""Tests de tools/migration/axes_kernel_core.py (primitives sur fixtures). Lancer :
python3 -m unittest discover -s tools/migration/tests
"""
import contextlib
import io
import os
import sys
import unittest

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), ".."))
import axes_kernel_core as a  # noqa: E402


def run(fn, src, *args):
    with contextlib.redirect_stdout(io.StringIO()):
        out, n = fn(src.splitlines(keepends=True), *args)
    return "".join(out), n


class Primitives(unittest.TestCase):
    def test_premier_bras_garde_inconditionnel(self):
        src = "a\n#if defined(X) || defined(Y)\n   x\n#else\n   y\n#endif\nb\n"
        new, n = run(a.keep_first_arm, src, "defined(X) || defined(Y)", "t")
        self.assertEqual((new, n), ("a\n   x\nb\n", 1))

    def test_premier_bras_absent(self):
        src = "#if defined(Z)\nz\n#endif\n"
        self.assertEqual(run(a.keep_first_arm, src, "defined(X)", "t"), (src, 0))

    def test_remplacement_exact_unique(self):
        self.assertEqual(run(a.replace_exact, "a\nb\nc\n", "b\n", "B\n", "t"), ("a\nB\nc\n", 1))
        self.assertEqual(run(a.replace_exact, "b\nb\n", "b\n", "B\n", "t"), ("b\nb\n", 0))


if __name__ == "__main__":
    unittest.main()
