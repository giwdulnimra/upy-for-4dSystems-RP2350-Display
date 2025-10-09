# -*- coding: utf-8 -*-
"""
Created on Wed May 28 10:59:59 2025
@author: armin
"""
import difflib
filepath1 = "/home/lenimra/Documents/bachelorarbeit/cgpt_graphics4d_wrapper.cpp"
filepath2 = "/home/lenimra/Documents/bachelorarbeit/gemini_graphics4d_wrapper.cpp"
output_path = "./output.diff"

# Zwei Dateien laden
with open(filepath1, encoding="utf-8") as f1:
    text1 = f1.readlines()

with open(filepath2, encoding="utf-8") as f2:
    text2 = f2.readlines()

# Vergleich
diff = difflib.unified_diff(
    text1, text2,
    fromfile=filepath1,
    tofile = filepath2,
    lineterm=''
)

# Ausgabe
with open(output_path, "w", encoding="utf-8") as out_file:
    for line in diff:
        print(line)
        out_file.write(line + "\n")
