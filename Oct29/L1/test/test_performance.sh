#!/bin/bash
set -euo pipefail

# ==============================================================
# Performance & Validation Test - monitor_cpu_activity
# Evaluates:
#   1) report.txt is generated (40 pts)
#   2) fopen()/fclose() used for /proc/uptime and /proc/cpuinfo (40 pts)
#   3) report.txt contains key concepts in EN or ES (20 pts)
# ==============================================================

# --- Colors ---
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[0;33m'
NC='\033[0m'

# --- Config ---
SRC_DIR="../src"
BUILD_DIR="../build"
EXECUTABLE=""
REPORT_FILE=""          # <-- ahora se detecta dinámicamente
FINAL_SCORE=0

echo "--- Compiling monitor_cpu_activity ---"
mkdir -p "$BUILD_DIR"
gcc -O2 -std=c11 "$SRC_DIR"/*.c -o "$BUILD_DIR/student_exec" 2> "$BUILD_DIR/build_errors.log" || {
  echo -e "${RED}Compilation FAILED. Check build_errors.log${NC}"
  exit 1
}
echo -e "${GREEN}Compilation successful!${NC}"

EXECUTABLE=$(find "$BUILD_DIR" -maxdepth 1 -type f -perm -u+x | head -n 1)
if [ -z "$EXECUTABLE" ]; then
  echo -e "${RED}ERROR: No executable found in $BUILD_DIR${NC}"
  exit 1
fi
echo -e "Using executable: ${YELLOW}$(basename "$EXECUTABLE")${NC}"

# 1) report.txt/reporte.txt generado (40 pts)
echo -e "\n--- Running program ---"
"$EXECUTABLE" > /dev/null 2>&1 || true

# Detectar archivo de reporte permitido en la raíz (..)
mapfile -t FOUND_FILES < <(find .. -maxdepth 1 -type f \( -iname 'report.txt' -o -iname 'reporte.txt' \) | sort)

if [ "${#FOUND_FILES[@]}" -eq 0 ]; then
  echo -e "${RED}FAIL:${NC} No se encontró report.txt/Report.txt/REPORT.txt o reporte.txt/Reporte.txt/REPORTE.txt en el directorio raíz."
  SCORE_REPORT=0
else
  REPORT_FILE="${FOUND_FILES[0]}"
  if [ "${#FOUND_FILES[@]}" -gt 1 ]; then
    echo -e "${YELLOW}WARNING:${NC} Se encontraron ${#FOUND_FILES[@]} archivos de reporte; usando: $(basename "$REPORT_FILE")"
  else
    echo -e "${GREEN}OK:${NC} Encontrado: $(basename "$REPORT_FILE")"
  fi
  SCORE_REPORT=40
fi

# 2) fopen/fclose en ambos pseudo-files (40 pts)
echo -e "\n--- Scanning source for fopen/fclose usage ---"
SRC_FILE=$(find "$SRC_DIR" -type f -name "*.c" | head -n 1)
if [ -z "$SRC_FILE" ]; then
  echo -e "${RED}ERROR:${NC} No C source file found in $SRC_DIR"
  SCORE_CODE=0
else
  FOPEN_UPTIME=$(grep -E 'fopen\s*\(.*"/proc/uptime"' "$SRC_FILE" | wc -l)
  FOPEN_CPUINFO=$(grep -E 'fopen\s*\(.*"/proc/cpuinfo"' "$SRC_FILE" | wc -l)
  FCLOSE_COUNT=$(grep -E 'fclose\s*\(' "$SRC_FILE" | wc -l)

  echo "Found fopen(/proc/uptime):  $FOPEN_UPTIME"
  echo "Found fopen(/proc/cpuinfo): $FOPEN_CPUINFO"
  echo "Found fclose():             $FCLOSE_COUNT"

  if [ "$FOPEN_UPTIME" -ge 1 ] && [ "$FOPEN_CPUINFO" -ge 1 ] && [ "$FCLOSE_COUNT" -ge 2 ]; then
    echo -e "${GREEN}OK:${NC} Proper use of fopen() and fclose() for both files."
    SCORE_CODE=40
  else
    echo -e "${YELLOW}WARNING:${NC} Missing or incomplete fopen()/fclose() usage."
    SCORE_CODE=20
  fi
fi

# 3) Contenido clave (EN o ES) (20 pts)
#    Grupo 1: CPU cores / núcleos
#    Grupo 2: uptime / tiempo de actividad
#    Grupo 3: idle / inactivo
#    Grupo 4: CPU utilization / usage / utilización / uso
echo -e "\n--- Checking key report content (EN/ES) ---"
if [ -z "$REPORT_FILE" ] || [ ! -f "$REPORT_FILE" ]; then
  echo -e "${RED}ERROR:${NC} Cannot check content; report file not found."
  SCORE_CONTENT=0
else
  GROUP_NAMES=(
    "CPU cores / núcleos"
    "Uptime / tiempo de actividad"
    "Idle / inactivo"
    "CPU utilization / usage / utilización / uso"
  )
  GROUP_PATTERNS=(
    'cpu cores|núcleos|nucleos|núcleos detectados|nucleos detectados|núcleos encontrados|nucleos encontrados'
    'uptime|tiempo de actividad|tiempo encendido|tiempo total'
    'idle|inactivo|tiempo inactivo'
    'cpu utilization|cpu usage|utilización de cpu|utilizacion de cpu|uso de cpu|porcentaje.*cpu'
  )

  FOUND=0
  for idx in "${!GROUP_PATTERNS[@]}"; do
    pat="${GROUP_PATTERNS[$idx]}"
    name="${GROUP_NAMES[$idx]}"
    if grep -Eqi -- "$pat" "$REPORT_FILE"; then
      echo -e "${GREEN}Found:${NC} $name"
      FOUND=$((FOUND+1))
    else
      echo -e "${YELLOW}Missing:${NC} $name"
    fi
  done

  if [ "$FOUND" -eq 4 ]; then
    SCORE_CONTENT=20
  elif [ "$FOUND" -ge 2 ]; then
    SCORE_CONTENT=10
  else
    SCORE_CONTENT=0
  fi
fi

# Nota final
FINAL_SCORE=$((SCORE_REPORT + SCORE_CODE + SCORE_CONTENT))
if [ "$FINAL_SCORE" -gt 100 ]; then FINAL_SCORE=100; fi

echo -e "\n--- Summary ---"
if [ "$FINAL_SCORE" -eq 100 ]; then
  echo -e "${GREEN}SUCCESS:${NC} All checks passed perfectly!"
elif [ "$FINAL_SCORE" -ge 70 ]; then
  echo -e "${YELLOW}Partial Success:${NC} Some criteria not fully met."
else
  echo -e "${RED}Needs Improvement:${NC} Program missing key requirements."
fi
echo -e "Final Grade: ${YELLOW}${FINAL_SCORE}${NC} / 100"
echo "-----------------------------"
echo "${FINAL_SCORE}" > "$BUILD_DIR/grade.txt"