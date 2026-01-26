#! /usr/bin/env bash

OS_TYPE=$(uname -s)
MV_CMD="mv"

# Rename files
if [ -f "components/driving_board/firmware/Core/Src/main.c" ]; then
    mv components/driving_board/firmware/Core/Src/main.c components/driving_board/firmware/Core/Src/cubemx_main.c
    echo "Renamed main.c -> cubemx_main.c"
fi

if [ -f "components/driving_board/firmware/Core/Inc/main.h" ]; then
    mv components/driving_board/firmware/Core/Inc/main.h components/driving_board/firmware/Core/Inc/cubemx_main.h
    echo "Renamed main.h -> cubemx_main.h"
fi

grep -rl '#include "main.h"' components/driving_board/firmware/Core/ | while read -r file; do
    sed -i '' 's/#include "main.h"/#include "cubemx_main.h"/g' "$file"
    echo "Updated include in $file"
done