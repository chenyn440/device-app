#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
BUILD_DIR="${BUILD_DIR:-$ROOT_DIR/build-wasm}"
OUT_DIR="${OUT_DIR:-$ROOT_DIR/dist/qml-wasm}"

# Preferred: explicit qt-cmake path for wasm kit
QT_WASM_CMAKE="${QT_WASM_CMAKE:-}"
if [[ -z "$QT_WASM_CMAKE" ]]; then
  if command -v qt-cmake >/dev/null 2>&1; then
    QT_WASM_CMAKE="$(command -v qt-cmake)"
  fi
fi

if [[ -z "$QT_WASM_CMAKE" ]]; then
  echo "未找到 QT_WASM_CMAKE。请设置 Qt for WebAssembly 的 qt-cmake 路径。"
  echo "示例: export QT_WASM_CMAKE=~/Qt/6.8.0/wasm_singlethread/bin/qt-cmake"
  exit 1
fi

cd "$ROOT_DIR"

CMAKE_ARGS=(-S . -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=Release)
if [[ -n "${QT_CHAINLOAD_TOOLCHAIN_FILE:-}" ]]; then
  CMAKE_ARGS+=(-DQT_CHAINLOAD_TOOLCHAIN_FILE="$QT_CHAINLOAD_TOOLCHAIN_FILE")
fi
if [[ -n "${EMSDK:-}" ]]; then
  CMAKE_ARGS+=(-DEMSDK="$EMSDK")
fi
if [[ -n "${QT_HOST_PATH:-}" ]]; then
  CMAKE_ARGS+=(-DQT_HOST_PATH="$QT_HOST_PATH")
else
  QT_WASM_PREFIX="$(cd "$(dirname "$QT_WASM_CMAKE")/.." && pwd)"
  QT_VERSION_DIR="$(cd "$QT_WASM_PREFIX/.." && pwd)"
  if [[ -d "$QT_VERSION_DIR/macos" ]]; then
    CMAKE_ARGS+=(-DQT_HOST_PATH="$QT_VERSION_DIR/macos")
  fi
fi

"$QT_WASM_CMAKE" "${CMAKE_ARGS[@]}"

CACHE_FILE="$BUILD_DIR/CMakeCache.txt"
if [[ ! -f "$CACHE_FILE" ]]; then
  echo "未找到 CMakeCache.txt，配置阶段失败。"
  exit 1
fi

SYSTEM_NAME="$(awk -F= '/^CMAKE_SYSTEM_NAME:/{print $2}' "$CACHE_FILE" | head -n 1)"
COMPILER="$(awk -F= '/^CMAKE_CXX_COMPILER:/{print $2}' "$CACHE_FILE" | head -n 1)"
HAS_EMSCRIPTEN="$(awk -F= '/^EMSCRIPTEN:INTERNAL=/{print $2}' "$CACHE_FILE" | head -n 1)"
if [[ "$SYSTEM_NAME" != "Emscripten" && "$HAS_EMSCRIPTEN" != "1" ]]; then
  echo "当前不是 WASM 工具链，已中止。"
  echo "检测到 CMAKE_SYSTEM_NAME=$SYSTEM_NAME"
  echo "检测到 CMAKE_CXX_COMPILER=$COMPILER"
  echo "检测到 EMSCRIPTEN=$HAS_EMSCRIPTEN"
  echo "请使用 Qt for WebAssembly 的 qt-cmake，例如："
  echo "  export QT_WASM_CMAKE=~/Qt/6.8.0/wasm_singlethread/bin/qt-cmake"
  exit 1
fi

cmake --build "$BUILD_DIR" -j 4 --target device-app-qml

APP_HTML="$(find "$BUILD_DIR" -name 'device-app-qml.html' | head -n 1 || true)"
if [[ -z "$APP_HTML" ]]; then
  echo "未找到 device-app-qml.html。"
  echo "请确认使用的是 Qt for WebAssembly 套件，并检查构建日志。"
  exit 1
fi

APP_DIR="$(dirname "$APP_HTML")"
mkdir -p "$OUT_DIR"
rsync -a --delete "$APP_DIR/" "$OUT_DIR/"

echo "QML WASM 构建完成: $OUT_DIR"
echo "入口文件: $OUT_DIR/device-app-qml.html"

bash "$ROOT_DIR/scripts/sync-qml-wasm-to-web.sh"
