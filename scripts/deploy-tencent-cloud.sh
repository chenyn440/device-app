#!/usr/bin/env bash
set -euo pipefail

usage() {
  cat <<'EOF'
Usage:
  bash scripts/deploy-tencent-cloud.sh --host <ip> [options]

Required:
  --host <ip-or-domain>         腾讯云主机地址

Options:
  --user <name>                 SSH 用户名 (default: root)
  --ssh-port <port>             SSH 端口 (default: 22)
  --identity <path>             SSH 私钥路径
  --remote-dir <path>           远端部署目录 (default: /opt/device-app)
  --gateway-port <port>         网关端口 (default: 8787)
  --skip-build                  跳过远端编译，直接重启已有二进制
  --help                        显示帮助

Example:
  bash scripts/deploy-tencent-cloud.sh \
    --host 1.2.3.4 \
    --user root \
    --identity ~/.ssh/id_rsa \
    --remote-dir /opt/device-app \
    --gateway-port 8787
EOF
}

ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"

HOST=""
USER_NAME="root"
SSH_PORT="22"
IDENTITY=""
REMOTE_DIR="/opt/device-app"
GATEWAY_PORT="8787"
SKIP_BUILD="0"

while [[ $# -gt 0 ]]; do
  case "$1" in
    --host) HOST="${2:-}"; shift 2 ;;
    --user) USER_NAME="${2:-}"; shift 2 ;;
    --ssh-port) SSH_PORT="${2:-}"; shift 2 ;;
    --identity) IDENTITY="${2:-}"; shift 2 ;;
    --remote-dir) REMOTE_DIR="${2:-}"; shift 2 ;;
    --gateway-port) GATEWAY_PORT="${2:-}"; shift 2 ;;
    --skip-build) SKIP_BUILD="1"; shift 1 ;;
    --help) usage; exit 0 ;;
    *) echo "Unknown arg: $1"; usage; exit 1 ;;
  esac
done

if [[ -z "$HOST" ]]; then
  echo "--host is required"
  usage
  exit 1
fi

SSH_ARGS=(-p "$SSH_PORT" -o StrictHostKeyChecking=accept-new)
if [[ -n "$IDENTITY" ]]; then
  SSH_ARGS+=(-i "$IDENTITY")
fi

RSYNC_SSH="ssh ${SSH_ARGS[*]}"

echo "[1/4] 上传项目到远端: ${USER_NAME}@${HOST}:${REMOTE_DIR}"
rsync -az --delete \
  --exclude ".git" \
  --exclude "build" \
  --exclude "build-local" \
  --exclude "build-release-macos" \
  --exclude "dist" \
  --exclude ".DS_Store" \
  -e "$RSYNC_SSH" \
  "$ROOT_DIR/" "${USER_NAME}@${HOST}:${REMOTE_DIR}/"

if [[ "$SKIP_BUILD" != "1" ]]; then
  echo "[2/4] 远端编译 device-gateway"
  ssh "${SSH_ARGS[@]}" "${USER_NAME}@${HOST}" "bash -lc '
    set -euo pipefail
    cd \"${REMOTE_DIR}\"
    if ! command -v cmake >/dev/null 2>&1; then
      echo \"cmake 未安装\" >&2
      exit 1
    fi
    if ! command -v qmake >/dev/null 2>&1; then
      echo \"Qt 未安装（至少需要 Qt6 + qmake）\" >&2
      exit 1
    fi
    QT_PREFIX=\$(qmake -query QT_INSTALL_PREFIX || true)
    if [[ -z \"\$QT_PREFIX\" ]]; then
      echo \"无法探测 Qt 安装路径\" >&2
      exit 1
    fi
    cmake -S . -B build-cloud -DQt6_DIR=\"\$QT_PREFIX/lib/cmake/Qt6\" -DCMAKE_BUILD_TYPE=Release
    cmake --build build-cloud -j 4 --target device-gateway
  '"
else
  echo "[2/4] 跳过远端编译"
fi

echo "[3/4] 重启远端 gateway"
ssh "${SSH_ARGS[@]}" "${USER_NAME}@${HOST}" "bash -lc '
  set -euo pipefail
  cd \"${REMOTE_DIR}\"
  mkdir -p logs
  pkill -f \"build-cloud/device-gateway|/device-gateway\" >/dev/null 2>&1 || true
  nohup env DEVICE_APP_GATEWAY_PORT=${GATEWAY_PORT} DEVICE_APP_WEB_ROOT=\"${REMOTE_DIR}/web\" \
    \"${REMOTE_DIR}/build-cloud/device-gateway\" >\"${REMOTE_DIR}/logs/device-gateway.log\" 2>&1 &
  sleep 1
  pgrep -f \"build-cloud/device-gateway|/device-gateway\" >/dev/null
'"

echo "[4/4] 部署完成"
echo "访问地址: http://${HOST}:${GATEWAY_PORT}/"
echo "远端日志: ${REMOTE_DIR}/logs/device-gateway.log"

