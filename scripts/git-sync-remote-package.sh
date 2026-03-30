#!/usr/bin/env bash
set -euo pipefail

usage() {
  cat <<'EOF'
Usage:
  bash scripts/git-sync-remote-package.sh --message "<commit message>" --host <ip> [options]

Required:
  --message <msg>               本地提交信息
  --host <ip-or-domain>         腾讯云主机地址

Optional:
  --branch <name>               Git 分支 (default: main)
  --user <name>                 SSH 用户名 (default: root)
  --ssh-port <port>             SSH 端口 (default: 22)
  --identity <path>             SSH 私钥路径
  --remote-dir <path>           远端仓库目录 (default: /opt/device-app)
  --skip-push                   跳过本地 push，只做远端拉取打包
  --skip-build                  跳过远端编译，只做打包
  --help                        显示帮助

Example:
  bash scripts/git-sync-remote-package.sh \
    --message "Tune page parity updates" \
    --host 1.2.3.4 \
    --user root \
    --identity ~/.ssh/id_rsa \
    --remote-dir /opt/device-app
EOF
}

ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
MESSAGE=""
HOST=""
BRANCH="main"
USER_NAME="root"
SSH_PORT="22"
IDENTITY=""
REMOTE_DIR="/opt/device-app"
SKIP_PUSH="0"
SKIP_BUILD="0"

while [[ $# -gt 0 ]]; do
  case "$1" in
    --message) MESSAGE="${2:-}"; shift 2 ;;
    --host) HOST="${2:-}"; shift 2 ;;
    --branch) BRANCH="${2:-}"; shift 2 ;;
    --user) USER_NAME="${2:-}"; shift 2 ;;
    --ssh-port) SSH_PORT="${2:-}"; shift 2 ;;
    --identity) IDENTITY="${2:-}"; shift 2 ;;
    --remote-dir) REMOTE_DIR="${2:-}"; shift 2 ;;
    --skip-push) SKIP_PUSH="1"; shift 1 ;;
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

if [[ "$SKIP_PUSH" != "1" && -z "$MESSAGE" ]]; then
  echo "--message is required unless --skip-push is used"
  usage
  exit 1
fi

SSH_ARGS=(-p "$SSH_PORT" -o StrictHostKeyChecking=accept-new)
if [[ -n "$IDENTITY" ]]; then
  SSH_ARGS+=(-i "$IDENTITY")
fi

cd "$ROOT_DIR"

if [[ "$SKIP_PUSH" != "1" ]]; then
  echo "[1/5] 本地提交并推送到 origin/$BRANCH"
  git add -A
  if ! git diff --cached --quiet; then
    git commit -m "$MESSAGE"
  else
    echo "无新增变更，跳过 commit"
  fi
  git push origin "$BRANCH"
else
  echo "[1/5] 跳过本地 push"
fi

echo "[2/5] 远端拉取最新代码"
ssh "${SSH_ARGS[@]}" "${USER_NAME}@${HOST}" "bash -lc '
  set -euo pipefail
  cd \"${REMOTE_DIR}\"
  git fetch origin
  git checkout \"${BRANCH}\"
  git pull --ff-only origin \"${BRANCH}\"
'"

if [[ "$SKIP_BUILD" != "1" ]]; then
  echo "[3/5] 远端编译 device-gateway (Release)"
  ssh "${SSH_ARGS[@]}" "${USER_NAME}@${HOST}" "bash -lc '
    set -euo pipefail
    cd \"${REMOTE_DIR}\"
    QT_PREFIX=\$(qmake -query QT_INSTALL_PREFIX || true)
    if [[ -z \"\$QT_PREFIX\" ]]; then
      echo \"未找到 qmake，无法探测 Qt6 安装路径\" >&2
      exit 1
    fi
    cmake -S . -B build-cloud -DQt6_DIR=\"\$QT_PREFIX/lib/cmake/Qt6\" -DCMAKE_BUILD_TYPE=Release
    cmake --build build-cloud -j 4 --target device-gateway
  '"
else
  echo "[3/5] 跳过远端编译"
fi

echo "[4/5] 远端打包"
ssh "${SSH_ARGS[@]}" "${USER_NAME}@${HOST}" "bash -lc '
  set -euo pipefail
  cd \"${REMOTE_DIR}\"
  mkdir -p dist
  TS=\$(date +%Y%m%d-%H%M%S)
  PKG=\"dist/device-gateway-linux-\${TS}.tar.gz\"
  tar -czf \"\$PKG\" \
    build-cloud/device-gateway \
    web \
    README.md \
    API_GUIDE.md \
    UI_GUIDE.md
  echo \"\$PKG\"
'"

echo "[5/5] 完成：本地 Git + 远端拉取 + 打包"

