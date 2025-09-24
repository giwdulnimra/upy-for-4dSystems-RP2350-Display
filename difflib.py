# -*- coding: utf-8 -*-
"""
Created on Wed May 28 10:59:59 2025
@author: armin
"""
import difflib

# Zwei Dateien laden
with open("./micropython/ports/rp2/boards/4DSYS_RP2350/4D Labs_gen4_rp2350_70ct.h", encoding="utf-8") as f1:
    text1 = f1.readlines()

with open("./micropython/ports/rp2/boards/4DSYS_RP2350/4D Systems_gen4_rp2350_70ct.h", encoding="utf-8") as f2:
    text2 = f2.readlines()

# Vergleich
diff = difflib.unified_diff(
    text1, text2,
    fromfile="./micropython/ports/rp2/boards/4DSYS_RP2350/4D Labs_gen4_rp2350_70ct.h",
    tofile="./micropython/ports/rp2/boards/4DSYS_RP2350/4D Systems_gen4_rp2350_70ct.h",
    lineterm=''
)

# Ausgabe
for line in diff:
    print(line)

