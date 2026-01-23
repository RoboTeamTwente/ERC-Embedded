OS_TYPE=$(uname -s)
MV_CMD="mv"
set -euo pipefail
BASE="../components"
# Rename files

find "$BASE" -type f | while read -r file; do
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

    echo "Appending firmware_definitions to: $CUBEMX_FILE"

    TMP_FILE="$(mktemp)"

    head -n -1 "$CUBEMX_FILE" >> "$TMP_FILE"
    
    echo -e "\n/* ---- START firmware_definitions ---- */\n" >> "$TMP_FILE"
    find "$FW_DIR" -type f -exec cat {} \; >> "$TMP_FILE"
    echo -e "\n/* ---- END firmware_definitions ---- */\n" >> "$TMP_FILE"

    tail -n -1 "$CUBEMX_FILE" >> "$TMP_FILE"



    # 3) Replace original file
    mv "$TMP_FILE" "$CUBEMX_FILE"
done


find "$BASE" -type f -name "cubemx_main.c" | while read -r FILE; do
  TMP_FILE="$(mktemp)"
  echo "READING $FILE"
  while read -r line; do
      echo "$line" >> "$TMP_FILE"
      if [[ "$line" =~ ^[[:space:]]*static[[:space:]]+[a-zA-Z_][a-zA-Z0-9_]*[[:space:]]+[a-zA-Z_][a-zA-Z0-9_]*\([^\)]*\)\;[[:space:]]*$ ]]; then        # Remove 'static' and trailing ';'
        echo "Static function found: $line"
        proto=$(echo "$line" | sed -E 's/^[[:space:]]*static[[:space:]]+//; s/;[[:space:]]*$//')

        # Extract function name
        name=$(echo "$proto" | sed -E 's/.*[[:space:]]+([a-zA-Z_][a-zA-Z0-9_]*)\(.*/\1/')

        # Extract return type
        ret=$(echo "$proto" | sed -E "s/[[:space:]]+$name\(.*//")

        # Extract argument list
        args=$(echo "$proto" | sed -E "s/.*$name\((.*)\)/\1/")

        # Build argument names (remove types)
        call_args=$(echo "$args" | sed -E 's/[a-zA-Z_][a-zA-Z0-9_]*[[:space:]]+//g')
        if [[ "$call_args" == "void" ]]; then 
          call_args=""
        fi

        echo "$ret ${name}_wrapper($args) {" >> "$TMP_FILE"
        if [[ "$ret" == "void" ]]; then
            echo "    $name($call_args);" >> "$TMP_FILE"
        else
            echo "    return $name($call_args);" >> "$TMP_FILE"
        fi
        echo "}" >> "$TMP_FILE"
        echo >> "$TMP_FILE"
        echo "added wrapper for static function ${name} in ${FILE}"
      fi
  done < "$FILE"
  mv "$TMP_FILE" "$FILE"
done


grep -rl '#include "main.h"' ../components/ | while read -r file; do
  sed -i 's/#include "main.h"/#include "cubemx_main.h"/g' "$file"
  echo "Updated include in $file"
done
