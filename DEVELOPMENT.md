# DEVELOPMENT

## 环境要求

- CMake `>= 3.21`
- C++17 编译器
- Qt 6，当前工程依赖：
  - `Core`
  - `Widgets`
  - `Charts`
  - `Network`

macOS 本地开发推荐：

- Homebrew 安装 `qt`、`cmake`

## 本地构建

### macOS

```bash
cmake -S . -B build -DQt6_DIR="$(brew --prefix qt)/lib/cmake/Qt6"
cmake --build build -j 4
open build/device-app.app
```

### 手动指定 Qt 路径

```bash
cmake -S . -B build -DQt6_DIR=/path/to/Qt6/lib/cmake/Qt6
cmake --build build -j 4
```

## 运行方式

编译完成后，macOS 默认产物：

- `build/device-app.app`

也可以直接运行可执行文件：

```bash
./build/device-app.app/Contents/MacOS/device-app
```

## 当前开发模式

当前仓库主要围绕 UI 快速迭代，常见流程：

1. 修改页面或组件
2. `cmake --build build -j 4`
3. 重启并打开 `build/device-app.app`
4. 对照截图继续调整

## 核心开发入口

### 页面

- `src/ui/pages/tune_page.cpp`
- `src/ui/pages/monitor_page.cpp`
- `src/ui/pages/data_page.cpp`
- `src/ui/pages/settings_page.cpp`

### 主窗口

- `src/ui/main_window.cpp`

### 服务层

- `src/app/services.h`
- `src/app/services.cpp`

### 核心类型

- `src/core/types.h`

### 设备适配器

- `src/device/device_adapter.h`
- `src/device/mock_device_adapter.cpp`
- `src/device/real_device_adapter.cpp`

## 调试建议

- 优先确认 Qt6 路径是否正确
- 页面布局问题优先从 `contentsMargins / spacing / fixedSize / stylesheet` 排查
- 如果界面未刷新，先确认是否重新编译并重新打开 `.app`
- 当前设备联调用 `MockDeviceAdapter`，不必依赖真实仪器

## 当前开发注意事项

- 当前工作区可能长期处于 UI 高频改动状态
- 不要把 `build/` 目录当成源码
- 真实设备协议尚未完成，不要把 Mock 行为当成最终协议
- 如果新增业务类型，优先放在 `src/core/types.h`
