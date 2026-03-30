#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
OUT_DIR="${OUT_DIR:-$ROOT_DIR/dist/qml-wasm}"
PORT="${PORT:-8091}"

if [[ ! -f "$OUT_DIR/device-app-qml.html" ]]; then
  echo "未找到 $OUT_DIR/device-app-qml.html，先执行: bash scripts/build-qml-wasm.sh"
  exit 1
fi

cd "$OUT_DIR"
URL="http://127.0.0.1:${PORT}/device-app-qml.html"
echo "预览地址: $URL"

if command -v open >/dev/null 2>&1; then
  open "$URL" >/dev/null 2>&1 || true
fi

python3 -m http.server "$PORT"
