
OS_TYPE=$(uname -s)
MV_CMD="mv"

# Rename files
if [ -f "./Core/Src/main.c" ]; then
    mv ./Core/Src/main.c ./Core/Src/cubemx_main.c
    echo "Renamed main.c -> cubemx_main.c"
fi

if [ -f "./Core/Inc/main.h" ]; then
    mv ./Core/Inc/main.h ./Core/Inc/cubemx_main.h
    echo "Renamed main.h -> cubemx_main.h"
fi

grep -rl '#include "main.h"' ./Core/ | while read -r file; do
    sed -i 's/#include "main.h"/#include "cubemx_main.h"/g' "$file"
    echo "Updated include in $file"
done
