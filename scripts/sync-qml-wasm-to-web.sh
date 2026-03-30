#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
OUT_DIR="${OUT_DIR:-$ROOT_DIR/dist/qml-wasm}"
WEB_DIR="${WEB_DIR:-$ROOT_DIR/web}"

REQUIRED_FILES=(
  "device-app-qml.html"
  "device-app-qml.js"
  "device-app-qml.wasm"
  "qtloader.js"
  "qtlogo.svg"
)

for f in "${REQUIRED_FILES[@]}"; do
  if [[ ! -f "$OUT_DIR/$f" ]]; then
    echo "缺少构建产物: $OUT_DIR/$f"
    echo "请先执行: bash scripts/build-qml-wasm.sh"
    exit 1
  fi
done

mkdir -p "$WEB_DIR"
rsync -a --delete \
  "$OUT_DIR/device-app-qml.html" \
  "$OUT_DIR/device-app-qml.js" \
  "$OUT_DIR/device-app-qml.wasm" \
  "$OUT_DIR/qtloader.js" \
  "$OUT_DIR/qtlogo.svg" \
  "$WEB_DIR/"

echo "已同步 wasm 页面到 web 目录: $WEB_DIR"
