#!/usr/bin/env bash
set -euo pipefail

PID_FILE="${PID_FILE:-/tmp/device-gateway.pid}"

if [[ -f "$PID_FILE" ]]; then
  PID="$(cat "$PID_FILE")"
  if kill -0 "$PID" 2>/dev/null; then
    kill "$PID" || true
    sleep 0.2
  fi
  rm -f "$PID_FILE"
fi

pkill -f "build-local/device-gateway|/device-gateway" >/dev/null 2>&1 || true
echo "gateway 已停止"

