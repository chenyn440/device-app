#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
BUILD_DIR="${BUILD_DIR:-$ROOT_DIR/build-local}"
PORT="${DEVICE_APP_GATEWAY_PORT:-8787}"
WEB_ROOT="${DEVICE_APP_WEB_ROOT:-$ROOT_DIR/web}"
PID_FILE="${PID_FILE:-/tmp/device-gateway.pid}"
LOG_FILE="${LOG_FILE:-/tmp/device-gateway.log}"
MODE_FILE="${MODE_FILE:-/tmp/device-web.mode}"
AI_PROVIDER="${DEVICE_APP_AI_PROVIDER:-cloud_zhipu}"
ZHIPU_BASE_URL="${DEVICE_APP_ZHIPU_BASE_URL:-https://open.bigmodel.cn/api/paas/v4}"
ZHIPU_MODEL="${DEVICE_APP_ZHIPU_MODEL:-glm-4-flash}"
ZHIPU_API_KEY="${DEVICE_APP_ZHIPU_API_KEY:-}"

cd "$ROOT_DIR"

if [[ -f "$PID_FILE" ]] && kill -0 "$(cat "$PID_FILE")" 2>/dev/null; then
  kill "$(cat "$PID_FILE")" || true
  sleep 0.3
fi

# Best-effort cleanup for stale gateway processes in the same build dir.
pkill -f "${BUILD_DIR}/device-gateway( |$)" >/dev/null 2>&1 || true

cmake --build "$BUILD_DIR" -j 4 --target device-gateway

if [[ "$AI_PROVIDER" == "cloud_zhipu" ]] && [[ -z "$ZHIPU_API_KEY" ]]; then
  echo "提示: 当前 AI_PROVIDER=cloud_zhipu，但 DEVICE_APP_ZHIPU_API_KEY 未配置。"
  echo "可先执行: export DEVICE_APP_ZHIPU_API_KEY='你的Key'"
fi

nohup env \
  DEVICE_APP_AI_PROVIDER="$AI_PROVIDER" \
  DEVICE_APP_ZHIPU_BASE_URL="$ZHIPU_BASE_URL" \
  DEVICE_APP_ZHIPU_MODEL="$ZHIPU_MODEL" \
  DEVICE_APP_ZHIPU_API_KEY="$ZHIPU_API_KEY" \
  DEVICE_APP_GATEWAY_PORT="$PORT" \
  DEVICE_APP_WEB_ROOT="$WEB_ROOT" \
  "$BUILD_DIR/device-gateway" >"$LOG_FILE" 2>&1 </dev/null &
PID=$!
MODE="gateway"

sleep 0.6
if kill -0 "$PID" 2>/dev/null; then
  echo "$PID" >"$PID_FILE"
  echo "gateway" >"$MODE_FILE"
  URL="http://127.0.0.1:${PORT}/"
  echo "gateway 已启动: PID=$PID URL=$URL"
else
  echo "gateway 启动失败，回退静态服务器，日志: $LOG_FILE"
  if ! command -v python3 >/dev/null 2>&1; then
    echo "python3 不可用，无法启动静态服务器"
    exit 1
  fi
  nohup python3 -m http.server "$PORT" --bind 127.0.0.1 --directory "$WEB_ROOT" >"$LOG_FILE" 2>&1 </dev/null &
  PID=$!
  sleep 0.4
  if ! kill -0 "$PID" 2>/dev/null; then
    echo "静态服务器启动失败，日志: $LOG_FILE"
    exit 1
  fi
  echo "$PID" >"$PID_FILE"
  echo "static" >"$MODE_FILE"
  MODE="static"
  URL="http://127.0.0.1:${PORT}/device-app-qml.html"
  echo "静态 Web 已启动: PID=$PID URL=$URL"
fi

# Re-check to avoid false-positive startup messages.
sleep 1
if ! kill -0 "$PID" 2>/dev/null; then
  if [[ "$MODE" == "gateway" ]]; then
    echo "gateway 启动后退出，切换静态服务器"
    nohup python3 -m http.server "$PORT" --bind 127.0.0.1 --directory "$WEB_ROOT" >"$LOG_FILE" 2>&1 </dev/null &
    PID=$!
    sleep 0.4
    if ! kill -0 "$PID" 2>/dev/null; then
      echo "静态服务器启动失败，日志: $LOG_FILE"
      exit 1
    fi
    echo "$PID" >"$PID_FILE"
    echo "static" >"$MODE_FILE"
    URL="http://127.0.0.1:${PORT}/device-app-qml.html"
    echo "静态 Web 已启动: PID=$PID URL=$URL"
  else
    echo "静态服务器启动后退出，日志: $LOG_FILE"
    exit 1
  fi
fi

if command -v open >/dev/null 2>&1; then
  open "$URL" >/dev/null 2>&1 || true
fi
