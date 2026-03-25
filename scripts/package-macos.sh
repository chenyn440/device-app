#!/usr/bin/env bash

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${BUILD_DIR:-$ROOT_DIR/build-release-macos}"
APP_VERSION="${APP_VERSION:-}"

if [[ -z "${APP_VERSION}" ]]; then
  APP_VERSION="$(sed -n 's/^project(DeviceApp VERSION \([0-9.]*\).*/\1/p' "$ROOT_DIR/CMakeLists.txt" | head -n 1)"
fi

if [[ -z "${APP_VERSION}" ]]; then
  echo "Unable to determine app version."
  exit 1
fi

QT_CMAKE_DIR="${QT_CMAKE_DIR:-}"
if [[ -z "${QT_CMAKE_DIR}" && -n "${Qt6_DIR:-}" ]]; then
  QT_CMAKE_DIR="${Qt6_DIR}"
fi

if [[ -z "${QT_CMAKE_DIR}" ]]; then
  if command -v brew >/dev/null 2>&1 && brew list qt >/dev/null 2>&1; then
    BREW_QT_PREFIX="$(brew --prefix qt)"
    if [[ -d "$BREW_QT_PREFIX/lib/cmake/Qt6" ]]; then
      QT_CMAKE_DIR="$BREW_QT_PREFIX/lib/cmake/Qt6"
    fi
  fi
fi

if [[ -z "${QT_CMAKE_DIR}" ]]; then
  echo "Qt6 CMake package path not found. Set QT_CMAKE_DIR or Qt6_DIR."
  exit 1
fi

QT_PREFIX="$(cd "$QT_CMAKE_DIR/../../.." && pwd)"
QT_LIB_DIR="$QT_PREFIX/lib"
QT_PLUGIN_DIR="$QT_PREFIX/plugins"

mkdir -p "$BUILD_DIR"

cmake -S "$ROOT_DIR" -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=Release -DQt6_DIR="$QT_CMAKE_DIR"
cmake --build "$BUILD_DIR" --config Release -j 4

APP_PATH="$BUILD_DIR/device-app.app"
if [[ ! -d "$APP_PATH" ]]; then
  echo "Expected app bundle not found: $APP_PATH"
  exit 1
fi

MACDEPLOYQT_BIN="${MACDEPLOYQT_BIN:-}"
if [[ -z "${MACDEPLOYQT_BIN}" && -d "$(dirname "$QT_CMAKE_DIR")/../../../bin" ]]; then
  CANDIDATE="$(cd "$(dirname "$QT_CMAKE_DIR")/../../../bin" && pwd)/macdeployqt"
  if [[ -x "$CANDIDATE" ]]; then
    MACDEPLOYQT_BIN="$CANDIDATE"
  fi
fi

if [[ -z "${MACDEPLOYQT_BIN}" && -n "${BREW_QT_PREFIX:-}" ]]; then
  CANDIDATE="$BREW_QT_PREFIX/bin/macdeployqt"
  if [[ -x "$CANDIDATE" ]]; then
    MACDEPLOYQT_BIN="$CANDIDATE"
  fi
fi

if [[ -z "${MACDEPLOYQT_BIN}" ]]; then
  echo "macdeployqt not found. Set MACDEPLOYQT_BIN."
  exit 1
fi

MACDEPLOYQT_EXTRA_ARGS=(
  -verbose=1
  -no-strip
  -libpath="$QT_LIB_DIR"
)

if [[ -d "$QT_PLUGIN_DIR/platforms" ]]; then
  MACDEPLOYQT_EXTRA_ARGS+=(-plugindir="$QT_PLUGIN_DIR")
fi

"$MACDEPLOYQT_BIN" "$APP_PATH" "${MACDEPLOYQT_EXTRA_ARGS[@]}"

DIST_DIR="$ROOT_DIR/dist"
mkdir -p "$DIST_DIR"
ARCHIVE_NAME="device-app-${APP_VERSION}-macos.zip"
rm -f "$DIST_DIR/$ARCHIVE_NAME"

/usr/bin/ditto -c -k --sequesterRsrc --keepParent "$APP_PATH" "$DIST_DIR/$ARCHIVE_NAME"

echo "Created: $DIST_DIR/$ARCHIVE_NAME"
