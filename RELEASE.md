# RELEASE

## Summary

项目已具备基于 GitHub Actions 的自动发布基础设施，发布目标为：

- GitHub Releases
- macOS
- Windows

## 文件组成

### GitHub Actions

- `.github/workflows/release.yml`

职责：

- 监听 `v*` tag
- 分别在 macOS 和 Windows runner 构建
- 调用本地打包脚本
- 上传产物并创建 GitHub Release

### 本地打包脚本

- `scripts/package-macos.sh`
- `scripts/package-windows.ps1`

职责：

- 构建 release 版本
- 收集 Qt 运行时
- 生成发布 zip 包

## 发布产物

默认命名：

- `device-app-<version>-macos.zip`
- `device-app-<version>-windows.zip`

## 发布步骤

### 1. 配置 GitHub 仓库 remote

```bash
git remote add origin <your-github-repo-url>
```

### 2. 推送主分支

```bash
git push -u origin main
```

### 3. 打 tag

```bash
git tag v0.1.0
git push origin v0.1.0
```

### 4. GitHub Actions 自动执行

工作流将：

1. checkout 代码
2. 安装 Qt 6
3. 解析版本号
4. 运行平台打包脚本
5. 上传产物
6. 创建 GitHub Release

## 本地打包

### macOS

```bash
bash ./scripts/package-macos.sh
```

可选环境变量：

- `APP_VERSION`
- `BUILD_DIR`
- `QT_CMAKE_DIR`
- `MACDEPLOYQT_BIN`

### Windows

```powershell
./scripts/package-windows.ps1 -QtCMakeDir "C:\Qt\6.8.2\msvc2022_64\lib\cmake\Qt6"
```

可选参数：

- `-AppVersion`
- `-BuildDir`
- `-QtCMakeDir`

## 当前状态

已验证：

- macOS 打包脚本可本地生成 zip

待验证：

- GitHub 上实际触发 release
- Windows runner 打包结果

## 当前限制

- 未做 macOS notarization
- 未做 macOS codesign
- 未做 Windows 安装器
- 当前发布形式为压缩包，不是安装程序
