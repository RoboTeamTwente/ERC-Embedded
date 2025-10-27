
OS_TYPE=$(uname -s)
MV_CMD="mv"

# Rename files
if [ -f "firmware/Core/Src/main.c" ]; then
    mv firmware/Core/Src/main.c firmware/Core/Src/cubemx_main.c
    echo "Renamed main.c -> cubemx_main.c"
fi

if [ -f "firmware/Core/Inc/main.h" ]; then
    mv firmware/Core/Inc/main.h firmware/Core/Inc/cubemx_main.h
    echo "Renamed main.h -> cubemx_main.h"
fi

grep -rl '#include "main.h"' firmware/Core/ | while read -r file; do
    sed -i 's/#include "main.h"/#include "cubemx_main.h"/g' "$file"
    echo "Updated include in $file"
done
