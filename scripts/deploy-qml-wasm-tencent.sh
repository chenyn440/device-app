#!/usr/bin/env bash
set -euo pipefail

usage() {
  cat <<'USAGE'
Usage:
  bash scripts/deploy-qml-wasm-tencent.sh --host <ip> [options]

Options:
  --host <ip-or-domain>     腾讯云主机
  --user <name>             SSH 用户 (default: root)
  --ssh-port <port>         SSH 端口 (default: 22)
  --identity <path>         SSH 私钥
  --remote-dir <path>       远端发布目录 (default: /opt/device-app-web)
  --build                   先本地构建 wasm
USAGE
}

ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
OUT_DIR="${OUT_DIR:-$ROOT_DIR/dist/qml-wasm}"

HOST=""
USER_NAME="root"
SSH_PORT="22"
IDENTITY=""
REMOTE_DIR="/opt/device-app-web"
DO_BUILD="0"

while [[ $# -gt 0 ]]; do
  case "$1" in
    --host) HOST="${2:-}"; shift 2 ;;
    --user) USER_NAME="${2:-}"; shift 2 ;;
    --ssh-port) SSH_PORT="${2:-}"; shift 2 ;;
    --identity) IDENTITY="${2:-}"; shift 2 ;;
    --remote-dir) REMOTE_DIR="${2:-}"; shift 2 ;;
    --build) DO_BUILD="1"; shift 1 ;;
    --help) usage; exit 0 ;;
    *) echo "Unknown arg: $1"; usage; exit 1 ;;
  esac
done

if [[ -z "$HOST" ]]; then
  echo "--host is required"
  exit 1
fi

if [[ "$DO_BUILD" == "1" ]]; then
  bash "$ROOT_DIR/scripts/build-qml-wasm.sh"
fi

if [[ ! -f "$OUT_DIR/device-app-qml.html" ]]; then
  echo "未找到 wasm 产物，请先执行: bash scripts/build-qml-wasm.sh"
  exit 1
fi

SSH_ARGS=(-p "$SSH_PORT" -o StrictHostKeyChecking=accept-new)
if [[ -n "$IDENTITY" ]]; then
  SSH_ARGS+=(-i "$IDENTITY")
fi

TS="$(date +%Y%m%d-%H%M%S)"
REMOTE_RELEASE_DIR="$REMOTE_DIR/releases/$TS"

ssh "${SSH_ARGS[@]}" "$USER_NAME@$HOST" "mkdir -p '$REMOTE_RELEASE_DIR' '$REMOTE_DIR/packages'"
rsync -az --delete -e "ssh ${SSH_ARGS[*]}" "$OUT_DIR/" "$USER_NAME@$HOST:$REMOTE_RELEASE_DIR/"

ssh "${SSH_ARGS[@]}" "$USER_NAME@$HOST" "bash -lc '
  set -euo pipefail
  ln -sfn "$REMOTE_RELEASE_DIR" "$REMOTE_DIR/current"
  tar -czf "$REMOTE_DIR/packages/device-app-qml-wasm-$TS.tar.gz" -C "$REMOTE_RELEASE_DIR" .
  echo "发布目录: $REMOTE_RELEASE_DIR"
  echo "current -> $REMOTE_DIR/current"
  echo "打包文件: $REMOTE_DIR/packages/device-app-qml-wasm-$TS.tar.gz"
'"

echo "部署完成，访问示例: http://$HOST/device-app-qml.html"
