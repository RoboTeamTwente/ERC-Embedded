import os
import yaml
import argparse
from typing import List, Self, Dict
from pydantic import BaseModel, ValidationError
from abc import ABC, abstractmethod
from dataclasses import dataclass
import sys
from pathlib import Path
import json


def to_dict(s: str) -> Dict:
    json_acceptable_string = s.replace("'", "\"")
    return json.loads(json_acceptable_string)


COMMON_SRC_FILTERS = """
    -<src/>
    +<lib/common/*>
    -<lib/common/*/*>
    +<lib/common/*/native/*>
    """


class Library(ABC):
    # self.path assumed to be in the format PLATFORMIO_ROOT:
    # ./lib/common/{name}/
    def __init__(self, path):
        self.is_testable = True
        self.path = path
        self.name = path.split("|")[-1]

    def get_path(self) -> str:
        return self.path

    @abstractmethod
    def get_build_flags(self, board: str = None) -> str:
        pass

    @abstractmethod
    def get_test_build_flags(self, board: str = None) -> str:
        pass


class CommonLibrary(Library):
    def get_build_flags(self, board: str = None):
        return f"    -I {self.path[2:]}\n"

    def get_test_build_flags(self, board: str = None):
        return self.get_build_flags(board)


class CommonPlatformDependentLibrary(Library):
    def __init__(self, path):
        super().__init__(path)
        self.available_boards = [
            folder for folder in os.listdir(self.path) if folder != "inc"]
        self.is_testable = "test" in self.available_boards

    def get_test_build_flags(self, board: str = None):
        return self.get_build_flags("test")

    def get_available_boards(self):
        return self.available_boards

    def get_build_flags(self, board: str):
        return f"    -I {self.path[2:]}/inc\n    -I {self.path[2:]}/{board}\n"


class BoardPlatformDependentLibrary(Library):
    def __init__(self, path):
        super().__init__(path)
        self.is_testable = "test" in os.listdir(path)

    def get_test_build_flags(self, board: str = None):
        return f"    -I {self.path}/test\n    -I {self.path[2:]}/inc" if self.is_testable else ""

    def get_build_flags(self, board: str = None):
        return f"    -I {self.path}/src\n    -I {self.path[2:]}/inc" if self.is_testable else ""


class BoardLibrary(Library):
    def get_build_flags(self, board: str = None):
        return f"    -I {self.path[2:]}\n"

    def get_test_build_flags(self, board: str = None):
        return self.get_build_flags(board)


def get_library(lib_path: str) -> Library | None:
    files = os.listdir(lib_path)
    if "inc" in files and not any([file.endswith(".c") for file in files]):
        return CommonPlatformDependentLibrary(lib_path)
    elif all([file.endswith(".c") or file.endswith(".h") for file in files]):
        return CommonLibrary(lib_path)
    return None


def get_all_common_libs(root_path: str) -> List[Library]:
    root_path = root_path if root_path[-1] != '/' else root_path[:-1]
    lib_path = f"{root_path}/lib/common"
    libraries = []

    for lib in os.listdir(lib_path):
        library = get_library(f"{lib_path}/{lib}")
        if library is None:
            continue
        libraries.append(library)
    return libraries


def get_non_common_libs(root_path: str, board: str) -> List[Library]:
    libs = []
    root_path = root_path if root_path[-1] != '/' else root_path[:-1]
    for lib in os.listdir(f"{root_path}/lib/{board}"):
        path = f"{root_path}/lib/{board}/{lib}"
        if "inc" in os.listdir(path):
            library = BoardPlatformDependentLibrary(path)
        else:
            library = BoardLibrary(path)
        libs.append(library)
    return libs


class ConfigEntry(BaseModel):
    @classmethod
    def parse_entry(cls, data: str) -> Self | None:
        """
        Safely parses a JSON string into an instance of this class.
        Returns None on failure.
        """
        try:
            # 3. Use 'cls', which will be 'Board' when you call Board.parse_entry
            return cls.model_validate_json(data)

        # Be specific with your exceptions
        except (ValidationError, json.JSONDecodeError) as e:
            print(f"--- FAILED TO PARSE ---")
            print(f"Config entry: {cls.__name__}")
            print(f"Input: {data}")
            print(f"Error: {e}")
            print("-----------------------")


@dataclass
class Board(ConfigEntry):
    """Configuration for a single board."""
    board_name: str
    board_type: str
    custom_board_settings: Dict | None = None

    def validate_board_firmware(self, project_root: str) -> bool:
        return os.path.isdir(f"{project_root}/lib/{self.board}/firmware")

    def validate_board_src(self, project_root: str) -> bool:
        return os.path.isdir(f"{project_root}/src/{self.board}")

    def get_all_libraries(self, project_root: str) -> List[Library]:
        libraries = []
        libraries.extend(get_all_common_libs(project_root))
        libraries = [lib for lib in libraries if not (isinstance(
            lib, CommonPlatformDependentLibrary) and not self.board_name in lib.available_boards)]
        libraries.extend(get_non_common_libs(project_root, self.board_name))
        return libraries

    def get_all_testable_libraries(self, project_root: str) -> List[Library]:
        libraries = self.get_all_libraries(project_root)
        return [lib for lib in libraries if lib.is_testable]

    def get_test_buld_flag_libs(self, project_root):
        paths = []
        for root, dirs, files in os.walk(f"{project_root}/lib/{self.board_name}"):
            if root.endswith("firmware"):
                continue
            if os.path.isdir(root):
                paths.append(root)

        return [f"    -I {path[2:]}\n" for path in paths]

    def get_firmware_build_flags(self, project_root) -> List[str]:
        paths = []
        for root, dirs, files in os.walk(f"{project_root}/lib/{self.board_name}"):
            if os.path.isdir(root):
                paths.append(root)
        return [f"    -I {path[2:]}\n" for path in paths]

    def get_build_flags(self, project_root: str) -> List[str]:
        flags = self.get_firmware_build_flags(project_root)

        libraries = self.get_all_libraries(project_root)
        flags.extend([lib.get_build_flags(self.board_name)
                     for lib in libraries])
        return flags

    def get_test_build_flags(self, project_root: str) -> List[str]:
        return [lib.get_test_build_flags() for lib in self.get_all_testable_libraries(project_root)]

    def get_ini(self, project_root: str, dev_opts: str, dev_flag_opts: str, test_opts: str) -> str:
        ret = f"[env:{self.board_name}]\n"
        ret += f"board = {self.board_type}\n"
        ret += dev_opts
        ret += f"build_flags=\n{''.join(self.get_build_flags(project_root))}"
        ret += dev_flag_opts

        ret += f"\n\n[env:test_{self.board_name}]\n"
        ret += test_opts
        ret += f"build_flags=\n{''.join(self.get_test_build_flags(project_root))}"
        return ret


@dataclass
class Debug(ConfigEntry):
    """Debug-specific settings."""
    baud_rate: int | None = None
    com_port: str | None = None
    testing_settings: Dict


@dataclass
class PlatfmormioOptions(ConfigEntry):
    """PlatformIO-specific overrides."""
    custom_platformio_settings: Dict
    custom_env_settings: Dict
    custom_extras_settings: Dict
    custom_per_env_settings: Dict

    @staticmethod
    def get_header_content(content: Dict) -> str:
        ret = ""
        for key, value in content.items():
            if isinstance(value, list):
                value = "\n    ".join(value)
            ret += f"{key} = {value}\n"

        return ret

    def __str__(self) -> str:
        ret = "[platformio]\n"
        ret += PlatfmormioOptions.get_header_content(
            self.custom_platformio_settings)
        ret += "\n\n[extra]\n"
        ret += PlatfmormioOptions.get_header_content(
            self.custom_extras_settings)
        ret += "\n\n[env]\n"
        ret += PlatfmormioOptions.get_header_content(
            self.custom_env_settings)

        return ret


class Configuration(ConfigEntry):
    """
    Main configuration class that loads all settings from a
    specified TOML file at initialization.
    """

    # Define the top-level fields
    boards: List[Board]
    testing_options: Dict
    platformio_options: PlatfmormioOptions
    root_path: Path = "."

    def set_root_path(self, path: Path):
        self.root_path = path

    def __str__(self) -> str:
        ret = str(self.platformio_options)
        common_build_flags = self.platformio_options.custom_extras_settings.get(
            "common_build_flags")
        if common_build_flags is not None:
            common_build_flags = "    " + "\n    ".join(common_build_flags)
        else:
            common_build_flags = ""
        testing_lib_extra_dirs = []
        if "lib_extra_dirs" in self.testing_options.keys():
            testing_lib_extra_dirs = self.testing_options["lib_extra_dirs"]
        per_env_lib_deps = []
        if "lib_deps" in self.platformio_options.custom_per_env_settings.keys():
            per_env_lib_deps = self.platformio_options.custom_per_env_settings["lib_deps"]
        for board in self.boards:
            self.testing_options["lib_extra_dirs"] = testing_lib_extra_dirs.copy(
            )
            self.testing_options["lib_extra_dirs"].append(
                f"lib/{board.board_name}")
            self.testing_options["test_filter"] = f"{board.board_name}/*"
            self.platformio_options.custom_per_env_settings["lib_deps"] = per_env_lib_deps.copy(
            )
            self.platformio_options.custom_per_env_settings["lib_deps"].extend(
                [f"{lib.path[2:]}" for lib in board.get_all_libraries(self.root_path) if isinstance(lib, (BoardLibrary, BoardPlatformDependentLibrary))])

            ret += board.get_ini(self.root_path, PlatfmormioOptions.get_header_content(self.platformio_options.custom_per_env_settings), common_build_flags,
                                 PlatfmormioOptions.get_header_content(self.testing_options))
        return ret


def compile_nanopb_files(project_root: Path, boards: List[Board]):
    proto_path = f"{project_root}/proto"
    for board in boards:
        component_paths = [
            f"{proto_path}/components/common/{component}" for component in os.listdir(f"{proto_path}/components/common")]
        if os.path.isdir(f"{proto_path}/components/{board.board_name}"):
            component_paths.extend(
                [f"{proto_path}/components/{board.board_name}/{component}" for component in os.listdir(
                    f"{proto_path}/components/{board.board_name}")]
            )

        resulting_proto_buffer = f"syntax = \"proto3\";\n"
        for component in component_paths:
            with open(component, "r") as f:
                resulting_proto_buffer += f.read()
        with open(f"{proto_path}/{board.board_name}.proto", "w") as f:
            f.write(resulting_proto_buffer)


def execute_test(env: str | None = None):
    # TODO: Implement
    pass


def execute_clean(env: str | None = None, do_all: bool = False):
    # TODO: Implement
    pass


def execute_run(env: str | None = None, upload: bool = False, monitor: bool = False):
    command = "pio run "
    if env is not None:
        command += f"-e {env} "
    if upload:
        command += "-t upload "
    if monitor:
        command += "&& pio device monitor"
    print(command)


def execute_compile(input: str, output: str):
    with open(input) as file:
        data = yaml.safe_load(file)

    config = Configuration.model_validate(data)
    compile_nanopb_files(".", config.boards)
    # TODO: config should be specialized to a specific output dir,
    with open(output, "w") as file:
        file.write(str(config))


def main():
    # 1. Create the main parser
    parser = argparse.ArgumentParser(
        description="A command-line script for managing project tasks."
    )

    # 2. Add subparsers
    # Subparsers are used to create distinct commands (e.g., 'run', 'test').
    # 'dest='command'' stores which subcommand was used.
    # 'required=True' ensures that one of the commands must be provided.
    subparsers = parser.add_subparsers(
        dest='command',
        required=True,
        help="The main command to execute."
    )

    # 3. Create the 'run' command parser
    run_parser = subparsers.add_parser(
        'run',
        help='Platformio Build'
    )
    run_parser.add_argument(
        '-e', '--env',
        help='Specify the environment/board (e.g., "main_board", "driving_board").',
        default=None  # Set a default environment
    )
    run_parser.add_argument(
        '-u', '--upload',
        action='store_true',  # This stores 'True' if the flag is present
        help='Wether to upload the built bin to the microcontroller'
    )
    run_parser.add_argument(
        '-m', '--monitor',
        action='store_true',
        help='Wether to run the serial monitor after built (should be run with -u)'
    )

    # 4. Create the 'test' command parser
    test_parser = subparsers.add_parser(
        'test',
        help='Execute the unit tests.'
    )
    test_parser.add_argument(
        '-e', '--env',
        help='Specify the test environment/board.',
        default=None  # Set a default environment for testing
    )

    # 5. Create the 'clean' command parser
    clean_parser = subparsers.add_parser(
        'clean',
        help='Clean build artifacts and temporary files.'
    )
    clean_parser.add_argument(
        '-e', '--env',
        help='Optional: specify an environment/board to target cleaning.'
    )
    clean_parser.add_argument(
        '-a', '--all',
        action='store_true',
        help='Clean all environments and cache.'
    )

    # 6. Create the 'compile'/'build' command parser
    # Use 'aliases' to allow 'build' to function as 'compile'.
    compile_parser = subparsers.add_parser(
        'compile',
        help='Compile the platformio.ini file.',
        aliases=['build']
    )
    compile_parser.add_argument(
        '-i', '--input',
        help='Specify the input config json file.',
        default='./config.yaml'
    )
    compile_parser.add_argument(
        '-o', '--output',
        help='Specify the outpput ini file.',
        default='./platformio.ini'
    )

    try:
        args = parser.parse_args()
        if args.command == "run":
            execute_run(args)
        elif args.command == "test":
            execute_test(args)
        elif args.command == "compile":
            execute_compile(args.input, args.output)
        elif args.command == "clean":
            execute_clean(**args)
        else:
            args.print_help()
    except argparse.ArgumentError as e:
        print(f"Error: {e}", file=sys.stderr)
        parser.print_help()
        sys.exit(1)


if __name__ == "__main__":
    main()
