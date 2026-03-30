#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
ENV_FILE="${ENV_FILE:-$ROOT_DIR/scripts/deploy.env}"

if [[ ! -f "$ENV_FILE" ]]; then
  echo "deploy.env not found: $ENV_FILE"
  echo "Please copy scripts/deploy.example.env to scripts/deploy.env and fill it."
  exit 1
fi

# shellcheck disable=SC1090
source "$ENV_FILE"

: "${TCLOUD_HOST:?TCLOUD_HOST is required}"
: "${TCLOUD_USER:=root}"
: "${TCLOUD_BRANCH:=main}"
: "${TCLOUD_SSH_PORT:=22}"
: "${TCLOUD_IDENTITY:=}"
: "${TCLOUD_REMOTE_DIR:=/opt/device-app}"

MSG="${1:-}"
if [[ -z "$MSG" ]]; then
  MSG="auto: $(date '+%Y-%m-%d %H:%M:%S')"
fi

cd "$ROOT_DIR"

CMD=(bash scripts/git-sync-remote-package.sh
  --message "$MSG"
  --host "$TCLOUD_HOST"
  --user "$TCLOUD_USER"
  --branch "$TCLOUD_BRANCH"
  --ssh-port "$TCLOUD_SSH_PORT"
  --remote-dir "$TCLOUD_REMOTE_DIR")

if [[ -n "$TCLOUD_IDENTITY" ]]; then
  CMD+=(--identity "$TCLOUD_IDENTITY")
fi

"${CMD[@]}"
