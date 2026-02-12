{pkgs? import <nixpkgs> {}}:
pkgs.mkShell {
  packages = [
    pkgs.python3
      pkgs.python3Packages.grpcio-tools
      pkgs.python3Packages.protobuf
    pkgs.protobuf
      pkgs.platformio
  ];

  shellHook = ''
    export PLATFORMIO_PYTHON="${pkgs.python3}/bin/python3"
  '';
}
