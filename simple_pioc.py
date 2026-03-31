import glob
from pathlib import Path
import re
import os
import sys
build_flags_tag = "build_flags"
board_path = "components/"
c_defines_tag = "C_DEFS"
env_syntax = ["[env:","]"]
def remove_slash(path):
    return path[:-1] if path[-1] in "/\\" else path 

def return_paths(glob_patterns):
    added_path_lists = []
    print(glob_patterns)
    for glob_pattern in glob_patterns:
        paths = [remove_slash(f).replace("\\","/") for f in glob.glob(glob_pattern[1:],recursive=True) if os.path.isdir(f)]
        if glob_pattern[0] == "+":
            added_path_lists.extend(paths)
        else:
            paths_set = set(paths)
            added_path_lists = [x for x in added_path_lists if x not in paths_set]         
    print(added_path_lists)
    return added_path_lists
     

def categorize_patterns(patterns):
    glob_patterns = []
    standard_build_flags = []
    for pattern in patterns:
        if pattern[:2] == "-<":
            glob_patterns.append("-" + pattern[2:-1])
        elif pattern[:2] == "+<":
            glob_patterns.append("+" + pattern[2:-1]) 
        else:
            standard_build_flags.append(pattern)
    return glob_patterns, standard_build_flags

def parse_pio_file(input_file_path, result_file_path):
    input_file = Path(input_file_path)

    output_file = Path(result_file_path)
    output_file.parent.mkdir(exist_ok=True, parents=True)
    board_file = ""
    with open(output_file, "w") as outputf:
        outputf.write("")
    with open(input_file, "r") as inputf: 
        with open(output_file, "a") as outputf:
            while line := inputf.readline():
                if(line.startswith(env_syntax[0])):
                    board_folder = line.strip().removeprefix(env_syntax[0]).removesuffix(env_syntax[1])
                if(line.startswith(build_flags_tag)):
                    outputf.write(line)
                    patterns = []
                    line = inputf.readline()
                    while line and re.match(r'[ \t]', line):
                        patterns.append(line.strip())
                        line = inputf.readline()

                    glob_patterns, standard_build_flags = categorize_patterns(patterns)
                    paths = return_paths(glob_patterns)
                    c_defines = parse_c_defines(board_folder)
                    for path in paths:
                        outputf.write("\t-I " + path + "\n")
                    for flag in standard_build_flags:
                        outputf.write("\t"+flag+"\n")
                    for c_define in c_defines:
                        outputf.write("\t" + c_define + "\n")
                if line:
                    outputf.write(line)

def parse_c_defines(board_folder):
    makefile = Path(board_path + board_folder + "/firmware/Makefile")
    defines = []
    with open(makefile, "r") as inputf:
        while line := inputf.readline():
            if(line.startswith(c_defines_tag)):
                line = inputf.readline().strip().rstrip("\\")
                while line and line[0] == "-":
                    defines.append(line[:-1] if line[-1]=="/" else line)
                    line = inputf.readline().strip().rstrip("\\")
                break
    return defines

if __name__ == "__main__":
    parse_pio_file("platformio.pioc","platformio.ini")
