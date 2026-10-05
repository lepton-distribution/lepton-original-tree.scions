#!/usr/bin/env python3
"""hal_conf_stm32f7.py — dérive le stm32f7xx_hal_conf.h d'une carte du modèle livré avec la HAL F7

Usage (racine du clone) :
  python3 tools/migration/hal_conf_stm32f7.py <bsp> <HSE_Hz> <MODULE>...
  ex. : python3 tools/migration/hal_conf_stm32f7.py stm32f746g_disco 25000000 GPIO RCC CORTEX PWR FLASH

Copie `hal_driver/Inc/stm32f7xx_hal_conf_template.h` (STM32CubeF7, non modifié) vers
`kernel/dev/bsp/<bsp>/stm32f7xx_hal_conf.h` : seuls les modules nommés restent activés
(HAL_MODULE_ENABLED toujours), les autres `#define HAL_<X>_MODULE_ENABLED` sont mis en
commentaire ; valeur de HSE_VALUE remplacée. Rejouable (le résultat ne dépend que du modèle et des
arguments).
"""
import pathlib
import re
import sys

CLONE = pathlib.Path(__file__).resolve().parents[2]
SRC = CLONE / "scion/sys/root/src/kernel"
TEMPLATE = SRC / "dev/arch/cortexm/stm32f7xx/hal_driver/Inc/stm32f7xx_hal_conf_template.h"


def main():
    if len(sys.argv) < 4:
        sys.exit(__doc__)
    bsp, hse, modules = sys.argv[1], int(sys.argv[2]), {m.upper() for m in sys.argv[3:]}
    text = TEMPLATE.read_text(encoding="ascii")
    seen = set()

    def module(m):
        name = m.group(2)
        seen.add(name)
        if name in modules:
            return m.group(0)
        return "/* %s */" % m.group(1).rstrip()

    text = re.sub(r"^(#define HAL_([A-Z0-9]+)_MODULE_ENABLED\s*)$", module, text, flags=re.M)
    unknown = modules - seen
    if unknown:
        sys.exit("modules absents du modèle : " + ", ".join(sorted(unknown)))
    text, n = re.subn(r"(#define HSE_VALUE\s+)\S+U", r"\g<1>%dU" % hse, text, count=1)
    if n != 1:
        sys.exit("HSE_VALUE introuvable dans le modèle")
    head = ("/* Lepton : généré par tools/migration/hal_conf_stm32f7.py depuis\n"
            " * hal_driver/Inc/stm32f7xx_hal_conf_template.h (STM32CubeF7 v1.17.4, HAL V1.3.3) ;\n"
            " * modules : %s ; HSE_VALUE %d. Ne pas éditer : rejouer le script. */\n"
            % (", ".join(sorted(modules)), hse))
    out = SRC / "dev/bsp" / bsp / "stm32f7xx_hal_conf.h"
    out.parent.mkdir(parents=True, exist_ok=True)
    out.write_text(head + text, encoding="ascii" if head.isascii() else "utf-8")
    print("%s : %d module(s) activé(s)" % (out.relative_to(CLONE), len(modules)))


if __name__ == "__main__":
    main()
