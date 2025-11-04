
set -euo pipefail

# Rutas
ROOT_DIR=".."
SRC_DIR="$ROOT_DIR/src"
INC_DIR="$ROOT_DIR/include"
CPPLINT="./cpplint.py"   # en el mismo folder del script

REPORT="cpplint_report.txt"

# Descubre fuentes y headers (solo si existen)
FILES=()
if [ -d "$SRC_DIR" ]; then
  while IFS= read -r -d '' f; do FILES+=("$f"); done < <(find "$SRC_DIR" -type f \( -name '*.c' -o -name '*.h' \) -print0)
fi
if [ -d "$INC_DIR" ]; then
  while IFS= read -r -d '' f; do FILES+=("$f"); done < <(find "$INC_DIR" -type f -name '*.h' -print0)
fi

# Si te interesa forzar archivos concretos, descomenta y añade si existen:
# [ -f "$ROOT_DIR/src/main.c" ] && FILES+=("$ROOT_DIR/src/main.c")
# [ -f "$ROOT_DIR/include/functions.h" ] && FILES+=("$ROOT_DIR/include/functions.h")
# [ -f "$ROOT_DIR/src/functions.c" ] && FILES+=("$ROOT_DIR/src/functions.c")

if [ "${#FILES[@]}" -eq 0 ]; then
  echo "No C headers/sources found under $SRC_DIR or $INC_DIR. Nothing to lint."
  echo "Style Score: 100 out of 100"
  exit 0
fi

echo "--- Running static analysis with cpplint ---"

# Filtros:
#  - omitimos whitespace/ending_newline, runtime/threadsafe_fn, build/include_subdir,
#    legal/copyright y whitespace/comments (tu excepción solicitada).
FILTERS="-whitespace/ending_newline,-runtime/threadsafe_fn,-build/include_subdir,-legal/copyright,-whitespace/comments"

# Ejecuta cpplint. Usa --extensions para incluir C, y un root estable.
"$CPPLINT" \
  --root="$ROOT_DIR" \
  --extensions=c,h \
  --filter="$FILTERS" \
  "${FILES[@]}" > "$REPORT" 2>&1 || true

# Suma la severidad: el entero entre corchetes al final de cada línea (p. ej. [...] [4])
SEVERITY_SUM=$(grep -o '\[[0-9]\+\]$' "$REPORT" | grep -o '[0-9]\+' | awk '{s+=$1} END {print s+0}')

# Calcula Style Score (mínimo 0)
STYLE_SCORE=$((100 - SEVERITY_SUM * 2))
if [ "$STYLE_SCORE" -lt 0 ]; then STYLE_SCORE=0; fi

echo "--- Style Check Report ---"
cat "$REPORT"
echo "--------------------------"
echo "Style Score: $STYLE_SCORE out of 100"