OS_TYPE=$(uname -s)
MV_CMD="mv"
set -euo pipefail
BASE="../components"
# Rename files

find ../components/ -type f | while read -r file; do
  # Extract the basename (filename without path)
  base="$(basename "$file")"

  if [[ "$base" == "main.c" ]]; then
    dir=$(dirname "$file") # Get directory of the file
    mv "$file" "$dir/cubemx_main.c"
    echo "Renamed $file to $dir/cubemx_main.c"
  fi

  if [[ "$base" == "main.h" ]]; then
    dir=$(dirname "$file") # Get directory of the file
    mv "$file" "$dir/cubemx_main.h"
    echo "Renamed $file to $dir/cubemx_main.h"
  fi
done


find "$BASE" -type d -name firmware_definitions | while read -r FW_DIR; do
    BOARD_DIR="$(dirname "$FW_DIR")"
    CUBEMX_FILE="$BOARD_DIR/firmware/Core/Inc/cubemx_main.h"

    # Skip if cubemx file does not exist
    [[ -f "$CUBEMX_FILE" ]] || continue

    echo "Prepending firmware_definitions to: $CUBEMX_FILE"

    TMP_FILE="$(mktemp)"

    # 1) Add firmware_definitions content first
    find "$FW_DIR" -type f -exec cat {} \; >> "$TMP_FILE"

    # Optional separator (recommended)
    echo -e "\n/* ---- END firmware_definitions ---- */\n" >> "$TMP_FILE"

    # 2) Append original cubemx file
    cat "$CUBEMX_FILE" >> "$TMP_FILE"

    # 3) Replace original file
    mv "$TMP_FILE" "$CUBEMX_FILE"
done

find ../components -type d -name firmware_definitions | while read -r dir; do
    board=$(basename "$(dirname "$dir")")
    find "$dir" -type f | sed "s|^|[$board] |"
done

grep -rl '#include "main.h"' ../components/ | while read -r file; do
  sed -i 's/#include "main.h"/#include "cubemx_main.h"/g' "$file"
  echo "Updated include in $file"
done
