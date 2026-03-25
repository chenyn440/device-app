# DeviceApp

四极质谱上位机桌面应用，基于 `Qt 6 Widgets + CMake`。

当前版本已经具备以下基础能力：

- 主窗口与左侧工作区导航
- 调谐页、监测页、数据页、设置页
- 设备连接、扫描控制、调谐参数、监测方法、单帧数据保存
- Mock 设备适配器
- GitHub Releases 自动发布基础设施

## 文档索引

- [ARCHITECTURE.md](./ARCHITECTURE.md)
  项目分层、核心类型、页面与服务关系
- [DEVELOPMENT.md](./DEVELOPMENT.md)
  本地环境、构建、运行、调试方式
- [RELEASE.md](./RELEASE.md)
  GitHub Actions、打包脚本、tag 发布流程
- [UI_GUIDE.md](./UI_GUIDE.md)
  当前界面结构、页面职责、UI 约束
- [API_GUIDE.md](./API_GUIDE.md)
  核心类型、设备接口、服务接口与仓储接口
- [TEST_PLAN.md](./TEST_PLAN.md)
  当前测试重点与验证路径
- [CONTRIBUTING.md](./CONTRIBUTING.md)
  开发约定、协作方式与文档更新规则
- [CHANGELOG.md](./CHANGELOG.md)
  当前版本变更记录
- [DELIVERY.md](./DELIVERY.md)
  当前交付内容、边界和验收建议
- [ROADMAP.md](./ROADMAP.md)
  下一阶段开发目标、里程碑和优先级

## 快速启动

macOS:

```bash
cmake -S . -B build -DQt6_DIR="$(brew --prefix qt)/lib/cmake/Qt6"
cmake --build build -j 4
open build/device-app.app
```

## 目录概览

```text
src/
  app/        应用上下文与服务装配
  core/       核心类型与应用设置
  device/     设备适配器接口与实现
  storage/    本地仓储与 JSON/CSV 持久化
  ui/         主窗口、页面、弹窗、控件
scripts/      本地打包脚本
.github/      GitHub Actions 工作流
```

## 发布概览

仓库已预留：

- `.github/workflows/release.yml`
- `scripts/package-macos.sh`
- `scripts/package-windows.ps1`

发布采用 Git tag 触发，默认产物：

- `device-app-<version>-macos.zip`
- `device-app-<version>-windows.zip`

详细说明见 [RELEASE.md](./RELEASE.md)。

## 当前假设

- 第一版自动发布平台为 `macOS + Windows`
- 产物为 zip，不包含 notarization、codesign 和 Windows 安装器
- 当前运行设备层以 `MockDeviceAdapter` 为主，真实协议仍为预留接入点
