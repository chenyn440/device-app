#!/usr/bin/env bash
set -euo pipefail

PID_FILE="${PID_FILE:-/tmp/device-gateway.pid}"
MODE_FILE="${MODE_FILE:-/tmp/device-web.mode}"
ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
BUILD_DIR="${BUILD_DIR:-$ROOT_DIR/build-local}"

if [[ -f "$PID_FILE" ]]; then
  PID="$(cat "$PID_FILE")"
  if kill -0 "$PID" 2>/dev/null; then
    kill "$PID" || true
    sleep 0.2
  fi
  rm -f "$PID_FILE"
fi

pkill -f "${BUILD_DIR}/device-gateway( |$)" >/dev/null 2>&1 || true
rm -f "$MODE_FILE"
echo "gateway 已停止"
