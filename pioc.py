import os
import argparse
import ast
from typing import List, Self, Dict
import typing
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
        self.path = path
        self.name = path.split("|")[-1]

    def get_path(self) -> str:
        return self.path

    @abstractmethod
    def get_build_flags(self, board: str = None) -> str:
        pass

    @abstractmethod
    def get_src_filters(self) -> str:
        pass

    @staticmethod
    def get_src_filter_for_type() -> str:
        pass

    @staticmethod
    def get_build_flags_for_type() -> str:
        pass


class PlatformAgnosticLibrary(Library):

    def get_build_flags(self, board: str = None):
        return f"    -I {self.path[2:]}\n"

    def get_build_flags_for_type(self):
        return ""

    def get_src_filters(self):
        return ""

    def get_src_filter_for_type():
        return ""


class PlatformSpecificLibrary(Library):
    def __init__(self, path):
        super().__init__(path)
        self.available_boards = [
            folder for folder in os.listdir(self.path) if folder != "inc"]

    def has_testing_implementation(self):
        return "native" in self.available_boards

    def get_test_build_flags(self):
        return self.get_build_flags("native")

    def get_available_boards(self):
        return self.available_boards

    def get_build_flags(self, board: str):
        return f"    -I {self.path[2:]}/inc\n    -I {self.path[2:]}/{board}\n"

    def get_build_flags_for_type(self):
        return ""

    def get_src_filters(self):
        return ""

    def get_src_filter_for_type():
        return ""


def get_library(lib_path: str) -> Library | None:
    files = os.listdir(lib_path)
    if "inc" in files and not any([file.endswith(".c") for file in files]):
        return PlatformSpecificLibrary(lib_path)
    elif all([file.endswith(".c") or file.endswith(".h") for file in files]):
        return PlatformAgnosticLibrary(lib_path)
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

    def get_firmware_build_flags(self, project_root) -> List[str]:
        paths = []
        for root, dirs, files in os.walk(f"{project_root}/lib/{self.board_name}/firmware"):
            if os.path.isdir(root):
                paths.append(root)
        return [f"    -I {path[2:]}\n" for path in paths]

    def get_build_flags(self, project_root: str) -> List[str]:
        flags = self.get_firmware_build_flags(project_root)
        for lib in get_all_common_libs(project_root):
            if isinstance(lib, PlatformAgnosticLibrary):
                flags.append(lib.get_build_flags())
                continue
            lib: PlatformSpecificLibrary = lib
            if self.board_name not in lib.available_boards:
                print(f"Library warning: board: {
                      self.board_name} not implemented for library: {lib.name}")
                continue
            flags.append(lib.get_build_flags(self.board_name))
        return flags

    def get_test_build_flags(self, project_root: str) -> List[str]:
        flags = []
        for lib in get_all_common_libs(project_root):
            if isinstance(lib, PlatformAgnosticLibrary):
                flags.append(lib.get_build_flags())
                continue
            lib: PlatformSpecificLibrary = lib
            if self.board_name not in lib.available_boards or not lib.has_testing_implementation():
                print(f"Library warning: board: {
                      self.board_name} not implemented for library: {lib.name}")
                continue
            flags.append(lib.get_test_build_flags())
        return flags

    def get_ini(self, project_root: str, dev_opts: str, dev_flag_opts: str, test_opts: str) -> str:
        ret = f"[env:{self.board_name}]\n"
        ret += f"board = {self.board_type}\n"
        ret += dev_opts
        if self.custom_board_settings is not None:
            ret += PlatfmormioOptions.get_header_content(
                self.custom_board_settings)

        ret += f"build_flags=\n{''.join(
            self.get_build_flags(project_root))}\n{dev_flag_opts}\n"
        ret += f"\n\n[env:test_{self.board_name}]\n"
        ret += test_opts

        ret += f"build_flags=\n{''.join(
            self.get_test_build_flags(project_root))}\n\n"
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
            common_build_flags = "\n    " + "\n    ".join(common_build_flags)
        else:
            common_build_flags = ""
        for board in self.boards:

            ret += board.get_ini(self.root_path, PlatfmormioOptions.get_header_content(self.platformio_options.custom_per_env_settings), common_build_flags,
                                 PlatfmormioOptions.get_header_content(self.testing_options))
        return ret


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
    with open(input, "r") as file:
        data = file.read()

    config = Configuration.parse_entry(data)
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
        default='./config.json'
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
