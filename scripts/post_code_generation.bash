OS_TYPE=$(uname -s)
MV_CMD="mv"

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

grep -rl '#include "main.h"' ../components/ | while read -r file; do
  sed -i 's/#include "main.h"/#include "cubemx_main.h"/g' "$file"
  echo "Updated include in $file"
done

grep -rl '^void' ../components/*/firmware/Core/Src/cubemx_main.c | while read -r file; do
  sed -i 's/^void/__weak void/g' "$file"
  echo "created weak voids in $file"
done
