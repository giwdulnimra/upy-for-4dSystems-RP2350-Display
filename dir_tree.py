# -*- coding: utf-8 -*-
"""
Created on Fri Apr  4 15:59:14 2025

@author: armin
"""

import os

def directory_tree(startpath, output_file):
    with open(output_file, 'w', encoding='utf-8') as f:
        for root, dirs, files in os.walk(startpath):
            level = root.replace(startpath, '').count(os.sep)
            indent = ' ' * 4 * level
            f.write('{}{}/\n'.format(indent, os.path.basename(root)))
            subindent = ' ' * 4 * (level + 1)
            for file in files:
                f.write('{}{}\n'.format(subindent, file))

if __name__ == "__main__":
    # Beispiel: Den Ordner angeben, den du auslesen möchtest:
    #start_directory = r"C:\Users\armin\OneDrive\6FS-MT-Jena_BA\Praxismodul\projekt_blinky_c"
    #start_directory =  r"C:\Users\armin\OneDrive\6FS-MT-Jena_BA\Bachelorarbeit\upy_display_export"#"\ws5_export"
    #start_directory = "/home/lenimra/Documents/bachelorarbeit/upy_display_export/ws5_export"
    start_directory = r"C:\Users\armin\OneDrive\6FS-MT-Jena_BA\Bachelorarbeit\upy_display_export\micropython\py"
    
    #output_txt = r"projekt_blinky_c_structure.txt"
    #output_txt = r"C:\Users\armin\OneDrive\6FS-MT-Jena_BA\Bachelorarbeit\upy_display_export\upy_display_export.txt"#"\ws5_structure.txt"
    output_txt = r"C:\Users\armin\Desktop\export_dir.txt"
    
    directory_tree(start_directory, output_txt)
    print("Verzeichnisstruktur wurde in '{}' gespeichert.".format(output_txt))
