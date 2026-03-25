# DeviceApp

四极质谱上位机桌面应用，基于 Qt 6 Widgets + CMake。

## 本地构建

macOS:

```bash
cmake -S . -B build -DQt6_DIR="$(brew --prefix qt)/lib/cmake/Qt6"
cmake --build build -j 4
open build/device-app.app
```

## 发布到 GitHub Releases

仓库已经预留：

- `.github/workflows/release.yml`
- `scripts/package-macos.sh`
- `scripts/package-windows.ps1`

发布流程采用 Git tag 触发，默认发布产物：

- `device-app-<version>-macos.zip`
- `device-app-<version>-windows.zip`

### 1. 配置 GitHub 仓库 remote

```bash
git remote add origin <your-github-repo-url>
```

### 2. 推送代码

```bash
git push -u origin main
```

### 3. 打 tag 触发 Release

```bash
git tag v0.1.0
git push origin v0.1.0
```

GitHub Actions 会自动：

1. 在 macOS runner 构建并打包 `.app`
2. 在 Windows runner 构建并打包 `.exe` 目录
3. 创建 GitHub Release
4. 上传两个平台的 zip 包

## 本地手动打包

### macOS

要求本机已安装 Qt 6，并且能找到 `macdeployqt`。

```bash
bash ./scripts/package-macos.sh
```

如果 Qt 不在默认位置，可以显式传入：

```bash
QT_CMAKE_DIR=/path/to/Qt6/lib/cmake/Qt6 \
MACDEPLOYQT_BIN=/path/to/macdeployqt \
bash ./scripts/package-macos.sh
```

### Windows

在 PowerShell 中执行：

```powershell
./scripts/package-windows.ps1 -QtCMakeDir "C:\Qt\6.8.2\msvc2022_64\lib\cmake\Qt6"
```

## 当前假设

- 第一版自动发布平台为 macOS + Windows
- 产物为 zip 包，不包含 notarization、codesign 和 Windows 安装器
- GitHub Actions 使用 `jurplel/install-qt-action` 安装 Qt 6
