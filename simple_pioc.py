import glob
from pathlib import Path
import re
import os
import sys
build_flags_tag = "build_flags"
default_env_tag = "[env]"
board_path = "components/"
c_defines_tag = "C_DEFS"
env_syntax = ["[env:","]"]
user_macro_prefix = "PIOC_"
macros = {"${{project_absolute_path}}$" : [os.path.abspath(".")]}
board_folder = ""

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



def set_macros(inputf,line):
    patterns = {}
    pos = inputf.tell()
    while(line and not line.startswith(env_syntax[0])):
        if(line and line.startswith(user_macro_prefix)):
            macro, value = [splitted_line.strip() for splitted_line in line.split("=", 1)]
            patterns[macro] = []
            if(value != ""):
                patterns[macro].append(value) 
            line = inputf.readline()
            while line and re.match(r'[ \t]', line):
                patterns[macro].append(line.strip())
                line = inputf.readline()
            continue
        pos = inputf.tell()
        line = inputf.readline()
    return patterns, pos


def flags_processing(patterns):
    global board_folder
    patterns = [set_env_constant(p,board_folder) for p in patterns]
    glob_patterns, standard_build_flags = categorize_patterns(patterns)
    c_defines = parse_c_defines(board_folder)
    paths = return_paths(glob_patterns)
    paths = ["-I" + path for path in paths]
    lists = []
    lists.extend(paths)
    lists.extend(standard_build_flags)
    lists.extend(c_defines)
    return "\n" + "".join(
    "\t" + item + "\n"
    for item in lists)


def replace_macros(input_file):
    lines = ""
    global macros
    while line := input_file.readline():
        match = next((macro for macro in macros.keys() if macro in line), None)
        if(line.startswith(default_env_tag)):
            macros, pos = set_macros(input_file,line)
            input_file.seek(pos)
            continue
        elif(match != None):
            indent = line[:len(line) - len(line.lstrip())]
            replacement = "\n".join(
                indent + item
                for item in macros[match]
            ) + "\n"
            lines += line.replace(line, replacement) 
            continue
        if line:
            lines += line
    print(lines)
    return lines

def parse_pio_file(input_file_path, result_file_path):
    input_file = Path(input_file_path)
    output_file = Path(result_file_path)
    output_file.parent.mkdir(exist_ok=True, parents=True)
    lines = ""
    with open(input_file, "r") as inputf:
        lines = replace_macros(inputf).splitlines(keepends=True)
    with open(output_file, "w") as outputf:
        outputf.write("")
    with open(output_file, "a") as outputf:
        i = 0
        while i < len(lines):
            line = lines[i]
            i+=1
            if(line.startswith(env_syntax[0])):
                board_folder = line.strip().removeprefix(env_syntax[0]).removesuffix(env_syntax[1])
            elif(line.startswith(build_flags_tag)):
                outputf.write(line)
                patterns = []
                line = lines[i]
                i+=1
                while i < len(lines) and re.match(r'[ \t]', line):
                    patterns.append(line.strip())
                    line = lines[i]
                    i+=1
                flags = flags_processing(patterns)
                outputf.write(flags)
            if line:
                outputf.write(line)


def parse_c_defines(board_folder):
    makefile = None
    defines = []
    try:
        makefile = Path(board_path + board_folder + "/firmware/Makefile")
        if not makefile.is_file():
            raise FileNotFoundError(makefile)
    except FileNotFoundError as e:
        print(f"\033[93mWARNING: Makefile not found: {makefile}\033[0m", file=sys.stderr)
    return defines
    with open(makefile, "r") as inputf:
        while line := inputf.readline():
            if(line.startswith(c_defines_tag)):
                line = inputf.readline().strip().rstrip("\\")
                while line and line[0] == "-":
                    defines.append(line[:-1] if line[-1]=="/" else line)
                    line = inputf.readline().strip().rstrip("\\")
                break
    return defines

def set_abs_path(pioc_file):
    abs_path = os.path.abspath(".")
    pioc_file = pioc_file.replace("${{project_absolute_path}}$", abs_path)
    return pioc_file

def set_env_constant(file,env):
    file = file.replace("${this.__env__}", env);
    return file

if __name__ == "__main__":
    parse_pio_file("platformio.pioc","platformio.ini")
