#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
BUILD_DIR="${BUILD_DIR:-$ROOT_DIR/build-local}"
PORT="${DEVICE_APP_GATEWAY_PORT:-8787}"
WEB_ROOT="${DEVICE_APP_WEB_ROOT:-$ROOT_DIR/web}"
PID_FILE="${PID_FILE:-/tmp/device-gateway.pid}"
LOG_FILE="${LOG_FILE:-/tmp/device-gateway.log}"

cd "$ROOT_DIR"

if [[ -f "$PID_FILE" ]] && kill -0 "$(cat "$PID_FILE")" 2>/dev/null; then
  kill "$(cat "$PID_FILE")" || true
  sleep 0.3
fi

pkill -f "build-local/device-gateway|/device-gateway" >/dev/null 2>&1 || true

cmake --build "$BUILD_DIR" -j 4 --target device-gateway

DEVICE_APP_GATEWAY_PORT="$PORT" DEVICE_APP_WEB_ROOT="$WEB_ROOT" \
  "$BUILD_DIR/device-gateway" >"$LOG_FILE" 2>&1 &
PID=$!
echo "$PID" >"$PID_FILE"

sleep 0.6
if ! kill -0 "$PID" 2>/dev/null; then
  echo "gateway 启动失败，日志: $LOG_FILE"
  exit 1
fi

URL="http://127.0.0.1:${PORT}/"
echo "gateway 已启动: PID=$PID URL=$URL"

if command -v open >/dev/null 2>&1; then
  open "$URL" >/dev/null 2>&1 || true
fi

